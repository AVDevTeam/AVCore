using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AVGUI
{
    class ChangePluginSettingsRequest
    {
        public int id = 1;
        public string CommandName = "ChangePluginSettings";
        public string PluginName;
        public Dictionary<string, List<string>> ChangedParams;

        public ChangePluginSettingsRequest(string _pluginName, Dictionary<string, List<string>> _changedParams)
        {
            PluginName = _pluginName;
            ChangedParams = _changedParams;
        }
    }
}
