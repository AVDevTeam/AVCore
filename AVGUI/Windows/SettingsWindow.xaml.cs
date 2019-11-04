using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
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
            Button thisBtn = (Button)e.Source;

            // Отправить запрос на информацию по плагину
            string request = JsonConvert.SerializeObject(new GetPluginInfoRequest(thisBtn.Content.ToString()));
            Pipe.SendMessage(request);
            Pipe.ListenMessage();
            string reply = Pipe.GetMessage();
            Pipe.StopListening();

            // Парсим информацию о модулях
            JObject jPluginInfo = JObject.Parse(reply);
            foreach(var item in jPluginInfo)
            {
                
                string name = item.Key;
                JToken value = item.Value;
                if(value is JArray)
                {

                }

                // Добавить txtbox
                Grid DynamicGrid = new Grid();

                // Строки в гриде
                ColumnDefinition gridColumn1 = new ColumnDefinition();
                ColumnDefinition gridColumn2 = new ColumnDefinition();
                ColumnDefinition gridColumn3 = new ColumnDefinition();
                DynamicGrid.ColumnDefinitions.Add(gridColumn1);
                DynamicGrid.ColumnDefinitions.Add(gridColumn2);
                DynamicGrid.ColumnDefinitions.Add(gridColumn3);

                // Добавить label с названием параметра
                Label labl = new Label();
                labl.Name = name + "_lbal";
                labl.Content = name;
                labl.HorizontalAlignment = HorizontalAlignment.Center;
                DynamicGrid.Children.Add(labl);
                Grid.SetColumn(labl, 0);

                // Добавить поле для изменения этого параметра 
                TextBox txtbox = new TextBox();
                txtbox.Name = name + "_txtbox";
                txtbox.Margin = new Thickness(5, 0, 5, 0);
                DynamicGrid.Children.Add(txtbox);
                Grid.SetColumn(txtbox, 1);

                // Добавить кнопку применить
                Button btn = new Button();
                btn.Name = name + "_btn";
                btn.Content = "Apply";
                //btn.Click += Plugin_Clicked;
                DynamicGrid.Children.Add(btn);
                Grid.SetColumn(btn, 2);

         

                PluginsParametersPanel.Children.Add(DynamicGrid);
                break;
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
                reply = Pipe.GetMessage(1000, 4);

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
    }
}
