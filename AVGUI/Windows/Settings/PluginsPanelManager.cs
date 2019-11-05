using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace AVGUI.Windows.Settings
{
    // Данный класс занимается работой с отображением информации о плагинах
    class PluginsPanelManager
    {
        PipeClient Pipe;                        
        StackPanel PluginsPanel;                // Панель с плагинам
        StackPanel PluginsParametersPanel;      // Панелья с параметрами плагинов

        public PluginsPanelManager(PipeClient _pipe, StackPanel _pluginPanel, StackPanel _pluginsParametersPanel)
        {
            Pipe = _pipe;
            PluginsPanel = _pluginPanel;
            PluginsParametersPanel = _pluginsParametersPanel;
        }

        // Создать кнопку очередного плагина
        public void AddToPluginsPanel(string _plugin)
        {
            Button btn = new Button();
            btn.Name = _plugin + "_btn";
            btn.Content = _plugin;
            btn.Margin = new Thickness(5, 2, 5, 2);
            btn.Click += Plugin_Clicked;
            //btn.Background
            PluginsPanel.Children.Add(btn);
        }

        

        // Когда нажали на плагин
        public void Plugin_Clicked(object sender, RoutedEventArgs e)
        {
            Button thisBtn = (Button)e.Source;

            // Очистить панель
            CleanPluginsParametersPanel();

            // Отправить запрос на информацию по плагину
            string request = JsonConvert.SerializeObject(new GetPluginInfoRequest(thisBtn.Content.ToString()));
            Pipe.SendMessage(request);
            Pipe.ListenMessage();
            string reply = Pipe.GetMessage();
            Pipe.StopListening();

            // Парсим информацию о модулях
            JObject jPluginInfo = JObject.Parse(reply);
            foreach (var item in jPluginInfo)
            {
                string value = "";              // Значение параметра
                string name = item.Key;         // Имя параметра
                JToken jValue = item.Value;     // Значение параметра в js

                // Если этот параметр является список
                if (jValue is JArray)
                {
                    foreach (string param in jValue)
                    {
                        value += param + "\n";
                    }
                    value = value.Substring(0, value.Length - 1);
                }
                else
                {
                    value += jValue.ToString();
                }

                AddPluginParamToParametersPanel(name, value);
            }

        }

        // Добавляет GroupBox с очередным параметром
        private void AddPluginParamToParametersPanel(string _name,  string _value)
        {
            // Добавить GroupBox с названием параметра
            GroupBox grbox = new GroupBox();
            grbox.Name = _name + "_lbal";
            grbox.Header = _name;
            grbox.Padding = new Thickness(2);

            // Создать грид и поделить его на 2 колонки
            Grid DynamicGrid = new Grid();
            ColumnDefinition gridColumn1 = new ColumnDefinition();
            ColumnDefinition gridColumn2 = new ColumnDefinition();
            DynamicGrid.ColumnDefinitions.Add(gridColumn1);
            DynamicGrid.ColumnDefinitions.Add(gridColumn2);

            // Добавить поле для изменения этого параметра 
            TextBox txtbox = new TextBox();
            txtbox.Text = _value;
            txtbox.Name = _name + "_txtbox";
            txtbox.Margin = new Thickness(5, 2, 5, 2);
            DynamicGrid.Children.Add(txtbox);
            Grid.SetColumn(txtbox, 0);

            // Добавить кнопку применить
            Button btn = new Button();
            btn.Name = _name + "_btn";
            btn.Content = "Apply";
            //btn.Click += Plugin_Clicked;
            btn.Margin = new Thickness(5, 2, 5, 2);
            btn.Background = Brushes.White;
            DynamicGrid.Children.Add(btn);
            Grid.SetColumn(btn, 1);

            // Вставить грид в GroupBox, а GroupBox в StackPanel
            grbox.Content = DynamicGrid;
            PluginsParametersPanel.Children.Add(grbox);
        }

        // Очищает панель с параметрами плагинов
        private void CleanPluginsParametersPanel()
        {
            while (PluginsParametersPanel.Children.Count > 0)
            {
                PluginsParametersPanel.Children.RemoveAt(PluginsParametersPanel.Children.Count - 1);
            }
        }
    }
}
