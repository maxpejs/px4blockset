using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace PX4FileDownloader
{

    class FileInfo
    {
        internal String FileName { get; set; }
        public FileInfo(String info)
        {

        }
    }

    class Helper
    {
        internal static List<List<String>> GetFileListFromPX4(SerialPort port)
        {
            List<List<String>> ret = new List<List<String>>();
            Stopwatch sw = new Stopwatch();

            // create six lists for file informations
            for (int i = 0; i < 6; i++)
            {
                ret.Add(new List<String>());
            }

            try
            {
                if (port.IsOpen)
                {
                    port.DiscardOutBuffer();
                    port.DiscardInBuffer();

                    port.Write("list all\r\n");

                    // parse received answer
                    String indata = "";
                    sw.Start();

                    do
                    {
                        indata += port.ReadExisting();
                    } while (!indata.Contains("--- file list end ---\r\n") && sw.ElapsedMilliseconds < 2000);

                    indata = indata.Replace("--- file list start ---\r\n", "");
                    indata = indata.Replace("--- file list end ---\r\n", "");

                    String[] files = Regex.Split(indata, "\r\n");

                    foreach (String s in files)
                    {
                        if (s.Length > 0)
                        {
                            String[] parts = s.Split('|');

                            if (parts.Length == 6)
                            {
                                parts[0] = parts[0].Replace("/", "");
                                // copy parts to lists 
                                for (int i = 0; i < 6; i++)
                                {
                                    ret[i].Add(parts[i]);
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "Exception", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            return ret;
        }


        internal static void DownloadFile(SerialPort port, String filename, String filepath, ProgressBar bar, int max)
        {
            Stopwatch sw = new Stopwatch();
            String indata = "";
            String start_match = "=== BEGIN ===\r\n";
            String end_match = "=== END ===";
            int timeout = int.MaxValue;

            bool end_found = false;

            bar.Minimum = 0;
            bar.Maximum = max;
            bar.Value = 0;

            try
            {
                if (port.IsOpen)
                {
                    port.DiscardOutBuffer();
                    port.DiscardInBuffer();

                    // send cmd for listing file
                    port.Write("list " + filename + "\r\n");

                    sw.Start();

                    // ====================================================================
                    // match start
                    do
                    {
                        indata += port.ReadExisting();
                        if (!port.IsOpen)
                        {
                            MessageBox.Show("COM port wurde geschlossen", "Abgebrochen", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }

                        Application.DoEvents();

                    } while (!indata.Contains(start_match) && sw.ElapsedMilliseconds < timeout);

                    if (sw.ElapsedMilliseconds >= timeout)
                    {
                        MessageBox.Show("Timeout at matching start", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }

                    indata = indata.Replace(start_match, "");

                    bar.Value = bar.Maximum / 10;

                    // create new file
                    File.WriteAllText(filepath, "");

                    sw.Restart();

                    // ====================================================================
                    // get rest data
                    do
                    {
                        indata += port.ReadExisting();

                        String[] lines = Regex.Split(indata, "\r\n");

                        if (lines.Length > 0)
                        {
                            for (int i = 0; i < (lines.Length - 1); i++)
                            {
                                if (end_found)
                                { // write back

                                }
                                else if (!lines[i].Contains(end_match))
                                {
                                    bar.Value = (bar.Value >= bar.Maximum) ? bar.Maximum : bar.Value + 1;
                                    File.AppendAllText(filepath, lines[i] + "\r\n");
                                }
                                else
                                {
                                    end_found = true;
                                    bar.Value = bar.Maximum;
                                }
                            }

                            indata = lines[lines.Length - 1];
                        }


                        Application.DoEvents();

                        if (!port.IsOpen)
                        {
                            MessageBox.Show("COM port wurde geschlossen", "Abgebrochen", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }

                    } while (!end_found && sw.ElapsedMilliseconds < timeout);

                    if (sw.ElapsedMilliseconds >= timeout)
                    {
                        MessageBox.Show("Timeout at matching start", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }

                    bar.Value = bar.Minimum;
                    MessageBox.Show("Download successful", "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);

                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "Exception", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }

        internal static void DeleteFile(SerialPort port, String filename)
        {
            try
            {
                if (port.IsOpen)
                {
                    port.DiscardOutBuffer();
                    port.DiscardInBuffer();

                    // send cmd for listing file
                    port.Write("del " + filename + "\r\n");

                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "Exception", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        internal static void DeleteAllFiles(SerialPort port)
        {
            try
            {
                if (port.IsOpen)
                {
                    port.DiscardOutBuffer();
                    port.DiscardInBuffer();

                    // send cmd for listing file
                    port.Write("del all\r\n");

                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "Exception", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
