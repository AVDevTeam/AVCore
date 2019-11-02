using Newtonsoft.Json;
using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;


namespace AVGUI
{
    /// <summary>
    /// Логика взаимодействия для SettingsWindow.xaml
    /// </summary>
    public partial class SettingsWindow : Window
    {
        PipeClient Pipe;

        public SettingsWindow(PipeClient _pipe)
        {
            InitializeComponent();
            Pipe = _pipe;
            ContentRendered += Main;
        }

        public void Main(Object sender, EventArgs e)
        {
            ContentRendered -= Main;

            string command;
            string answer = "";

            command = JsonConvert.SerializeObject(new EntumerateModulesCommand());

            Pipe.SendMessage(command);  // Отправить 
            Pipe.ListenMessage();

            // Ждать сообщение 3 секунды
            Pipe.GetMessage(3);


            //ConnectionProgressBar.Value = 100;
            //// Удалось подключиться 
            //if (Pipe.Connect() != -1)
            //{
            //    ConnectionTextBlock.Text = "Подключение установлено";
            //    //pipe.SendMessage("EnumeratePlugins");

            //}
            //// Не удалось
            //else
            //{
            //    ConnectionTextBlock.Text = "Не удалось подключиться";
            //}

        }

        // Закрытие окна
        private void SettingsWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            Pipe.Close();
        }

        private void ConnectionProgressBar_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {

        }
    }
}
