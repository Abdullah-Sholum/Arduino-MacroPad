using System.IO;
using System.Text.Json;
using MacropadApp.Models;
namespace MacropadApp.Services
{
    public class ConfigService : IConfigService
    {
        private readonly string _filePath;

        public ConfigService()
        {
            string folder = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "MacropadApp");
            Directory.CreateDirectory(folder);
            _filePath = Path.Combine(folder, "config.json");
        }

        public AppConfig Load()
        {
            if (!File.Exists(_filePath)) return new AppConfig();

            try
            {
                string json = File.ReadAllText(_filePath);
                return JsonSerializer.Deserialize<AppConfig>(json) ?? new AppConfig();
            }
            catch
            {
                return new AppConfig(); // file corrupt/rusak -> mulai dari config kosong daripada crash
            }
        }

        public void Save(AppConfig config)
        {
            var options = new JsonSerializerOptions { WriteIndented = true };
            string json = JsonSerializer.Serialize(config, options);
            File.WriteAllText(_filePath, json);
        }
    }
}
