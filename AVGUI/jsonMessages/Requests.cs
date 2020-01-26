using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AVGUI
{

    //id 0 - На запрос требуется ответ
    //id 1 - На запрос не требуется ответ

    // Выгрузить плагин
    class UnloadPluginRequest
    {
        public int id = 1;
        public string CommandName = "UnloadPlugin";
        public string PluginName;

        public UnloadPluginRequest(string _pluginName)
        {
            PluginName = _pluginName;
        }
    }

    // Загрузить плагин
    class LoadPluginRequest
    {
        public int id = 1;
        public string CommandName = "LoadPlugin";
        public string PluginName;

        public LoadPluginRequest(string _pluginName)
        {
            PluginName = _pluginName;
        }
    }

    // Изменить настройки плагина
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

    // Команда на запуск сканирования
    class ScanCommand
    {
        public int id = 0;
        public string CommandName = "Scan";
        public int Mode;

        public ScanCommand(int _mode)
        {
            Mode = _mode;
        }
    }

    // Перечислить все модули
    class EntumeratePluginsRequest
    {
        public int id = 0;
        public string CommandName = "EnumeratePlugins";
    }
}
