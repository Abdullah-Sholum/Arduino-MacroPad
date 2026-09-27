using System.Collections.Concurrent;
using System.Diagnostics;
using System.Runtime.InteropServices;
using NAudio.CoreAudioApi;

namespace MacropadApp.Services
{
    public class AudioService : IAudioService, IDisposable
    {
        private readonly object _lock = new();
        private readonly Dictionary<string, AudioSessionControl> _sessionCache =
            new(StringComparer.OrdinalIgnoreCase);

        // Semua kerja COM audio dieksekusi lewat SATU thread dedicated ini,
        // supaya objek COM selalu diakses dari thread yang sama persis —
        // menghindari cross-thread marshaling yang memicu Event object churn besar-besaran.
        private readonly BlockingCollection<Action> _workQueue = new();
        private readonly Thread _audioThread;
        private volatile bool _isDisposed = false;

        public AudioService()
        {
            _audioThread = new Thread(ProcessQueue)
            {
                IsBackground = true,
                Name = "AudioWorkerThread"
            };
            _audioThread.SetApartmentState(ApartmentState.MTA);
            _audioThread.Start();
        }

        private void ProcessQueue()
        {
            foreach (var action in _workQueue.GetConsumingEnumerable())
            {
                try
                {
                    action();
                }
                catch
                {
                    // jangan biarkan satu error mematikan seluruh worker thread
                }
            }
        }

        public void RefreshSessionCache()
        {
            if (_isDisposed) return;

            _workQueue.Add(() =>
            {
                lock (_lock)
                {
                    // lepas semua referensi lama sebelum diisi ulang
                    foreach (var oldSession in _sessionCache.Values)
                    {
                        try { Marshal.ReleaseComObject(oldSession); } catch { }
                    }
                    _sessionCache.Clear();

                    MMDeviceEnumerator? enumerator = null;
                    MMDevice? device = null;
                    AudioSessionManager? sessionManager = null;

                    try
                    {
                        enumerator = new MMDeviceEnumerator();
                        device = enumerator.GetDefaultAudioEndpoint(DataFlow.Render, Role.Multimedia);
                        sessionManager = device.AudioSessionManager;
                        var sessions = sessionManager.Sessions;

                        for (int i = 0; i < sessions.Count; i++)
                        {
                            var session = sessions[i];
                            try
                            {
                                int pid = (int)session.GetProcessID;
                                if (pid == 0)
                                {
                                    Marshal.ReleaseComObject(session);
                                    continue;
                                }

                                using var proc = Process.GetProcessById(pid);
                                // simpan sesi ini di cache — JANGAN release, dipakai lagi nanti oleh SetVolume
                                _sessionCache[proc.ProcessName] = session;
                            }
                            catch
                            {
                                try { Marshal.ReleaseComObject(session); } catch { }
                            }
                        }
                    }
                    catch
                    {
                        // gagal ambil default device / sessionManager -> cache kosong, aman, tidak crash
                    }
                    finally
                    {
                        if (sessionManager != null)
                        {
                            try { Marshal.ReleaseComObject(sessionManager); } catch { }
                        }
                        device?.Dispose();
                        enumerator?.Dispose();
                    }
                }
            });
        }

        public List<string> GetActiveSessionProcessNames()
        {
            lock (_lock)
            {
                return _sessionCache.Keys.ToList();
            }
        }

        public void SetVolume(string processName, double normalizedLevel)
        {
            if (_isDisposed) return;
            if (string.IsNullOrEmpty(processName) || processName == "None") return;

            _workQueue.Add(() =>
            {
                lock (_lock)
                {
                    if (_sessionCache.TryGetValue(processName, out var session))
                    {
                        try
                        {
                            session.SimpleAudioVolume.Volume = (float)normalizedLevel;
                        }
                        catch
                        {
                            // sesi sudah mati (app ditutup) sejak cache terakhir di-refresh
                            _sessionCache.Remove(processName);
                        }
                    }
                }
            });
        }

        public void SetMasterVolume(double normalizedLevel)
        {
            if (_isDisposed) return;

            _workQueue.Add(() =>
            {
                try
                {
                    using var enumerator = new MMDeviceEnumerator();
                    using var device = enumerator.GetDefaultAudioEndpoint(DataFlow.Render, Role.Multimedia);
                    device.AudioEndpointVolume.MasterVolumeLevelScalar = (float)normalizedLevel;
                }
                catch { }
            });
        }

        public void Dispose()
        {
            _isDisposed = true;
            _workQueue.CompleteAdding();

            lock (_lock)
            {
                foreach (var session in _sessionCache.Values)
                {
                    try { Marshal.ReleaseComObject(session); } catch { }
                }
                _sessionCache.Clear();
            }
        }
    }
}