namespace PX4FileDownloader
{
    partial class MainForm
    {
        /// <summary>
        /// Erforderliche Designervariable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Verwendete Ressourcen bereinigen.
        /// </summary>
        /// <param name="disposing">True, wenn verwaltete Ressourcen gelöscht werden sollen; andernfalls False.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Vom Windows Form-Designer generierter Code

        /// <summary>
        /// Erforderliche Methode für die Designerunterstützung.
        /// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
        /// </summary>
        private void InitializeComponent()
        {
            this.MyButtonConnect = new System.Windows.Forms.Button();
            this.MyButtonDisconnect = new System.Windows.Forms.Button();
            this.MyComboBoxBaudRate = new System.Windows.Forms.ComboBox();
            this.MyComboBoxComPort = new System.Windows.Forms.ComboBox();
            this.MyLabelBaudRate = new System.Windows.Forms.Label();
            this.MyLabelComPort = new System.Windows.Forms.Label();
            this.MyComboBoxFileList = new System.Windows.Forms.ComboBox();
            this.MyTextBoxSelectedPath = new System.Windows.Forms.TextBox();
            this.MyButtonOpenFolder = new System.Windows.Forms.Button();
            this.MySaveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.MyLabelSelectFile = new System.Windows.Forms.Label();
            this.MyLabelSelectFolder = new System.Windows.Forms.Label();
            this.MyButtonStartDownload = new System.Windows.Forms.Button();
            this.MyTextBoxFileInfo = new System.Windows.Forms.TextBox();
            this.MyLabelFileInfo = new System.Windows.Forms.Label();
            this.MyProgressBar = new System.Windows.Forms.ProgressBar();
            this.MyButtonDeleteFile = new System.Windows.Forms.Button();
            this.MyButtonDeleteAllFiles = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // MyButtonConnect
            // 
            this.MyButtonConnect.Location = new System.Drawing.Point(87, 33);
            this.MyButtonConnect.Name = "MyButtonConnect";
            this.MyButtonConnect.Size = new System.Drawing.Size(75, 23);
            this.MyButtonConnect.TabIndex = 0;
            this.MyButtonConnect.Text = "Connect";
            this.MyButtonConnect.UseVisualStyleBackColor = true;
            this.MyButtonConnect.Click += new System.EventHandler(this.MyButtonConnect_Click);
            // 
            // MyButtonDisconnect
            // 
            this.MyButtonDisconnect.Enabled = false;
            this.MyButtonDisconnect.Location = new System.Drawing.Point(168, 33);
            this.MyButtonDisconnect.Name = "MyButtonDisconnect";
            this.MyButtonDisconnect.Size = new System.Drawing.Size(75, 23);
            this.MyButtonDisconnect.TabIndex = 1;
            this.MyButtonDisconnect.Text = "Disconnect";
            this.MyButtonDisconnect.UseVisualStyleBackColor = true;
            this.MyButtonDisconnect.Click += new System.EventHandler(this.MyButtonDisonnect_Click);
            // 
            // MyComboBoxBaudRate
            // 
            this.MyComboBoxBaudRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.MyComboBoxBaudRate.FormattingEnabled = true;
            this.MyComboBoxBaudRate.Items.AddRange(new object[] {
            "9600",
            "19200",
            "57600",
            "115200"});
            this.MyComboBoxBaudRate.Location = new System.Drawing.Point(87, 6);
            this.MyComboBoxBaudRate.Name = "MyComboBoxBaudRate";
            this.MyComboBoxBaudRate.Size = new System.Drawing.Size(121, 21);
            this.MyComboBoxBaudRate.TabIndex = 5;
            this.MyComboBoxBaudRate.SelectedValueChanged += new System.EventHandler(this.MyBaudRateComboBox_SelectedValueChanged);
            // 
            // MyComboBoxComPort
            // 
            this.MyComboBoxComPort.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.MyComboBoxComPort.FormattingEnabled = true;
            this.MyComboBoxComPort.Location = new System.Drawing.Point(319, 6);
            this.MyComboBoxComPort.Name = "MyComboBoxComPort";
            this.MyComboBoxComPort.Size = new System.Drawing.Size(121, 21);
            this.MyComboBoxComPort.TabIndex = 6;
            this.MyComboBoxComPort.SelectedValueChanged += new System.EventHandler(this.MyComboBoxComPort_SelectedValueChanged);
            // 
            // MyLabelBaudRate
            // 
            this.MyLabelBaudRate.AutoSize = true;
            this.MyLabelBaudRate.Location = new System.Drawing.Point(28, 9);
            this.MyLabelBaudRate.Name = "MyLabelBaudRate";
            this.MyLabelBaudRate.Size = new System.Drawing.Size(53, 13);
            this.MyLabelBaudRate.TabIndex = 7;
            this.MyLabelBaudRate.Text = "Baud rate";
            // 
            // MyLabelComPort
            // 
            this.MyLabelComPort.AutoSize = true;
            this.MyLabelComPort.Location = new System.Drawing.Point(261, 9);
            this.MyLabelComPort.Name = "MyLabelComPort";
            this.MyLabelComPort.Size = new System.Drawing.Size(52, 13);
            this.MyLabelComPort.TabIndex = 8;
            this.MyLabelComPort.Text = "COM port";
            // 
            // MyComboBoxFileList
            // 
            this.MyComboBoxFileList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.MyComboBoxFileList.Enabled = false;
            this.MyComboBoxFileList.FormattingEnabled = true;
            this.MyComboBoxFileList.Location = new System.Drawing.Point(87, 224);
            this.MyComboBoxFileList.Name = "MyComboBoxFileList";
            this.MyComboBoxFileList.Size = new System.Drawing.Size(353, 21);
            this.MyComboBoxFileList.TabIndex = 10;
            this.MyComboBoxFileList.DropDown += new System.EventHandler(this.MyComboBoxFileList_DropDown);
            this.MyComboBoxFileList.SelectedValueChanged += new System.EventHandler(this.MyComboBoxFile_SelectedValueChanged);
            // 
            // MyTextBoxSelectedPath
            // 
            this.MyTextBoxSelectedPath.Enabled = false;
            this.MyTextBoxSelectedPath.Location = new System.Drawing.Point(87, 251);
            this.MyTextBoxSelectedPath.Name = "MyTextBoxSelectedPath";
            this.MyTextBoxSelectedPath.Size = new System.Drawing.Size(353, 20);
            this.MyTextBoxSelectedPath.TabIndex = 11;
            this.MyTextBoxSelectedPath.TextChanged += new System.EventHandler(this.MyTextBoxSelectedPath_TextChanged);
            this.MyTextBoxSelectedPath.DoubleClick += new System.EventHandler(this.MyTextBoxSelectedPath_DoubleClick);
            // 
            // MyButtonOpenFolder
            // 
            this.MyButtonOpenFolder.Enabled = false;
            this.MyButtonOpenFolder.Location = new System.Drawing.Point(446, 249);
            this.MyButtonOpenFolder.Name = "MyButtonOpenFolder";
            this.MyButtonOpenFolder.Size = new System.Drawing.Size(33, 23);
            this.MyButtonOpenFolder.TabIndex = 12;
            this.MyButtonOpenFolder.Text = "...";
            this.MyButtonOpenFolder.UseVisualStyleBackColor = true;
            this.MyButtonOpenFolder.Click += new System.EventHandler(this.MyButtonOpenFolder_Click);
            // 
            // MySaveFileDialog
            // 
            this.MySaveFileDialog.RestoreDirectory = true;
            // 
            // MyLabelSelectFile
            // 
            this.MyLabelSelectFile.AutoSize = true;
            this.MyLabelSelectFile.Location = new System.Drawing.Point(4, 227);
            this.MyLabelSelectFile.Name = "MyLabelSelectFile";
            this.MyLabelSelectFile.Size = new System.Drawing.Size(77, 13);
            this.MyLabelSelectFile.TabIndex = 13;
            this.MyLabelSelectFile.Text = "Select Log File";
            // 
            // MyLabelSelectFolder
            // 
            this.MyLabelSelectFolder.AutoSize = true;
            this.MyLabelSelectFolder.Location = new System.Drawing.Point(12, 254);
            this.MyLabelSelectFolder.Name = "MyLabelSelectFolder";
            this.MyLabelSelectFolder.Size = new System.Drawing.Size(69, 13);
            this.MyLabelSelectFolder.TabIndex = 14;
            this.MyLabelSelectFolder.Text = "Select Folder";
            // 
            // MyButtonStartDownload
            // 
            this.MyButtonStartDownload.Enabled = false;
            this.MyButtonStartDownload.Location = new System.Drawing.Point(340, 277);
            this.MyButtonStartDownload.Name = "MyButtonStartDownload";
            this.MyButtonStartDownload.Size = new System.Drawing.Size(100, 23);
            this.MyButtonStartDownload.TabIndex = 15;
            this.MyButtonStartDownload.Text = "Start Download";
            this.MyButtonStartDownload.UseVisualStyleBackColor = true;
            this.MyButtonStartDownload.Click += new System.EventHandler(this.MyButtonStartDownload_Click);
            // 
            // MyTextBoxFileInfo
            // 
            this.MyTextBoxFileInfo.BackColor = System.Drawing.SystemColors.ButtonHighlight;
            this.MyTextBoxFileInfo.Enabled = false;
            this.MyTextBoxFileInfo.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MyTextBoxFileInfo.Location = new System.Drawing.Point(87, 62);
            this.MyTextBoxFileInfo.Multiline = true;
            this.MyTextBoxFileInfo.Name = "MyTextBoxFileInfo";
            this.MyTextBoxFileInfo.ReadOnly = true;
            this.MyTextBoxFileInfo.Size = new System.Drawing.Size(353, 156);
            this.MyTextBoxFileInfo.TabIndex = 16;
            // 
            // MyLabelFileInfo
            // 
            this.MyLabelFileInfo.AutoSize = true;
            this.MyLabelFileInfo.Location = new System.Drawing.Point(37, 65);
            this.MyLabelFileInfo.Name = "MyLabelFileInfo";
            this.MyLabelFileInfo.Size = new System.Drawing.Size(44, 13);
            this.MyLabelFileInfo.TabIndex = 17;
            this.MyLabelFileInfo.Text = "File Info";
            // 
            // MyProgressBar
            // 
            this.MyProgressBar.Location = new System.Drawing.Point(87, 306);
            this.MyProgressBar.Name = "MyProgressBar";
            this.MyProgressBar.Size = new System.Drawing.Size(353, 23);
            this.MyProgressBar.TabIndex = 19;
            // 
            // MyButtonDeleteFile
            // 
            this.MyButtonDeleteFile.Enabled = false;
            this.MyButtonDeleteFile.Location = new System.Drawing.Point(213, 277);
            this.MyButtonDeleteFile.Name = "MyButtonDeleteFile";
            this.MyButtonDeleteFile.Size = new System.Drawing.Size(100, 23);
            this.MyButtonDeleteFile.TabIndex = 20;
            this.MyButtonDeleteFile.Text = "Delete File";
            this.MyButtonDeleteFile.UseVisualStyleBackColor = true;
            this.MyButtonDeleteFile.Click += new System.EventHandler(this.MyButtonDeleteFile_Click);
            // 
            // MyButtonDeleteAllFiles
            // 
            this.MyButtonDeleteAllFiles.Enabled = false;
            this.MyButtonDeleteAllFiles.Location = new System.Drawing.Point(87, 277);
            this.MyButtonDeleteAllFiles.Name = "MyButtonDeleteAllFiles";
            this.MyButtonDeleteAllFiles.Size = new System.Drawing.Size(100, 23);
            this.MyButtonDeleteAllFiles.TabIndex = 21;
            this.MyButtonDeleteAllFiles.Text = "Delete All Files";
            this.MyButtonDeleteAllFiles.UseVisualStyleBackColor = true;
            this.MyButtonDeleteAllFiles.Click += new System.EventHandler(this.MyButtonDeleteAllFiles_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 347);
            this.Controls.Add(this.MyButtonDeleteAllFiles);
            this.Controls.Add(this.MyButtonDeleteFile);
            this.Controls.Add(this.MyProgressBar);
            this.Controls.Add(this.MyLabelFileInfo);
            this.Controls.Add(this.MyTextBoxFileInfo);
            this.Controls.Add(this.MyButtonStartDownload);
            this.Controls.Add(this.MyLabelSelectFolder);
            this.Controls.Add(this.MyLabelSelectFile);
            this.Controls.Add(this.MyButtonOpenFolder);
            this.Controls.Add(this.MyTextBoxSelectedPath);
            this.Controls.Add(this.MyComboBoxFileList);
            this.Controls.Add(this.MyLabelComPort);
            this.Controls.Add(this.MyLabelBaudRate);
            this.Controls.Add(this.MyComboBoxComPort);
            this.Controls.Add(this.MyComboBoxBaudRate);
            this.Controls.Add(this.MyButtonDisconnect);
            this.Controls.Add(this.MyButtonConnect);
            this.MaximumSize = new System.Drawing.Size(500, 385);
            this.MinimumSize = new System.Drawing.Size(500, 385);
            this.Name = "MainForm";
            this.ShowIcon = false;
            this.Text = "PX4FileDownloader";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.CloseAppication);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button MyButtonConnect;
        private System.Windows.Forms.Button MyButtonDisconnect;
        private System.Windows.Forms.ComboBox MyComboBoxBaudRate;
        private System.Windows.Forms.ComboBox MyComboBoxComPort;
        private System.Windows.Forms.Label MyLabelBaudRate;
        private System.Windows.Forms.Label MyLabelComPort;
        private System.Windows.Forms.ComboBox MyComboBoxFileList;
        private System.Windows.Forms.TextBox MyTextBoxSelectedPath;
        private System.Windows.Forms.Button MyButtonOpenFolder;
        private System.Windows.Forms.SaveFileDialog MySaveFileDialog;
        private System.Windows.Forms.Label MyLabelSelectFile;
        private System.Windows.Forms.Label MyLabelSelectFolder;
        private System.Windows.Forms.Button MyButtonStartDownload;
        private System.Windows.Forms.TextBox MyTextBoxFileInfo;
        private System.Windows.Forms.Label MyLabelFileInfo;
        private System.Windows.Forms.ProgressBar MyProgressBar;
        private System.Windows.Forms.Button MyButtonDeleteFile;
        private System.Windows.Forms.Button MyButtonDeleteAllFiles;
    }
}

