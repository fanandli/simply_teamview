using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace ClientServerBridger.CSBModels
{
    class SocketModel
    {

        #region 成员属性

        private string _ip = string.Empty;
        private int _port = 0;
        private Socket _socket = null;
        private byte[] buffer = new byte[1024 * 1024 * 2];

        public string Ip { get => _ip; set => _ip = value; }
        public int Port { get => _port; set => _port = value; }
        public Socket Socketm { get => _socket; set => _socket = value; }
        public byte[] Buffer { get => buffer; set => buffer = value; }
        #endregion

        #region 构造方法

        /// <summary>
        /// 构造函数
        /// </summary>
        public SocketModel()
        {

        }

        public SocketModel(string ip, int port)
        {
            this._ip = ip;
            this._port = port;
        }

        public SocketModel(int port)
        {
            this._ip = "222.95.4.144";
            this._port = port;
        }
        #endregion

    }
}
