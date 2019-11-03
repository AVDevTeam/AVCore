using System;
using System.Windows;
using System.Windows.Controls;
using Newtonsoft.Json;

namespace AVGUI
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        PipeClient Pipe;
        int ScanMode = 1;

        public MainWindow()
        {
            InitializeComponent();
            ContentRendered += Main;
        }

        public void Main(Object sender, EventArgs e)
        {
            ContentRendered -= Main;

            // Подключить к сервису
            Pipe = new PipeClient("AVCoreConnection");

            // Пока пользователь не нажем "нет" попытки подключиться будут потовряться 
            while (!Pipe.Connect(1000))
            {
                if (MessageBox.Show("Connetion to AVCover failed. Tra again?", "Connetion to AVCover failed",
                    MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.No)
                {
                    break;
                }
            }
        }

        // Слайдер для изменения режима сканирования
        private void ScanModeSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            ((Slider)sender).SelectionEnd = e.NewValue;

            switch (ScanModeSlider.SelectionEnd)
            {
                case 0:
                    ScanButtonTextBlock.Text = "ПОЛНОЕ СКАНИРОВАНИЕ";
                    ScanMode = 0;
                    break;
                case 1:
                    ScanButtonTextBlock.Text = "БЫСТРОЕ СКАНИРОВАНИЕ";
                    ScanMode = 1;
                    break;
                case 2:
                    ScanButtonTextBlock.Text = "ВЫБОРОЧНОЕ СКАНИРОВАНИЕ";
                    ScanMode = 2;
                    break;
            }
        }

        private void SettingsButton_Click(object sender, RoutedEventArgs e)
        {
            SettingsWindow settingsWindow = new SettingsWindow(Pipe);
            settingsWindow.ShowDialog();
        }

        // Кликнули по кнопке скана
        private void ScanButton_Click(object sender, RoutedEventArgs e)
        {
            // Отправить команду на сканирование
            string command = JsonConvert.SerializeObject(new ScanCommand(ScanMode));
            Pipe.SendMessage(command);
        }
    }
}
