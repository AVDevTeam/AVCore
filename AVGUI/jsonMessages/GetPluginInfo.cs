using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AVGUI
{
    // Запрос на информацию по плагину
    class GetPluginInfoRequest
    {
        public int id = 0;
        public string CommandName = "GetPluginInfo";
        public string PluginName;

        public GetPluginInfoRequest(string _pluginName)
        {
            PluginName = _pluginName;
        }
    }
}