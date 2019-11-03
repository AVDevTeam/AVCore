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

            // Запрос на список всех модулей сервиса
            string answer = "";
            string command = JsonConvert.SerializeObject(new EntumerateModulesCommand());

            Pipe.SendMessage(command);
            Pipe.ListenMessage();

            answer = Pipe.GetMessage(1);

            // Пока ответ от сервера не пришел
            int i = 0;
            while (answer == "")
            {
                i++;

                // Ожидать по секунде и снова проверять
                answer = Pipe.GetMessage(1);

                // Если за 3 секунды ответа не последовала - задать вопрос о продолжении
                if (i == 3)
                {
                    i = 0;
                    if (MessageBox.Show("AVCore didn't give modules list. Tra again?", "Get modules list failed",
                   MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.No)
                    {
                        ConnectionTextBlock.Text = "AVCore didnt give modules list. Try open settings window again later";
                        break;
                    }
                }

            }

            if (answer != "")
            {
                ConnectionTextBlock.Text = answer;
            }

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
