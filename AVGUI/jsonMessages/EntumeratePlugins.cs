using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AVGUI
{
    // Ответ 
    class EntumeratePluginsReply
    {
        public List<string> Plugins;
    }

    // Запрос на список модулей сервиса
    class EntumeratePluginsRequest
    {
        public int id = 0;
        public string CommandName = "EnumeratePlugins";
    }

}
