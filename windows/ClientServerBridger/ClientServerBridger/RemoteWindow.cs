using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MSTSCLib;

using ClientServerBridger.CSBModels;
using ClientServerBridger.Common;

namespace ClientServerBridger
{
    public partial class RemoteWindow : Form
    {
        public RemoteWindow()
        {
            InitializeComponent();
            CheckForIllegalCrossThreadCalls = false; //允许跨线程操作此窗口

            //GetControlEventHandler getControlEventHandler = new GetControlEventHandler(FormCommon.GetControl);
            //getControlEventHandler(this.PanelMenuShow);
        }

        
        private void RemoteWindow_Load(object sender, EventArgs e)
        {
            //LogicCommon logicCommon = new LogicCommon();
            this.WindowRemote.Server = IP;
            this.WindowRemote.AdvancedSettings2.RDPPort = Port;
            this.WindowRemote.AdvancedSettings7.EnableCredSspSupport = true;
            //this.WindowRemote.UserName = "Administrator";
            //IMsTscNonScriptable secured = (IMsTscNonScriptable)this.WindowRemote.GetOcx();
            //secured.ClearTextPassword = "123456";
            this.WindowRemote.Connect();
            //TimerMenu.Start();
            
        }

        static string IP;
        static int Port;

        public static void GetIPConfig(string ip)
        {
            IP = ip;
        }
        public static void GetPortConfig(int port)
        {
            Port = port;
        }

        //static string Config;

        //public static void GetConfig(string config)
        //{
        //    Config = config;
        //}

        //private void TimerMenu_Tick(object sender, EventArgs e)
        //{
        //    if (Cursor.Position.Y - this.Top < 50)
        //    {
        //        this.PanelMenuShow.Show();
        //        //timer1.Start();
        //        //PanelShow();
        //        //this.PanelMenuShow.Visible = true;

        //        //FormCommon formCommon = new FormCommon();
        //        //formCommon.TimerCreate();
        //    }
        //    else
        //    {
        //        this.PanelMenuShow.Hide();
        //    }
        //}

        //private void timer1_Tick(object sender, EventArgs e)
        //{
        //    int i = 2;
        //    int y = this.PanelMenuShow.Height;
        //    this.PanelMenuShow.Height = y / i;
        //    i = i + 1;
        //}

        private void PanelShow()
        {
            FormCommon formCommon = new FormCommon();
            formCommon.TimerCreate();
        }

        public delegate void GetControlEventHandler(Control control);
    }
}
