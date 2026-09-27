//Logika Utama untuk aplikasi. 

using MacropadApp.Helpers;
using MacropadApp.Models;
using MacropadApp.Services;
using System.Collections.ObjectModel;
using System.IO;
using System.Windows.Input;
using System.Windows.Threading;
using WpfApplication = System.Windows.Application;
using WpfOpenFileDialog = Microsoft.Win32.OpenFileDialog;

namespace MacropadApp.ViewModels
{
    public class MainViewModel : ViewModelBase
    {
        private readonly ISerialService _serialService;
        private readonly IAudioService _audioService;
        private readonly IConfigService _configService;
        private readonly Dictionary<int, SliderChannel> _sliderLookup;
        private readonly DispatcherTimer _activeSessionsTimer;
        private readonly HashSet<string> _ignoredProcessNames;

        public ObservableCollection<SliderChannel> Sliders { get; }
        public ObservableCollection<AppEntry> RegisteredApps { get; }
        public ObservableCollection<string> ActiveAppNames { get; } = new();
        public ObservableCollection<string> IgnoredAppNames { get; } = new();
        public ICommand UnignoreAppCommand { get; }

        private List<string> _activeSessionNames = new();

        private double _masterVolume;
        public double MasterVolume
        {
            get => _masterVolume;
            set
            {
                if (SetField(ref _masterVolume, value))
                    _audioService.SetMasterVolume(value / 100.0);
            }
        }

        private string _connectionStatus = "Searching...";
        public string ConnectionStatus
        {
            get => _connectionStatus;
            set => SetField(ref _connectionStatus, value);
        }

        public ICommand AddAppCommand { get; }
        public ICommand DeleteAppCommand { get; }
        public ICommand ToggleManageAppsCommand { get; }

        private bool _isManageAppsOpen;
        public bool IsManageAppsOpen
        {
            get => _isManageAppsOpen;
            set => SetField(ref _isManageAppsOpen, value);
        }

        public void ToggleManageAppsInternal() => IsManageAppsOpen = !IsManageAppsOpen;

        public MainViewModel(ISerialService serialService, IAudioService audioService, IConfigService configService)
        {

            _serialService = serialService;
            _audioService = audioService;
            _configService = configService;

            var config = _configService.Load();
            RegisteredApps = new ObservableCollection<AppEntry>(config.RegisteredApps);
            _ignoredProcessNames = new HashSet<string>(config.IgnoredProcessNames, StringComparer.OrdinalIgnoreCase);

            // urutan sesuai tata letak panel fisik: kanan ke kiri (6,5,4,3,2), Master terpisah
            Sliders = new ObservableCollection<SliderChannel>
            {
                new() { SliderNumber = 6 },
                new() { SliderNumber = 5 },
                new() { SliderNumber = 4 },
                new() { SliderNumber = 3 },
                new() { SliderNumber = 2 },
            };
            _sliderLookup = Sliders.ToDictionary(s => s.SliderNumber);

            // muat assignment tersimpan
            foreach (var slider in Sliders)
            {
                slider.AssignedApp = config.SliderAssignments.TryGetValue(slider.SliderNumber, out var app)
                    ? app : "None";

                // begitu user ganti pilihan dropdown -> simpan & hitung ulang pool tiap slider
                slider.PropertyChanged += (s, e) =>
                {
                    if (e.PropertyName == nameof(SliderChannel.AssignedApp))
                    {
                        SaveConfig();
                        RefreshAllDropdownPools();
                    }
                    else if (e.PropertyName == nameof(SliderChannel.Value))
                    {
                        var appName = slider.AssignedApp;
                        var volumeLevel = slider.Value / 100.0;

                        // lempar ke background thread — JANGAN blokir UI thread dengan kerja COM/audio
                        _audioService.SetVolume(appName, volumeLevel);
                    }
                };
            }

            AddAppCommand = new RelayCommand(AddApp);
            DeleteAppCommand = new RelayCommand<string>(DeleteApp);
            ToggleManageAppsCommand = new RelayCommand(ToggleManageAppsInternal);
            UnignoreAppCommand = new RelayCommand<string>(UnignoreApp);

            _serialService.ConnectionStatusChanged += (s, status) =>
            {
                try
                {
                    if (WpfApplication.Current == null) return;
                    WpfApplication.Current.Dispatcher.Invoke(() => ConnectionStatus = status);
                }
                catch (TaskCanceledException)
                {
                    // dispatcher sedang shutdown, event ini datang terlambat -> abaikan dengan aman
                }
            };

            _serialService.DataReceived += (s, e) =>
            {
                try
                {
                    if (WpfApplication.Current == null) return;
                    WpfApplication.Current.Dispatcher.Invoke(() => UpdateSliderValues(e.Values));
                }
                catch (TaskCanceledException)
                {
                }
            };
            Task.Run(() => _serialService.Start());
            _activeSessionsTimer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(15) };
            _activeSessionsTimer.Tick += (s, e) =>
            {
                _audioService.RefreshSessionCache();
                var names = _audioService.GetActiveSessionProcessNames();

                WpfApplication.Current?.Dispatcher.Invoke(() =>
                {
                    _activeSessionNames = names;
                    RefreshAllDropdownPools();
                });
            };
            _activeSessionsTimer.Start();

            Task.Run(() =>
            {
                _audioService.RefreshSessionCache();
                var names = _audioService.GetActiveSessionProcessNames();

                WpfApplication.Current?.Dispatcher.Invoke(() =>
                {
                    _activeSessionNames = names;
                    RefreshAllDropdownPools();
                });
            });
        }

        private void AddApp()
        {
            var dialog = new WpfOpenFileDialog
            {
                Title = "Pilih aplikasi",
                Filter = "Executable (*.exe)|*.exe",
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles)
            };

            if (dialog.ShowDialog() != true) return;

            string processName = Path.GetFileNameWithoutExtension(dialog.FileName);

            if (RegisteredApps.Any(a => a.ProcessName.Equals(processName, StringComparison.OrdinalIgnoreCase)))
                return;

            _ignoredProcessNames.Remove(processName); // TAMBAHAN: batalkan ignore kalau user sengaja tambah lagi
            RegisteredApps.Add(new AppEntry { ProcessName = processName, DisplayName = processName });
            SaveConfig();
            RefreshAllDropdownPools();
        }
        private void DeleteApp(string processName)
        {
            if (string.IsNullOrEmpty(processName) || processName == "None") return;

            var entry = RegisteredApps.FirstOrDefault(a =>
                a.ProcessName.Equals(processName, StringComparison.OrdinalIgnoreCase));

            if (entry != null)
            {
                RegisteredApps.Remove(entry);
            }

            // apapun sumbernya (RegisteredApps atau sesi aktif), begitu di-X,
            // masukkan ke ignore list supaya tidak muncul lagi selamanya
            _ignoredProcessNames.Add(processName);

            foreach (var slider in Sliders.Where(s =>
                s.AssignedApp.Equals(processName, StringComparison.OrdinalIgnoreCase)))
            {
                slider.AssignedApp = "None";
            }

            SaveConfig();
            RefreshAllDropdownPools();
        }
        private void UnignoreApp(string processName)
        {
            if (string.IsNullOrEmpty(processName)) return;

            _ignoredProcessNames.Remove(processName);
            SaveConfig();
            RefreshAllDropdownPools();
        }
        private void RefreshAllDropdownPools()
        {
            var visibleNames = RegisteredApps.Select(a => a.ProcessName)
                .Union(_activeSessionNames, StringComparer.OrdinalIgnoreCase)
                .Where(name => !_ignoredProcessNames.Contains(name))
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .ToList();

            //"Aktif/Terdaftar"
            if (!ActiveAppNames.SequenceEqual(visibleNames, StringComparer.OrdinalIgnoreCase))
            {
                ActiveAppNames.Clear();
                foreach (var name in visibleNames) ActiveAppNames.Add(name);
            }

            //"Diabaikan"
            var ignoredList = _ignoredProcessNames.ToList();
            if (!IgnoredAppNames.SequenceEqual(ignoredList, StringComparer.OrdinalIgnoreCase))
            {
                IgnoredAppNames.Clear();
                foreach (var name in ignoredList) IgnoredAppNames.Add(name);
            }

            foreach (var slider in Sliders)
            {
                var assignedElsewhere = Sliders
                    .Where(s => s.SliderNumber != slider.SliderNumber && s.AssignedApp != "None")
                    .Select(s => s.AssignedApp);

                var pool = new List<string> { "None" };
                pool.AddRange(visibleNames.Except(assignedElsewhere, StringComparer.OrdinalIgnoreCase));

                if (!slider.AvailableApps.SequenceEqual(pool, StringComparer.OrdinalIgnoreCase))
                {
                    slider.AvailableApps = pool;
                }
            }
        }
        private void SaveConfig()
        {
            var config = new AppConfig
            {
                RegisteredApps = RegisteredApps.ToList(),
                SliderAssignments = Sliders.ToDictionary(s => s.SliderNumber, s => s.AssignedApp),
                IgnoredProcessNames = _ignoredProcessNames.ToList() // BARU
            };
            _configService.Save(config);
        }
        private void UpdateSliderValues(int[] rawValues)
        {
            if (rawValues.Length == 0) return;

            MasterVolume = ToPercent(rawValues[0]);

            for (int firmwareIndex = 1; firmwareIndex < rawValues.Length; firmwareIndex++)
            {
                int targetSliderNumber = firmwareIndex + 1;
                if (_sliderLookup.TryGetValue(targetSliderNumber, out var channel))
                {
                    channel.Value = ToPercent(rawValues[firmwareIndex]);
                }
            }
        }
        private static double ToPercent(int raw) => raw / 1023.0 * 100.0;
    }
}
