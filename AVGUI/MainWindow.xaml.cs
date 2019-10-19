using System;
using System.Windows;
using System.Windows.Controls;

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

        private void ScanModeSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            ((Slider)sender).SelectionEnd = e.NewValue;

            switch (ScanModeSlider.SelectionEnd)
            {
                case 0:
                    ScanButtonTextBlock.Text = "ПОЛНОЕ СКАНИРОВАНИЕ";
                    break;
                case 1:
                    ScanButtonTextBlock.Text = "БЫСТРОЕ СКАНИРОВАНИЕ";
                    break;
                case 2:
                    ScanButtonTextBlock.Text = "ВЫБОРОЧНОЕ СКАНИРОВАНИЕ";
                    break;
            }

        }

        private void SettingsButton_Click(object sender, RoutedEventArgs e)
        {
            SettingsWindow settingsWindow = new SettingsWindow();
            settingsWindow.ShowDialog();
        }

        private void ScanButton_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
