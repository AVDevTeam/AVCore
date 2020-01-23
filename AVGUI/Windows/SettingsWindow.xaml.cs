using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace AVGUI
{
    /// <summary>
    /// Логика взаимодействия для SettingsWindow.xaml
    /// </summary>
    public partial class SettingsWindow : Window
    {
        PipeClient pipe;

        public SettingsWindow()
        {
            InitializeComponent();
            Loaded += Main;
        }

        public void Main(Object sender, EventArgs e)
        {
            Loaded -= Main;

            // Подключить к серверу с настройками
            pipe = new PipeClient("AVCoreSettings");
            pipe.Connect();

            pipe.SendMessage("EnumeratePlugins");
        }

        // Закрытие окна
        private void SettingsWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            pipe.Close();
        }
        
    }
}
