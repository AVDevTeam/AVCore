using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Documents;

namespace AVGUI
{
    //status : 0 - запрос чего-то
    //status : 1 - ответ на запрос
    //id 0 - Выполнение действия (получить информацию или что-то сделать)
    //id 1 - Изменение настроек модулей

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
}
