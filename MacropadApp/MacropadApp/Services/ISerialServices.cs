
namespace MacropadApp.Services
{
    public class SliderDataEventArgs : EventArgs
    {
        public int[] Values { get; }
        public SliderDataEventArgs(int[] values) => Values = values;
    }

    public interface ISerialService
    {
        event EventHandler<SliderDataEventArgs>? DataReceived;
        event EventHandler<string>? ConnectionStatusChanged;

        void Start();
        void Stop();
    }
}
