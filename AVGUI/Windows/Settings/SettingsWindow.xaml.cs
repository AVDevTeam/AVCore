﻿using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using AVGUI.Windows.Settings;

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

            // Создаем менеджера панели плагинов
            PluginsPanelManager PPManager = new PluginsPanelManager(Pipe, PluginsPanel, PluginsParametersPanel);

            string jPluginsList = GetModulesList();

            // Если модули пришли
            if (jPluginsList != "")
            {
                // Отобразить их в панели плагинов
                EntumeratePluginsReply PluginsListJson = JsonConvert.DeserializeObject<EntumeratePluginsReply>(jPluginsList);
                foreach (string plugin in PluginsListJson.Plugins)
                {
                    PPManager.AddToPluginsPanel(plugin);
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
    }
}