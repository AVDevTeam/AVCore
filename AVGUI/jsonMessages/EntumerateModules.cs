using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AVGUI
{
    // Ответ 
    class EntumerateModulesAnswer
    {
        public List<string> Modules;
    }

    // Команда для получения списка модулей сервиса
    class EntumerateModulesCommand
    {
        public int id = 0;
        public string CommandName = "EnumerateModules";
    }

}
