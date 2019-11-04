using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace AVGUI
{
    public class PipeClient
    {
        private NamedPipeClientStream Pipe;
        private StreamReader reader;
        private StreamWriter writer;

        Thread messageListener;     // Поток ожидащюий ответа от сервиса
        string message = "";             // Ответ от сервиса

        // Подключиться к pipe (_time - кол-вол миллисекунд для таймаута)
        public bool Connect(int _time)
        {
            try
            {
                Pipe.Connect(_time);
            }
            catch (TimeoutException e)
            {
                return false;
            }
            return true;
        }

        // Закрыть соединение
        public void Close()
        {
            Pipe.Close();
            Pipe.Dispose();
        }

        // Отправка сообщения клиенту
        public void SendMessage(string _message)
        {
            writer.WriteLine(_message);
            writer.Flush();
        }

        // Принять сообщения (отдельный поток)
        public void ListenMessage()
        {
            messageListener.Start();
        }

        // Ждет сообщение _ms милисекунд, _times раз проверяя пришло ли, если да, то забирает его
        public string GetMessage(int _ms, int _times)
        {
            string retMessage = "";

            for (int i = 0; i < _times; i++)
            {
                Thread.Sleep(_ms / _times);
                
                // Если ответ пришел
                if (message != "")
                {
                    retMessage = message;
                    message = "";
                }
            }

            return retMessage;
        }

        // Бесконца ждет сообщение, если что-то пришло, то забирает его
        public string GetMessage()
        {
            while (message != "")
            {
                Thread.Sleep(1000);
            }

            string retMessage = message;
            message = "";

            return retMessage;
        }

        public PipeClient(string _pipeName)
        {
            Pipe = new NamedPipeClientStream(_pipeName);    // Имя pipe'а
            reader = new StreamReader(Pipe);                // Читать сообщения
            writer = new StreamWriter(Pipe);                // Отправлять сообщения

            messageListener = new Thread(() => { message = reader.ReadLine(); });   // Поток читает сообщение и кладет его в message
        }


        ~PipeClient()
        {
            Close();
        }
    }
}
