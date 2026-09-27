using System.IO.Ports;
using System.Management;

namespace MacropadApp.Services
{
    public class SerialService : ISerialService
    {
        private const string DeviceIdentifier = "MACROPAD";
        private const int BaudRate = 9600;
        private const int HandshakeTimeoutMs = 1500;

        private SerialPort? _activePort;
        private ManagementEventWatcher? _watcher;
        private readonly object _lock = new();

        public event EventHandler<SliderDataEventArgs>? DataReceived;
        public event EventHandler<string>? ConnectionStatusChanged;

        public void Start()
        {
            RaiseStatus("Searching...");

            // retry terbatas khusus kasus device sudah tercolok sebelum app dibuka
            for (int attempt = 0; attempt < 5 && _activePort == null; attempt++)
            {
                TryFindDevice();
                if (_activePort == null) Thread.Sleep(1000);
            }

            _watcher = new ManagementEventWatcher(
                new WqlEventQuery("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2 OR EventType = 3"));
            _watcher.EventArrived += OnDeviceChangeNotified;
            _watcher.Start();
        }

        private volatile bool _isStopping = false;

        public void Stop()
        {
            _isStopping = true;
            _watcher?.Stop();
            _watcher?.Dispose();
            ClosePort();
        }

        private void OnDeviceChangeNotified(object sender, EventArrivedEventArgs e)
        {
            try
            {
                uint eventType = Convert.ToUInt32(e.NewEvent.Properties["EventType"].Value);

                lock (_lock)
                {
                    if (eventType == 2 && _activePort == null)
                    {
                        RaiseStatus("Searching...");
                        TryFindDevice();
                    }
                }
            }
            finally
            {
                e.NewEvent?.Dispose();
            }
        }

        private void TryFindDevice()
        {
            foreach (var portName in SerialPort.GetPortNames())
            {
                if (_activePort != null) return;

                SerialPort? candidate = null;
                try
                {
                    candidate = new SerialPort(portName, BaudRate)
                    {
                        ReadTimeout = HandshakeTimeoutMs,
                        NewLine = "\r\n",
                        DtrEnable = true,
                        RtsEnable = true
                    };
                    candidate.Open();
                    Thread.Sleep(200);

                    string? matchedLine = null;
                    for (int attempt = 0; attempt < 3 && matchedLine == null; attempt++)
                    {
                        string line = candidate.ReadLine();
                        if (line.StartsWith(DeviceIdentifier))
                        {
                            matchedLine = line;
                        }
                    }

                    if (matchedLine != null)
                    {
                        _activePort = candidate;
                        RaiseStatus($"Connected ({portName})");
                        ParseAndRaise(matchedLine);
                        StartReadLoop();
                        return;
                    }

                    candidate.Close();
                    candidate.Dispose();
                }
                catch (Exception ex)
                {
                    System.Diagnostics.Debug.WriteLine($"[{portName}] gagal: {ex.GetType().Name} - {ex.Message}");
                    candidate?.Dispose();
                }
            }
            if (_activePort == null) RaiseStatus("Disconnected");
        }

        private void StartReadLoop()
        {
            var port = _activePort;
            if (port == null) return;

            Task.Run(() =>
            {
                port.ReadTimeout = SerialPort.InfiniteTimeout;
                while (true)
                {
                    try
                    {
                        string line = port.ReadLine();
                        ParseAndRaise(line);
                    }
                    catch
                    {
                        if (!_isStopping)
                        {
                            RaiseStatus("Disconnected");
                        }
                        ClosePort();
                        return;
                    }
                }
            });
        }

        private void ParseAndRaise(string line)
        {
            if (!line.StartsWith(DeviceIdentifier)) return;

            var parts = line.Split('|');
            var values = new int[parts.Length - 1];
            for (int i = 1; i < parts.Length; i++)
                int.TryParse(parts[i], out values[i - 1]);

            DataReceived?.Invoke(this, new SliderDataEventArgs(values));
        }

        private void ClosePort()
        {
            try
            {
                _activePort?.Close();
                _activePort?.Dispose();
            }
            catch { /* abaikan error saat cleanup port rusak */ }
            _activePort = null;
        }

        private void RaiseStatus(string status) => ConnectionStatusChanged?.Invoke(this, status);
    }
}
