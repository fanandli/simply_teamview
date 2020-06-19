using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using ClientServerBridger.Common;
using ClientServerBridger.CSBModels;

namespace ClientServerBridger
{
    public partial class RemoteConnector : UserControl
    {
        //private string HostName = "";
        public string MainframeName
        {
            get;set;
        }
        public int MainframePort
        {
            get;set;
        }
        public string MainframeIP
        {
            get;set;
        }
        public Dictionary<string, int> MainframeIPPort
        {
            get;set;
        }
        public delegate void RemoteIPConfigTransmit(string IP);
        public delegate void RemotePortConfigTransmit(int Port);

        public RemoteConnector()
        {
            InitializeComponent();
        }

        private void BtnConnection_Click(object sender, EventArgs e)
        {
            RemoteIPConfigTransmit remoteIPConfigTransmit = new RemoteIPConfigTransmit(RemoteWindow.GetIPConfig);
            remoteIPConfigTransmit(MainframeIP);
            RemotePortConfigTransmit remotePortConfigTransmit = new RemotePortConfigTransmit(RemoteWindow.GetPortConfig);
            remotePortConfigTransmit(MainframePort);
            RemoteWindow remoteWindow = new RemoteWindow();
            remoteWindow.Show();
        }

        private void RemoteConnector_Load(object sender, EventArgs e)
        {
            BusinessLogic businessLogic = new BusinessLogic();

            this.LabelRemoteInfo.Text = this.MainframeName;
        }

        private void LabelRemoteInfo_Click(object sender, EventArgs e)
        {
            LogicCommon logicCommon = new LogicCommon();
            logicCommon.SocketListen();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //LogicCommon logicCommon = new LogicCommon();
            //logicCommon.GetMessage(this.textBox1);
        }
    }
}