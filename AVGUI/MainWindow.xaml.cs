using System;
using System.IO;
using System.IO.Pipes;
using System.Windows;

namespace AVGUI
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            Console.WriteLine("WIK");

            //Client
            var client = new NamedPipeClientStream("AVCorePipe");
            client.Connect();
            StreamReader reader = new StreamReader(client);
            StreamWriter writer = new StreamWriter(client);

            string input = Console.ReadLine();
            input = "asd";
            /*
            if (String.IsNullOrEmpty(input))
            {
                break;
            }
            */
            writer.WriteLine(input);
            writer.Flush();


        }
    }
}
