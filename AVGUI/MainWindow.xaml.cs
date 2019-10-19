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
            Loaded += Main;
        }

        public void Main(Object sender, EventArgs e)
        {
            Loaded -= Main;

            PipeClient pipe = new PipeClient("AVCorePipe");
            pipe.Connect();

            string message = pipe.ReciveMessage();
            pipe.SendMessage("HELLO");
        }
    }
}
