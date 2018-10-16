using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PX4FileDownloader
{
    public partial class MainForm : Form
    {
        private bool _connected;

        private static SerialPort _serialPort;
        private List<List<String>> _fileInfo;

        public MainForm()
        {
            _serialPort = null;
            _connected = false;

            InitializeComponent();

            Version ver = Assembly.GetExecutingAssembly().GetName().Version;
            Text += " ver. " + ver.Major + "." + ver.Minor;

            UpdateComPortList();

            MyComboBoxBaudRate.Text     = Properties.Settings.Default.BaudRate;
            MyComboBoxComPort.Text      = Properties.Settings.Default.ComPort;
            MyTextBoxSelectedPath.Text  = Properties.Settings.Default.SelectedPath;
            MyComboBoxFileList.Text     = Properties.Settings.Default.SelectedFile;

            Application.DoEvents();
        }

        private void FlipEnablebility()
        {
            _connected = !_connected;

            if (!_connected)
            {
                MyButtonDisconnect.Enabled   = false;
                MyButtonConnect.Enabled     = true;
                MyButtonStartDownload.Enabled = false;
                MyButtonDeleteAllFiles.Enabled = false;
                MyButtonDeleteFile.Enabled = false;
                MyButtonOpenFolder.Enabled = false;

                MyTextBoxFileInfo.Enabled = false;
                MyTextBoxSelectedPath.Enabled = false;

                MyComboBoxFileList.Enabled  = false;
                MyComboBoxComPort.Enabled   = true;
                MyComboBoxBaudRate.Enabled  = true;
            }
            else
            {
                MyButtonDisconnect.Enabled   = true;
                MyButtonConnect.Enabled     = false;
                MyButtonStartDownload.Enabled = true;
                MyButtonDeleteAllFiles.Enabled = true;
                MyButtonDeleteFile.Enabled = true;
                MyButtonOpenFolder.Enabled = true;

                MyTextBoxFileInfo.Enabled = true;
                MyTextBoxSelectedPath.Enabled = true;

                MyComboBoxFileList.Enabled  = true;
                MyComboBoxComPort.Enabled   = false;
                MyComboBoxBaudRate.Enabled  = false;
            }
            Application.DoEvents();
        }

        private void UpdateComPortList()
        {
            // Get a list of serial port names.
            string[] ports = SerialPort.GetPortNames();

            // clear old COM-Port drop down list
            MyComboBoxComPort.Items.Clear();

            foreach (string port in ports)
            {
                MyComboBoxComPort.Items.Add(port);
            }
        }

        private void ComPortDataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            //if (_comPortRxEventEnabled == true)
            //{
            //    SerialPort sp = (SerialPort)sender;
            //    string indata = sp.ReadExisting();

            //    this.Invoke(new MethodInvoker(delegate ()
            //    {
            //        mycustomdebug(indata);
            //    }
            //    ));
            //}
        }
        // ===================================================

        private void MyComboBoxComPort_SelectedValueChanged(object sender, EventArgs e)
        {
            Properties.Settings.Default.ComPort = ((ComboBox)sender).Text;
        }

        private void MyBaudRateComboBox_SelectedValueChanged(object sender, EventArgs e)
        {
            Properties.Settings.Default.BaudRate = ((ComboBox)sender).Text;
        }

        private void MyButtonConnect_Click(object sender, EventArgs e)
        {
            try
            {
                _serialPort = new SerialPort(Properties.Settings.Default.ComPort);

                _serialPort.BaudRate    = Int32.Parse(Properties.Settings.Default.BaudRate);
                _serialPort.Parity      = Parity.None;
                _serialPort.StopBits    = StopBits.One;
                _serialPort.DataBits    = 8;
                _serialPort.Handshake   = Handshake.None;
                _serialPort.RtsEnable   = false;

                _serialPort.DataReceived += new SerialDataReceivedEventHandler(ComPortDataReceivedHandler);

                _serialPort.Open();
                FlipEnablebility();
            }
            catch (Exception ex)
            {
                if (_serialPort != null)
                {
                    _serialPort.Close();
                }
                MessageBox.Show(ex.Message, "COM Port konnte nicht geöffnet werden", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void MyButtonDisonnect_Click(object sender, EventArgs e)
        {
            try
            {
                _serialPort.Close();
                _serialPort = null;
                FlipEnablebility();
            }
            catch (Exception ex)
            {  
                MessageBox.Show(ex.Message, "COM Port konnte nicht geschlossen werden", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void MyButtonOpenFolder_Click(object sender, EventArgs e)
        {
            if (MySaveFileDialog.FileName.Length == 0)
            {
                MySaveFileDialog.FileName = "default.txt";
            }

            if (MySaveFileDialog.ShowDialog() == DialogResult.OK)
            {
                MyTextBoxSelectedPath.Text = MySaveFileDialog.FileName;
            }
        }

        private void MyComboBoxFileList_DropDown(object sender, EventArgs e)
        {
            MyComboBoxFileList.Items.Clear();
            _fileInfo = Helper.GetFileListFromPX4(_serialPort);
            MyComboBoxFileList.Items.AddRange(_fileInfo[0].ToArray<object>());
        }

        private void MyComboBoxFile_SelectedValueChanged(object sender, EventArgs e)
        {
            int idx = MyComboBoxFileList.SelectedIndex;
            int padlengt = 20;

            if (idx >= 0)
            {
                Properties.Settings.Default.SelectedFile = ((ComboBox)sender).Text;
                MyTextBoxFileInfo.Clear();
                MyTextBoxFileInfo.AppendText("file:".PadRight(padlengt, ' ') + _fileInfo[0][idx] + "\r\n");
                MyTextBoxFileInfo.AppendText("size:".PadRight(padlengt, ' ') + _fileInfo[1][idx] + " Bytes\r\n");
                MyTextBoxFileInfo.AppendText("time length:".PadRight(padlengt, ' ') + _fileInfo[2][idx] + "m " + _fileInfo[3][idx] + "s\r\n");
                MyTextBoxFileInfo.AppendText("signals:".PadRight(padlengt, ' ') + _fileInfo[4][idx] + "\r\n");
                MyTextBoxFileInfo.AppendText("sample time:".PadRight(padlengt, ' ') + _fileInfo[5][idx] + " ms\r\n");

                int size = Int32.Parse(_fileInfo[1][idx]);
                int downloadtime = (((size * 10) / (_serialPort.BaudRate * 8)) * 10) + 20;

                MyTextBoxFileInfo.AppendText("download time: ".PadRight(padlengt, ' ') + downloadtime / 60 + "m " + (downloadtime - ((downloadtime / 60) * 60)) + "s\r\n");

                //sprintf(buffArr, "/%s|%i|%i|%i|%i|%i\r\n", fno.fname, (int)fno.fsize / 1000,
                //    (int)(timeline / 60), (int)(timeline - (timeline / 60) * 60), (int)sig_cnt, (int)sampletime);
            }

        }

        private void MyButtonStartDownload_Click(object sender, EventArgs e)
        {
            if (MyTextBoxSelectedPath.Text.Length > 0 && MyComboBoxFileList.Items.Count > 0)
            {
                int idx = MyComboBoxFileList.SelectedIndex;
                int length_ms = (Int32.Parse(_fileInfo[2][idx]) * 60 + Int32.Parse(_fileInfo[3][idx])) * 1000;

                int a = Int32.Parse(_fileInfo[5][idx]);
                a = a == 0 ? 5 : a;

                Helper.DownloadFile(_serialPort, MyComboBoxFileList.Text, MyTextBoxSelectedPath.Text, MyProgressBar, length_ms / a);
            }
        }

        private void MyTextBoxSelectedPath_DoubleClick(object sender, EventArgs e)
        {
            MyButtonOpenFolder_Click(sender, e);
        }

        private void MyTextBoxSelectedPath_TextChanged(object sender, EventArgs e)
        {
            Properties.Settings.Default.SelectedPath = ((TextBox)sender).Text;
        }

        private void CloseAppication(object sender, FormClosingEventArgs e)
        {
            Properties.Settings.Default.Save();
        }

        private void MyButtonDeleteFile_Click(object sender, EventArgs e)
        {
            if (MyComboBoxFileList.Text.Length == 0)
            {
                return;
            }
            if (MessageBox.Show("Delete File " + MyComboBoxFileList.Text + "? Sure?", "Delete File", MessageBoxButtons.YesNo, MessageBoxIcon.Error) == DialogResult.Yes)
            {
                Helper.DeleteFile(_serialPort, MyComboBoxFileList.Text);
            }
        }

        private void MyButtonDeleteAllFiles_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Delete all files? Sure?", "Delete files", MessageBoxButtons.YesNo, MessageBoxIcon.Error) == DialogResult.Yes)
            {
                Helper.DeleteAllFiles(_serialPort);
                MessageBox.Show("Done", "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }
    }
}
