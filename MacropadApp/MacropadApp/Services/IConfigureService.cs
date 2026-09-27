using System;
using System.Collections.Generic;
using System.Text;

namespace MacropadApp.Services
{
    public interface IConfigService
    {
        Models.AppConfig Load();
        void Save(Models.AppConfig config);
    }
}
