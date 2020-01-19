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

        // Таблица формата {ИмяМодуля : {ИмяПараметра : {значение, ..., ...}, ... } 
        Dictionary<string, Dictionary<string, List<string>>> changedParams;
        string ChosenModuleName;

        public PluginsPanelManager(PipeClient _pipe, StackPanel _pluginPanel, StackPanel _pluginsParametersPanel)
        {
            Pipe = _pipe;
            PluginsPanel = _pluginPanel;
            PluginsParametersPanel = _pluginsParametersPanel;

            changedParams = new Dictionary<string, Dictionary<string, List<string>>>();
        }

        // Создать кнопку очередного плагина
        public void AddToPluginsPanel(string _plugin)
        {
            Button btn = new Button();
            btn.Name = _plugin + "_btn";
            btn.Content = _plugin;
            btn.Margin = new Thickness(5, 2, 5, 2);
            btn.Click += Plugin_Clicked;
            PluginsPanel.Children.Add(btn);
        }

        // Возвращает новые параметры плагинов
        public Dictionary<string, Dictionary<string, List<string>>> getNewSettings()
        {
            return changedParams;
        }

        // Когда нажали на плагин
        private void Plugin_Clicked(object sender, RoutedEventArgs e)
        {
            Button thisBtn = (Button)e.Source;
            ChosenModuleName = thisBtn.Content.ToString();

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
                List<string> value = new List<string>();    // Значение параметра
                string name = item.Key;                     // Имя параметра
                JToken jValue = item.Value;                 // Значение параметра в js

                // Если этот параметр является список
                if (jValue is JArray)
                {
                    foreach (string param in jValue)
                    {
                        value.Add(param);
                    }
                }
                else
                {
                    value.Add(jValue.ToString());
                }

                AddPluginParamToParametersPanel(name, value);
            }

        }

        // Добавляет GroupBox с очередным параметром
        private void AddPluginParamToParametersPanel(string _name, List<string> _value)
        {
            // Добавить GroupBox с названием параметра
            GroupBox grbox = new GroupBox();
            grbox.Name = _name + "_groupbox";
            grbox.Header = _name;
            grbox.Padding = new Thickness(2);

            // Создать грид и поделить его на колонки
            Grid DynamicGrid = new Grid();
            DynamicGrid.Name = _name + "_grid";
            ColumnDefinition gridColumn1 = new ColumnDefinition();
            ColumnDefinition gridColumn2 = new ColumnDefinition();
            ColumnDefinition gridColumn3 = new ColumnDefinition();
            DynamicGrid.ColumnDefinitions.Add(gridColumn1);
            DynamicGrid.ColumnDefinitions.Add(gridColumn2);
            DynamicGrid.ColumnDefinitions.Add(gridColumn3);

            // Список строк отражающих текущее значение какого-то параметра модуля
            ListBox listBox = new ListBox();
            listBox.Name = _name + "_listbox";
            listBox.MaxHeight = 65;
            foreach (string val in _value)
            {
                listBox.Items.Add(val);
            }
            DynamicGrid.Children.Add(listBox);
            Grid.SetColumn(listBox, 0);

            // Поле для добавления нового значения
            TextBox txtbox = new TextBox();
            txtbox.Name = _name + "_txtbox";
            txtbox.Margin = new Thickness(5, 0, 5, 0);
            txtbox.TextWrapping = TextWrapping.Wrap;
            DynamicGrid.Children.Add(txtbox);
            Grid.SetColumn(txtbox, 1);


            // Грид содержащий 2 кнопки (Добавить и удалить параметр)
            Grid DynamicBtnGrid = new Grid();
            DynamicGrid.Name = _name + "_btnGrid";
            ColumnDefinition btnGridColumn1 = new ColumnDefinition();
            ColumnDefinition btnGridColumn2 = new ColumnDefinition();
            DynamicBtnGrid.ColumnDefinitions.Add(btnGridColumn1);
            DynamicBtnGrid.ColumnDefinitions.Add(btnGridColumn2);

            // Кнопка добавить (добавляет строку из прошло инпута в список строк)
            Button addBtn = new Button();
            addBtn.Name = _name + "_addBtn";
            addBtn.Content = "Add";
            addBtn.Click += addBtn_Clicked;
            addBtn.Margin = new Thickness(5, 0, 5, 0);
            addBtn.Background = Brushes.White;
            DynamicBtnGrid.Children.Add(addBtn);
            Grid.SetColumn(addBtn, 0);

            // Удаляет выбранную строку из списка строк
            Button deleteBtn = new Button();
            deleteBtn.Name = _name + "_deleteBtn";
            deleteBtn.Content = "Delete";
            deleteBtn.Click += deleteBtn_Clicked;
            deleteBtn.Margin = new Thickness(5, 0, 5, 0);
            deleteBtn.Background = Brushes.White;
            DynamicBtnGrid.Children.Add(deleteBtn);
            Grid.SetColumn(deleteBtn, 1);

            DynamicGrid.Children.Add(DynamicBtnGrid);
            Grid.SetColumn(DynamicBtnGrid, 2);

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

        // Кликнули по кнопке Add 
        private void addBtn_Clicked(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)e.Source;

            string groupboxName = btn.Name.Substring(0, btn.Name.IndexOf("_")) + "_groupbox";

            Grid grid           = null;
            GroupBox groupBox   = null;
            ListBox listBox     = null;
            TextBox textBox     = null;

            // Нашли нужный параметр среди всех параметров в стакпанел
            foreach (GroupBox child in PluginsParametersPanel.Children)
            {
                if (child.Name == groupboxName)
                {
                    groupBox = child;
                    grid = (Grid)child.Content;
                    break;
                }
            }

            // Нашли в этом параметре listbox
            foreach (var child in grid.Children)
            {
                if (child is ListBox)
                {
                    listBox = (ListBox)child;
                    break;
                }
            }

            // Нашли в этом параметре textbox
            foreach (var child in grid.Children)
            {
                if (child is TextBox)
                {
                    textBox = (TextBox)child;
                    break;
                }
            }

            // Если поле пустое, то ничего не делать
            if (textBox.Text == "")
            {
                return;
            }

            // Добавили в listBox новое значение
            listBox.Items.Add(textBox.Text);
            textBox.Text = "";

            // Список новых значений измененого параметра плагина
            List<string> value = new List<string>();
            foreach (string item in listBox.Items)
            {
                value.Add(item);
            }

            // Добавили данную настройку в список на изменение
            try
            {
                // Пытаемся добавить новую настройку
                changedParams[ChosenModuleName].Add(groupBox.Header.ToString(), value);
            }
            // Если для данного модуля еще не было настроек, то заводим ключ с именем модуля
            catch(KeyNotFoundException ex)
            {
                changedParams.Add(ChosenModuleName, null);
                changedParams[ChosenModuleName] = new Dictionary<string, List<string>> { { groupBox.Header.ToString(), value } };
            }
            // Если у данного модуля данная настройка уже изменялась, то нужно ее перетереть
            catch(System.ArgumentException ex2)
            {
                changedParams[ChosenModuleName] = new Dictionary<string, List<string>> { { groupBox.Header.ToString(), value } };
            }
        }

        // Кликнули по кнопке Delete
        private void deleteBtn_Clicked(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)e.Source;

            string groupboxName = btn.Name.Substring(0, btn.Name.IndexOf("_")) + "_groupbox";

            Grid grid = null;
            GroupBox groupBox = null;
            ListBox listBox = null;

            // Нашли нужный параметр среди всех параметров в стакпанел
            foreach (GroupBox child in PluginsParametersPanel.Children)
            {
                if (child.Name == groupboxName)
                {
                    groupBox = child;
                    grid = (Grid)child.Content;
                    break;
                }
            }

            // Нашли в этом параметре listbox
            foreach (var child in grid.Children)
            {
                if (child is ListBox)
                {
                    listBox = (ListBox)child;
                    break;
                }
            }

            // Удалили выделенный элемент
            if(listBox.SelectedIndex != -1)
            {
                listBox.Items.RemoveAt(listBox.SelectedIndex);
            }

            // Измененный список значение
            List<string> value = new List<string>();
            foreach (string item in listBox.Items)
            {
                value.Add(item);
            }

            // Добавили данную настройку в список на изменение
            try
            {
                changedParams[ChosenModuleName].Add(groupBox.Header.ToString(), value);
            }
            catch (KeyNotFoundException ex)
            {
                changedParams.Add(ChosenModuleName, null);
                changedParams[ChosenModuleName] = new Dictionary<string, List<string>> { { groupBox.Header.ToString(), value } };
            }
            catch (System.ArgumentException ex2)
            {
                changedParams[ChosenModuleName] = new Dictionary<string, List<string>> { { groupBox.Header.ToString(), value } };
            }
        }
    }
}
