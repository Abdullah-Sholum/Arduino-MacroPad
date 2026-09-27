using WpfApplication = System.Windows.Application;
using WpfStartupEventArgs = System.Windows.StartupEventArgs;
using WpfExitEventArgs = System.Windows.ExitEventArgs;

using MacropadApp.Services;
using MacropadApp.ViewModels;
using MacropadApp.Views;

namespace MacropadApp
{
    public partial class App : WpfApplication
    {
        private ISerialService? _serialService;
        private AudioService? _audioService;

        protected override void OnStartup(WpfStartupEventArgs e)
        {
            base.OnStartup(e);
            ShutdownMode = System.Windows.ShutdownMode.OnExplicitShutdown;

            _serialService = new SerialService();
            _audioService = new AudioService();
            var configService = new ConfigService();
            var viewModel = new MainViewModel(_serialService, _audioService, configService);
            var window = new MainWindow { DataContext = viewModel };
            window.Show();
        }

        protected override void OnExit(WpfExitEventArgs e)
        {
            _serialService?.Stop();
            _audioService?.Dispose();
            base.OnExit(e);
        }
    }

}
