
namespace MacropadApp.Services
{
    public interface IAudioService
    {
        void RefreshSessionCache();
        List<string> GetActiveSessionProcessNames();
        void SetVolume(string processName, double normalizedLevel);
        void SetMasterVolume(double normalizedLevel);
    }
}
