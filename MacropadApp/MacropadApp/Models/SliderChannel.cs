//model mewarisi ViewModelBase untuk binding dan notifikasii perubahan, bersifat aktif karena nilai berubah2 ketika slider digeser geser.

using MacropadApp.ViewModels;
namespace MacropadApp.Models
{
    public class SliderChannel : ViewModelBase
    {
        public int SliderNumber { get; set; }

        private double _value;
        public double Value
        {
            get => _value;
            set => SetField(ref _value, value);
        }

        private List<string> _availableApps = new();
        public List<string> AvailableApps
        {
            get => _availableApps;
            set => SetField(ref _availableApps, value);
        }

        private string _assignedApp = "None";
        public string AssignedApp
        {
            get => _assignedApp;
            set => SetField(ref _assignedApp, value);
        }
    }
}
