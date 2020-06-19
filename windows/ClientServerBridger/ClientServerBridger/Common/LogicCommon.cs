using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using System.Net.NetworkInformation;
using System.Windows.Forms;
using System.Runtime.Serialization.Json;
using System.IO;

using ClientServerBridger.CSBModels;

namespace ClientServerBridger.Common
{
    /// <summary>
    /// 逻辑通用类
    /// </summary>
    class LogicCommon
    {

        #region IP端口通用设置

        /// <summary>
        /// 获取本地IP
        /// </summary>
        /// <returns></returns>
        public static string GetLocalIP()
        {
            try
            {
                string name = Dns.GetHostName();
                IPAddress[] ipadrlist = Dns.GetHostAddresses(name);
                foreach (IPAddress ipa in ipadrlist)
                {
                    if (ipa.AddressFamily == AddressFamily.InterNetwork)
                    {
                        return ipa.ToString();
                    }
                }
                return "未查询到IP地址";
            }
            catch
            {
                return "IP地址查询异常";
            }

        }

        /// <summary>
        /// IP地址判断
        /// </summary>
        /// <param name="ip">IP</param>
        /// <returns></returns>
        private string IpFormat(string ip)
        {
            Match m = Regex.Match(ip, @"(\d|[1-9]\d|1\d{2}|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d{2}|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d{2}|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d{2}|2[0-4]\d|25[0-5])");
            if (!m.Success)
            {
                return "IP格式不正确";
            }
            else
            {
                return ip;
            }
        }

        /// <summary>
        /// 获取本地可用端口号
        /// </summary>
        /// <param name="startPort">开始端口</param>
        /// <returns></returns>
        private object GetLocalAvailablePort(int startPort)
        {
            const string PortReleaseGuid = "8875BD8E-4D5B-11DE-B2F4-691756D89593";
            int port = startPort;
            bool isAvailable = true;

            var mutex = new Mutex(false, string.Concat("Global/", PortReleaseGuid));
            mutex.WaitOne();
            try
            {
                IPGlobalProperties ipGlobalProperties = IPGlobalProperties.GetIPGlobalProperties();
                IPEndPoint[] endPoints = ipGlobalProperties.GetActiveTcpListeners();
                do
                {
                    if (!isAvailable)
                    {
                        port++;
                        isAvailable = true;
                    }
                    foreach (IPEndPoint endPoint in endPoints)
                    {
                        if (endPoint.Port != port) 
                            continue;
                        isAvailable = false;
                        break;
                    }
                }
                while (!isAvailable && port < IPEndPoint.MaxPort);
                if (!isAvailable)
                {
                    throw new ApplicationException("");
                }
                return port;
            }
            catch(Exception ex)
            {
                return ex.ToString();
            }
            finally
            {
                mutex.ReleaseMutex();
            }
        }

        /// <summary>
        /// 端口号格式判断
        /// </summary>
        /// <param name="port">端口号</param>
        /// <returns></returns>
        private string PortFormat(string port)
        {
            Match m = Regex.Match(port, @"^[1-9]$|(^[1-9][0-9]$)|(^[1-9][0-9][0-9]$)|(^[1-9][0-9][0-9][0-9]$)|(^[1-6][0-5][0-5][0-3][0-5]$)");
            if (!m.Success)
            {
                return "端口号格式不正确";
            }
            else
            {
                return port;
            }
        }
        #endregion

        #region Socket通用设置

        /// <summary>
        /// Socket连接
        /// </summary>
        public void SocketConnect()
        {
            try
            {
                SocketModel socketModel = new SocketModel()
                {
                    Socketm = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp)
                };
                IPAddress address = IPAddress.Parse(socketModel.Ip);
                IPEndPoint endPoint = new IPEndPoint(address, socketModel.Port);
                socketModel.Socketm.Connect(endPoint);

                //Thread ConnectContent = new Thread(DoWork);
                //ConnectContent.Start();
            }
            catch(Exception ex)
            {
                LogHelper.WriteLog(ex.ToString());
            }
        }

        /// <summary>
        /// Socket本地连接
        /// </summary>
        /// <param name="port">端口号</param>
        public void SocketConnect(int port)
        {
            try
            {
                SocketModel socketModel = new SocketModel(port)
                {
                    Socketm = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp)
                };
                IPAddress address = IPAddress.Parse(socketModel.Ip);
                IPEndPoint endPoint = new IPEndPoint(address, port);
                socketModel.Socketm.Connect(endPoint);
            }
            catch(Exception ex)
            {
                LogHelper.WriteLog(ex.ToString());
            }
        }

        /// <summary>
        /// Socket监听
        /// </summary>
        public void SocketListen()
        {
            int Port = 8888;
            try
            {
                SocketModel socketModel = new SocketModel(Port)
                {
                    Socketm = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp)
                };
                IPAddress iPAddress = IPAddress.Parse(socketModel.Ip);
                IPEndPoint iPEndPoint = new IPEndPoint(iPAddress, socketModel.Port);
                socketModel.Socketm.Bind(iPEndPoint);
                socketModel.Socketm.Listen(int.MaxValue);

                //Thread ListenContent = new Thread(DoWork);
                //ListenContent.Start();
            }
            catch (Exception)
            {
                throw;
            }
        }

        /// <summary>
        /// 发送消息
        /// </summary>
        /// <param name="socket">Socket</param>
        public void SocketListen(Socket socket)
        {
            SocketConnect();
            try
            {
                while (true)
                {
                    //SocketModel socketModel = new SocketModel();
                    Socket clientSocket = socket.Accept();
                    clientSocket.Send(Encoding.UTF8.GetBytes("服务端发送消息"));
                    //Thread thread = new Thread(SocketConnect);
                    //thread.Start(clientSocket);
                }
            }
            catch (Exception ex)        
            {
                LogHelper.WriteLog(ex.ToString());
            }

        }

        /// <summary>
        /// 根据请求获取服务端数据
        /// </summary>
        /// <param name="bt"></param>
        /// <returns></returns>
        public string GetMessage(byte[] bt)
        {
            Socket _socket = null;
            try
            {
                SocketModel socketModel = new SocketModel();
                BusinessLogic businessLogic = new BusinessLogic();

                _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPAddress address = IPAddress.Parse(businessLogic.ReturnIP());
                IPEndPoint endPoint = new IPEndPoint(address, businessLogic.ReturnPort());
                _socket.Connect(endPoint);

                _socket.Send(bt);

                int length = _socket.Receive(socketModel.Buffer);
                //textBox.Text = Encoding.UTF8.GetString(socketModel.Buffer, 0, length);
                return Msg.GetJsonString(socketModel.Buffer);
            }
            catch (Exception ex)
            {
                _socket.Shutdown(SocketShutdown.Both);
                _socket.Close();
                LogHelper.WriteLog(ex.ToString());
                return null;
            }
        }

        /// <summary>
        /// 获取服务端defaulthost数据
        /// </summary>
        /// <returns></returns>
        public string GetMessage()
        {
            Socket _socket = null;
            try
            {
                SocketModel socketModel = new SocketModel();
                BusinessLogic businessLogic = new BusinessLogic();

                _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPAddress address = IPAddress.Parse(businessLogic.ReturnIP());
                IPEndPoint endPoint = new IPEndPoint(address, businessLogic.ReturnPort());
                _socket.Connect(endPoint);

                _socket.Send(Msg.NewQueryClientsMsg());
                //_socket.Send(Msg.NewQueryServiceAddressMsg("b42e99eee25e.2076ffffff934effffffa0ffffffe3.wrs.netbridge.vip", "rdp"));

                int length = _socket.Receive(socketModel.Buffer);
                //textBox.Text = Encoding.UTF8.GetString(socketModel.Buffer, 0, length);
                return Msg.GetJsonString(socketModel.Buffer);
            }
            catch (Exception ex)
            {
                _socket.Shutdown(SocketShutdown.Both);
                _socket.Close();
                LogHelper.WriteLog(ex.ToString());
                return null;
            }
        }
        #endregion

        #region 通信数据处理

        /// <summary>
        /// Json转换String
        /// </summary>
        /// <param name="WebAddress">待处理网页地址</param>
        /// <returns></returns>
        public string JsonToString(string WebAddress)
        {
            WebClient MyWebClient = new WebClient
            {
                Credentials = CredentialCache.DefaultCredentials
            };
            return MyWebClient.DownloadString(WebAddress);
        }

        /// <summary>
        /// Json转换String获取IP
        /// </summary>
        /// <param name="WebAddress">待处理网页地址</param>
        public string JsonToStringIP(string WebAddress)
        {
            WebClient MyWebClient = new WebClient
            {
                Credentials = CredentialCache.DefaultCredentials
            };
            string a = MyWebClient.DownloadString(WebAddress);

            DataContractJsonSerializer ser1 = new DataContractJsonSerializer(typeof(WebJsonModel));
            using (MemoryStream ms = new MemoryStream(Encoding.Unicode.GetBytes(a)))
            {
                WebJsonModel foo1 = ser1.ReadObject(ms) as WebJsonModel;
                return foo1.ip;
            }
        }

        /// <summary>
        /// Json转换String获取端口
        /// </summary>
        /// <param name="WebAddress">待处理网页地址</param>
        /// <returns></returns>
        public int JsonToStringPort(string WebAddress)
        {
            WebClient MyWebClient = new WebClient
            {
                Credentials = CredentialCache.DefaultCredentials
            };
            string a = MyWebClient.DownloadString(WebAddress);

            DataContractJsonSerializer ser1 = new DataContractJsonSerializer(typeof(WebJsonModel));
            using (MemoryStream ms = new MemoryStream(Encoding.Unicode.GetBytes(a)))
            {
                WebJsonModel foo1 = ser1.ReadObject(ms) as WebJsonModel;
                return foo1.port;
            }
        }
        #endregion

    }

    #region 信息处理类

    public class Msg
    {
        public static void _pack_subtlv(List<byte> _buf, byte subtlvtype, List<byte> value)
        {
            int pos = _buf.Count;
            _buf.AddRange(value);

            short len = (short)value.Count;
            if (subtlvtype % 2 == 0)
            {
                _buf.Add(0);
                len += 1;
            }
            _buf.InsertRange(pos, new List<byte> { subtlvtype, 0, (byte)(len >> 8), (byte)(len & 0x00FF) });
        }

        public static string GetJsonString(byte[] msg)
        {
            int msglen = (msg[2] << 8) + msg[3] + 4;
            int i = 4;
            while (i < msglen)
            {
                int len = (msg[i + 2] << 8) + msg[i + 3];
                if (msg[i] == 100)
                {
                    return System.Text.Encoding.Default.GetString(msg.Skip(i + 4).Take(len - 1).ToArray<byte>());
                }

                i += 4 + len;
            }

            return null;
        }

        public static byte[] NewQueryClientsMsg()
        {
            List<byte> _buf = new List<byte>();
            _pack_subtlv(_buf, 100, System.Text.Encoding.Default.GetBytes("{\"msgtype\":\"getcfg\"}").ToList<byte>());
            _pack_subtlv(_buf, 102, System.Text.Encoding.Default.GetBytes("synheader").ToList<byte>());

            short len = (short)_buf.Count;
            _buf.InsertRange(0, new List<byte> { 0x88, 0, (byte)(len >> 8), (byte)(len & 0x00FF) });
            return _buf.ToArray();
        }

        public static byte[] NewQueryServiceAddressMsg(string hostname, string service)
        {
            List<byte> _buf = new List<byte>();
            string jsostr = "{\"msgtype\":\"QUERYSVCADDR\", \"domain\":\"" + hostname + "\", \"svctype\":\"" + service + "\"}";
            _pack_subtlv(_buf, 100, System.Text.Encoding.Default.GetBytes(jsostr).ToList<byte>());
            _pack_subtlv(_buf, 102, System.Text.Encoding.Default.GetBytes("synheader").ToList<byte>());

            short len = (short)_buf.Count;
            _buf.InsertRange(0, new List<byte> { 0x88, 0, (byte)(len >> 8), (byte)(len & 0x00FF) });
            return _buf.ToArray();
        }
    }
    #endregion

}