using Newtonsoft.Json;
using System;
using System.Windows;
using AVGUI.Windows.Settings;
using Newtonsoft.Json.Linq;

namespace AVGUI
{
    public partial class SettingsWindow : Window
    {
        PipeClient Pipe;

        PluginsPanelManager PPManager;          // Менеджер панели плагинов


        public SettingsWindow(PipeClient _pipe)
        {
            InitializeComponent();
            Pipe = _pipe;
            ContentRendered += Main;
        }

        public void Main(Object sender, EventArgs e)
        {
            ContentRendered -= Main;

            // Создаем менеджера панели плагинов
            PPManager = new PluginsPanelManager(Pipe, PluginsPanel, PluginsParametersPanel);

            string jPluginsList = GetModulesList();

            // Если модули пришли
            if (jPluginsList != "")
            {
                // Отобразить их в панели плагинов
                JObject jSonMessage = JObject.Parse(jPluginsList);
                foreach (var plugin in jSonMessage)
                {
                    int isRun = (int)plugin.Value["IsRun"];
                    int version = 0;
                    string description = "";

                    if (isRun == 1)
                    {
                        version = (int)plugin.Value["Version"];
                        description = (string)plugin.Value["Description"];
                    }

                    PPManager.AddToPluginsPanel(plugin.Key, isRun, version, description);
                }
            }
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
                reply = Pipe.GetMessage(500, 2);

                if (reply == "")
                {
                    if (MessageBox.Show("AVCore didn't give modules list. Tra again?", "Get modules list failed",
                        MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.No)
                    {
                        MessageBox.Show("AVCore didnt give modules list. Try open settings window again later");
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

        // Настройки решили отменить
        private void cancelBtn_Clicked(object sender, RoutedEventArgs e)
        {

            this.Close();
            // TO DO отмена настроек
        }

        // Настройки решили применить
        private void acceptBtn_Clicked(object sender, RoutedEventArgs e)
        {
            // Отправили сообщение об изменении параметров для каждого модуля
            foreach(var item in PPManager.getNewSettings())
            {
                string request = JsonConvert.SerializeObject(new ChangePluginSettingsRequest(item.Key, item.Value));
                Pipe.SendMessage(request);
            }
            this.Close();


        }
    }
}
