//Model untuk simpan konfigurasi aplikasi. daftar aplikasi, slider assignment, dan daftar proses yang diabaikan. 

namespace MacropadApp.Models
{
    public class AppConfig
    {
        public List<AppEntry> RegisteredApps { get; set; } = new();
        public Dictionary<int, string> SliderAssignments { get; set; } = new();
        public List<string> IgnoredProcessNames { get; set; } = new();
    }
}
