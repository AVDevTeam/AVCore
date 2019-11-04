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

            string jModulesList = GetModulesList();

            // Если модули пришли
            if (jModulesList != "")
            {
                ConnectionTextBlock.Text = jModulesList;

                // Отобразить их в панели плагинов
                EntumerateModulesAnswer ModulesListAns = JsonConvert.DeserializeObject<EntumerateModulesAnswer>(jModulesList);
                AddModulesToPluginsPanel(ModulesListAns);
            }

        }

        // Очищает панель с кнопками
        //private void ClearHandlesPanel()
        //{
        //    while (HandlesPanel.Children.Count > 0)
        //    {
        //        HandlesPanel.Children.RemoveAt(HandlesPanel.Children.Count - 1);
        //    }
        //}

        // Создать кнопку модуля
        private void AddModulesToPluginsPanel(EntumerateModulesAnswer _modulesListAns)
        {
            Button btn;

            foreach (string module in _modulesListAns.Modules)
            {
                btn = new Button();
                btn.Name = module + "_btn";
                btn.Content = module;
                btn.Click += Plugin_Clicked;
                PluginsPanel.Children.Add(btn);
            }
        }

        // Когда нажали на плагин
        private void Plugin_Clicked(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)e.Source;
            MessageBox.Show(btn.Content.ToString());
        }


        // На одну из кнопок (хэндлов объекта) кликнули
        //private void Button_Clicked(object sender, RoutedEventArgs e)
        //{
        //    Button btn = (Button)e.Source;
        //    WinObj.Cursor = Cursors.Cross;

        //    // Из всех переданных ранее объектов находится нужный и берется в фокус
        //    foreach (ObjectInfo obj in m_TransmittedObjectsList)
        //    {
        //        if (obj.Name == btn.Name)
        //        {
        //            // Заполнили информацию об объекте, который хотим создать
        //            m_NewObject = obj;
        //            FillObjectInfoForm(m_NewObject);

        //            // Так как объект еще не создан - формы невалидны
        //            NtfTypeChangeTextBox.IsEnabled = false;             // Поле ввода нотификатора стало невалидным
        //            NtfTypeValueChangeTextBox.IsEnabled = false;        // Поле ввода значения нотификатора стало невалидным
        //            NtfTypeValueChangeBtn.IsEnabled = false;            // Кнопка изменения нотификатора стала невалидной
        //            ObjectDeleteBtn.IsEnabled = false;                  // Кнопка удалить стала валидной
        //        }
        //    }
        //}

        // Отправить запрос на список модулей
        private string GetModulesList()
        {
            string ret = "";
            string command = JsonConvert.SerializeObject(new EntumerateModulesCommand());

            Pipe.SendMessage(command);
            Pipe.ListenMessage();

            while (ret == "")
            {
                ret = Pipe.GetMessage(1000, 4);

                if (ret == "")
                {
                    if (MessageBox.Show("AVCore didn't give modules list. Tra again?", "Get modules list failed",
                        MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.No)
                    {
                        ConnectionTextBlock.Text = "AVCore didnt give modules list. Try open settings window again later";
                        return "";
                        //break;
                    }
                }
            }

            return ret;
        }

        // Закрытие окна
        private void SettingsWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            Pipe.Close();
        }
    }
}
