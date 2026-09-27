using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace MacropadApp.Helpers
{
    public class DropdownDeleteButtonVisibilityConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            bool isInsidePopup = values.Length > 0 && values[0] is bool b && b;
            string? appName = values.Length > 1 ? values[1] as string : null;

            if (!isInsidePopup) return Visibility.Collapsed;
            if (string.IsNullOrEmpty(appName) || appName == "None") return Visibility.Collapsed;

            return Visibility.Visible;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
            => throw new NotImplementedException();
    }
}
