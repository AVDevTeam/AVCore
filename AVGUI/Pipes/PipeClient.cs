using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AVGUI
{
    class PipeClient
    {
        private NamedPipeClientStream Pipe;
        private StreamReader reader;
        private StreamWriter writer;

        public PipeClient(string _pipeName)
        {
            Pipe = new NamedPipeClientStream("AVCorePipe");
            reader = new StreamReader(Pipe);
            writer = new StreamWriter(Pipe);
        }

        public void Connect()
        {
            Pipe.Connect();
        }

        public void SendMessage(string _message)
        {
            writer.WriteLine(_message);
            writer.Flush();
        }

        public string ReciveMessage()
        {
            string message = reader.ReadLine();
            return message;
        }
    }
}
