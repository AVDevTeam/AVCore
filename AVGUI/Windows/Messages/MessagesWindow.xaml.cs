using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Windows;


namespace AVGUI.Windows.Messages
{
    /// <summary>
    /// Логика взаимодействия для MessagesWindow.xaml
    /// </summary>
    public partial class MessagesWindow : Window
    {
        PipeClient Pipe;
        LogManager LogManager;      // Отвечает за логи

        public MessagesWindow(PipeClient _pipe)
        {
            InitializeComponent();
            Pipe = _pipe;
            ContentRendered += Main;
        }

        // Парсит json и пишет out сообщения
        private void parseJSonMessage(string _jSonMessage)
        {
            if(_jSonMessage == "")
            {
                return;
            }

            JObject jSonMessage = JObject.Parse(_jSonMessage);

            foreach (var item in jSonMessage)
            {
                if (item.Key == "alert")
                {
                    foreach (var item2 in item.Value)
                    {
                        LogManager.OutAlert(item2.ToString());
                    }
                       
                }
                else if(item.Key == "log")
                {
                    foreach (var item2 in item.Value)
                    {
                        LogManager.OutLog(item2.ToString());
                    }
                    //LogManager.OutLog(item.Value.ToString());
                }
                else if (item.Key == "debug")
                {
                    foreach (var item2 in item.Value)
                    {
                        LogManager.OutDebug(item2.ToString());
                    }
                    //LogManager.OutDebug(item.Value.ToString());
                }
                else if (item.Key == "warning")
                {
                    foreach (var item2 in item.Value)
                    {
                        LogManager.OutWarning(item2.ToString());
                    }
                    //LogManager.OutWarning(item.Value.ToString());
                }
            }

        }

        public void Main(Object sender, EventArgs e)
        {
            ContentRendered -= Main;
            LogManager = new LogManager(OutputHeader, RichTextBox1, AlertsHeader, RichTextBox2, WarningsHeader, RichTextBox3, DebugHeader, RichTextBox4);

            string reply = "";
            string request = "ready";

            Pipe.SendMessage(request);
            Pipe.ListenMessage();

            reply = Pipe.GetMessage(500, 2);
            parseJSonMessage(reply);
        }
    }
}
