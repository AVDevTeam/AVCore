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

        // Парсит json и пишет ouut сообщения
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
                    LogManager.OutAlert(item.Value.ToString());
                }
                else if(item.Key == "log")
                {
                    LogManager.OutLog(item.Value.ToString());
                }
                else if (item.Key == "debug")
                {
                    LogManager.OutDebug(item.Value.ToString());
                }
                else if (item.Key == "warning")
                {
                    LogManager.OutWarning(item.Value.ToString());
                }
            }
        }

        public void Main(Object sender, EventArgs e)
        {
            ContentRendered -= Main;
            LogManager = new LogManager(OutputHeader, RichTextBox1, AlertsHeader, RichTextBox2, WarningsHeader, RichTextBox3, DebugHeader, RichTextBox4);

            Pipe.Connect(500);

            Pipe.ListenMessage();

            string reply = "";
            reply = Pipe.GetMessage(500, 2);
            parseJSonMessage(reply);
        }
    }
}
