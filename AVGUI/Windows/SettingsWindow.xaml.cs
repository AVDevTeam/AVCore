using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

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

            string jPluginsList = GetModulesList();

            // Если модули пришли
            if (jPluginsList != "")
            {
                ConnectionTextBlock.Text = jPluginsList;

                // Отобразить их в панели плагинов
                EntumeratePluginsReply PluginsListJson = JsonConvert.DeserializeObject<EntumeratePluginsReply>(jPluginsList);
                AddModulesToPluginsPanel(PluginsListJson);
            }

        }

        // Создать кнопку плагина
        private void AddModulesToPluginsPanel(EntumeratePluginsReply _pluginsListJson)
        {
            Button btn;

            foreach (string plugin in _pluginsListJson.Plugins)
            {
                btn = new Button();
                btn.Name = plugin + "_btn";
                btn.Content = plugin;
                btn.Click += Plugin_Clicked;
                PluginsPanel.Children.Add(btn);
            }
        }

        // Когда нажали на плагин
        private void Plugin_Clicked(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)e.Source;

            // Отправить запрос на информацию по плагину
            string request = JsonConvert.SerializeObject(new GetPluginInfoRequest(btn.Content.ToString()));
            Pipe.SendMessage(request);
            Pipe.ListenMessage();
            string reply = Pipe.GetMessage();
            Pipe.StopListening();

            MessageBox.Show(reply);
        }

        // Отправить запрос на список модулей
        private string GetModulesList()
        {
            string reply = "";
            string request = JsonConvert.SerializeObject(new EntumeratePluginsRequest());

            Pipe.SendMessage(request);
            Pipe.ListenMessage();

            while (reply == "")
            {
                reply = Pipe.GetMessage(1000, 4);

                if (reply == "")
                {
                    if (MessageBox.Show("AVCore didn't give modules list. Tra again?", "Get modules list failed",
                        MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.No)
                    {
                        ConnectionTextBlock.Text = "AVCore didnt give modules list. Try open settings window again later";
                        break;
                    }
                }
            }

            Pipe.StopListening();
            return reply;
        }

        // Закрытие окна
        private void SettingsWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            //Pipe.Close();
        }
    }
}
