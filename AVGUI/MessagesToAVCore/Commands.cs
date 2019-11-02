using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Documents;

namespace AVGUI
{
    class ScanCommand
    {
        public string CommandName = "Scan";
        public int Mode;

        public ScanCommand(int _mode)
        {
            Mode = _mode;
        }
    }
}
