using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Documents;

namespace AVGUI
{
    //id 0 - На запрос требуется ответ
    //id 1 - На запрос не требуется ответ

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
