using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Windows.Forms;

using ClientServerBridger.Common;
using ClientServerBridger.CSBModels;

namespace ClientServerBridger
{
    public partial class MainForm : Form
    {
        public delegate void GetRadiusEventHandler(int Radius);

        public MainForm()
        {

            InitializeComponent();
            CheckForIllegalCrossThreadCalls = false;

            GetRadiusEventHandler GR = new GetRadiusEventHandler(FormCommon.GetRadius);
            GR(40);

            FormCommon FC = new FormCommon();
            this.Region = FC.Round(this.Width, this.Height);
        }

        private void PicClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            try
            {
                BusinessLogic businessLogic = new BusinessLogic();
                List<string> listCName = businessLogic.MainframeCName();
                int modx = 0;
                int mody = 0;
                int mName = 0;
                foreach (KeyValuePair<int, string> kvp in businessLogic.MainframeInfo())
                {
                    if (modx <= 4)
                    {
                        RemoteConnector remoteConnector = new RemoteConnector
                        {
                            Location = new Point(80 + modx * 250, 80 + mody * 250),
                            Visible = true
                        };
                        remoteConnector.MainframeName = listCName[mName];
                        remoteConnector.MainframePort = kvp.Key;
                        remoteConnector.MainframeIP = kvp.Value;
                        PanelBody.Controls.Add(remoteConnector);
                        modx++;
                    }
                    else
                    {
                        RemoteConnector remoteConnector = new RemoteConnector
                        {
                            Location = new Point(80 + modx % 5 * 250, 80 + mody * 250),
                            Visible = true
                        };
                        remoteConnector.MainframeName = listCName[mName];
                        remoteConnector.MainframePort = kvp.Key;
                        remoteConnector.MainframeIP = kvp.Value;
                        PanelBody.Controls.Add(remoteConnector);
                        modx++;
                    }
                    if(modx % 5 == 0)
                    {
                        mody++;
                    }
                    mName++;
                }
            }
            catch(Exception ex)
            {
                LogHelper.WriteLog(ex.ToString());
            }
        }

        private void PanelBody_MouseDown(object sender, MouseEventArgs e)
        {
            FormCommon.ReleaseCapture();
            FormCommon.SendMessage(this.Handle, 0x0112, 0xF012, 0);
        }

        private void PicMin_Click(object sender, EventArgs e)
        {
            this.WindowState = FormWindowState.Minimized;
        }
    }
}