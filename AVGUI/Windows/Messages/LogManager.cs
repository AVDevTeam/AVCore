using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;

namespace AVGUI
{
    class LogManager
    {
        public int m_LogNumber { get; set; }            // Общее количество логов
        readonly TextBlock m_OutputHeader;              // Заголовок логов
        readonly RichTextBox m_OutputRichTextBox;       // Поле логов

        public int m_AlertNumber { get; set; }
        readonly TextBlock m_AlertsHeader;
        readonly RichTextBox m_AlertsRichTextBox;

        public int m_DebugNumber { get; set; }
        readonly TextBlock m_DebugHeader;
        readonly RichTextBox m_DebugRichTextBox;

        public int m_WarningNumber { get; set; }
        readonly TextBlock m_WarningHeader;
        readonly RichTextBox m_WarningRichTextBox;

        public LogManager(TextBlock _outputHeader, RichTextBox _outputRichTextBox, TextBlock _alertsHeader, RichTextBox _alertsRichTextBox,
                          TextBlock _warningHeader, RichTextBox m_warningRichTextBox, TextBlock _debugHeader, RichTextBox _debugRichTextBox)
        {
            m_LogNumber = 0;
            m_AlertNumber = 0;
            m_DebugNumber = 0;
            m_WarningNumber = 0;
            m_OutputHeader = _outputHeader;
            m_AlertsHeader = _alertsHeader;
            m_DebugHeader = _debugHeader;
            m_WarningHeader = _warningHeader;
            m_OutputRichTextBox = _outputRichTextBox;
            m_AlertsRichTextBox = _alertsRichTextBox;
            m_DebugRichTextBox = _debugRichTextBox;
            m_WarningRichTextBox = m_warningRichTextBox;
        }

        // Лог в общее окно логов
        public void OutLog(string _text)
        {
            m_LogNumber++;
            String label = "Output (" + m_LogNumber + ")";
            m_OutputHeader.Text = label;

            TextRange outString = new TextRange(m_OutputRichTextBox.Document.ContentEnd, m_OutputRichTextBox.Document.ContentEnd);
            outString.Text = _text + "\r\n";

            m_OutputRichTextBox.ScrollToEnd();
        }
        // Лог ошибки
        public void OutAlert(string _text)
        {
            m_LogNumber++;
            m_AlertNumber++;
            String label1 = "Output (" + m_LogNumber + ")";
            String label2 = "Alert (" + m_AlertNumber + ")";

            m_OutputHeader.Text = label1;
            m_AlertsHeader.Text = label2;

            TextRange outString = new TextRange(m_OutputRichTextBox.Document.ContentEnd, m_OutputRichTextBox.Document.ContentEnd);
            outString.Text = "Alert: " + _text + "\r\n";
            outString.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.Red);

            outString = new TextRange(m_AlertsRichTextBox.Document.ContentEnd, m_AlertsRichTextBox.Document.ContentEnd);
            outString.Text = _text + "\r\n";

            m_AlertsRichTextBox.ScrollToEnd();
            m_OutputRichTextBox.ScrollToEnd();
        }
        // Лог отладки
        public void OutDebug(string _text)
        {
            m_LogNumber++;
            m_DebugNumber++;

            String label1 = "Output (" + m_LogNumber + ")";
            String label2 = "Debug (" + m_DebugNumber + ")";

            m_OutputHeader.Text = label1;
            m_DebugHeader.Text = label2;

            TextRange outString = new TextRange(m_OutputRichTextBox.Document.ContentEnd, m_OutputRichTextBox.Document.ContentEnd);
            outString.Text = "Debug: " + _text + "\r\n";
            outString.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.Sienna);

            outString = new TextRange(m_DebugRichTextBox.Document.ContentEnd, m_DebugRichTextBox.Document.ContentEnd);
            outString.Text = _text + "\r\n";

            m_DebugRichTextBox.ScrollToEnd();
            m_OutputRichTextBox.ScrollToEnd();
        }
        // Лог предупреждение
        public void OutWarning(string _text)
        {
            m_LogNumber++;
            m_WarningNumber++;
            String label1 = "Output (" + m_LogNumber + ")";
            String label2 = "Warning (" + m_WarningNumber + ")";

            m_OutputHeader.Text = label1;
            m_WarningHeader.Text = label2;

            TextRange outString = new TextRange(m_OutputRichTextBox.Document.ContentEnd, m_OutputRichTextBox.Document.ContentEnd);
            outString.Text = "Warning: " + _text + "\r\n";
            outString.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.Goldenrod);

            outString = new TextRange(m_WarningRichTextBox.Document.ContentEnd, m_WarningRichTextBox.Document.ContentEnd);
            outString.Text = _text + "\r\n";

            m_WarningRichTextBox.ScrollToEnd();
            m_OutputRichTextBox.ScrollToEnd();
        }
    }

    
}
