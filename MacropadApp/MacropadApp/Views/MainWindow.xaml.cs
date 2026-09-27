using MacropadApp.ViewModels;
using System.ComponentModel;
using System.Reflection;
using System.Windows;
using System.Windows.Input;
using FormsContextMenuStrip = System.Windows.Forms.ContextMenuStrip;
using FormsForm = System.Windows.Forms.Form;
using FormsNotifyIcon = System.Windows.Forms.NotifyIcon;
using WpfApplication = System.Windows.Application;
using WpfWindow = System.Windows.Window;

namespace MacropadApp.Views
{
    public partial class MainWindow : WpfWindow
    {
        private FormsNotifyIcon? _trayIcon;
        private FormsForm? _trayOwnerForm;
        private bool _isExiting = false;

        public MainWindow()
        {
            InitializeComponent();
            SetupTrayIcon();

            Deactivated += MainWindow_Deactivated;
            //System.Diagnostics.Debug.WriteLine($"[MainWindow] Constructor dipanggil. Instance ID: {this.GetHashCode()}");
        }

        private void SetupTrayIcon()
        {
            var menu = new FormsContextMenuStrip();
            menu.Items.Add("Show", null, (s, e) => ShowWindow());
            menu.Items.Add("Exit", null, (s, e) => ExitApplication());

            var assembly = Assembly.GetExecutingAssembly();
            using var iconStream = assembly.GetManifestResourceStream("MacropadApp.Assets.app_icon.ico");

            _trayIcon = new FormsNotifyIcon
            {
                Icon = iconStream != null
                    ? new System.Drawing.Icon(iconStream)
                    : System.Drawing.SystemIcons.Application, // fallback kalau resource gagal ditemukan
                Visible = true,
                Text = "Macropad V3",
                ContextMenuStrip = menu
            };

            _trayIcon.DoubleClick += (s, e) => ShowWindow();
        }

        private void ShowWindow()
        {
            Show();
            WindowState = System.Windows.WindowState.Normal;
            Activate();
        }

        private void ExitApplication()
        {
            _isExiting = true;
            Close();
            WpfApplication.Current.Shutdown();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            if (!_isExiting)
            {
                if (DataContext is MainViewModel vm)
                {
                    vm.IsManageAppsOpen = false;
                }
                e.Cancel = true;
                Hide();
                return;
            }

            _trayIcon?.Dispose();
            _trayOwnerForm?.Dispose();
            base.OnClosing(e);
        }

        private void MainWindow_Deactivated(object sender, EventArgs e)
        {
            if (DataContext is MainViewModel vm)
            {
                vm.IsManageAppsOpen = false;
            }
        }

        private void Button_Click(object sender, System.Windows.RoutedEventArgs e)
        {

        }

        private void Slider_ValueChanged(object sender, System.Windows.RoutedPropertyChangedEventArgs<double> e)
        {

        }

        private void Button_Click_1(object sender, System.Windows.RoutedEventArgs e)
        {

        }

        private void Popup_Closed(object sender, EventArgs e)
        {
            if (DataContext is MainViewModel vm)
            {
                vm.IsManageAppsOpen = false;
            }
        }

        private void ManageAppsButton_Click(object sender, RoutedEventArgs e)
        {
            if (DataContext is MainViewModel vm)
            {
                vm.IsManageAppsOpen = !vm.IsManageAppsOpen; // toggle murni, tidak ada lagi campur tangan WPF
            }
        }

        protected override void OnPreviewMouseDown(MouseButtonEventArgs e)
        {
            base.OnPreviewMouseDown(e);

            if (DataContext is MainViewModel vm && vm.IsManageAppsOpen)
            {
                var clickedElement = e.OriginalSource as DependencyObject;

                bool clickedInsidePopup = ManageAppsPopup.Child != null &&
                    clickedElement != null &&
                    IsDescendantOf(ManageAppsPopup.Child, clickedElement);

                bool clickedOnButton = clickedElement != null &&
                    IsDescendantOf(ManageAppsButton, clickedElement);

                if (!clickedInsidePopup && !clickedOnButton)
                {
                    vm.IsManageAppsOpen = false;
                }
            }
        }

        private bool IsDescendantOf(DependencyObject parent, DependencyObject child)
        {
            var current = child;
            while (current != null)
            {
                if (current == parent) return true;
                current = System.Windows.Media.VisualTreeHelper.GetParent(current);
            }
            return false;
        }
    }
}