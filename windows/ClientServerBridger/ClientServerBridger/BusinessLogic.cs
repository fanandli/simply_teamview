using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Drawing;
using System.Runtime.Serialization.Json;
using Newtonsoft.Json.Linq;

using ClientServerBridger.Common;
using ClientServerBridger.CSBModels;

namespace ClientServerBridger
{
    public class BusinessLogic
    {
        public void ServerConnection()
        {

        }

        /// <summary>
        /// 本地用户id读取
        /// </summary>
        /// <param name="Path">本地文件路径</param>
        public string LocalUserIdImport(string Path)
        {
            try
            {
                StreamReader UserIdReader = new StreamReader(Path, Encoding.Default);
                string content = UserIdReader.ReadLine();
                return content;
            }
            catch (Exception ex)
            {
                LogHelper.WriteLog(ex.Message.ToString(), ex);
                return string.Empty;
            }
        }

        //public delegate void RemoteConfigTransmit(string Ip, string RUsername, string RPassword);
        //public static void GetRemoteConfig(string Ip, string RUsername, string RPassword)
        //{
        //    RemoteConfigTransmit remoteConfigTransmit = new RemoteConfigTransmit(BusinessLogic.GetRemoteConfig);
        //    remoteConfigTransmit("192.168.119.128", "Administrator", "123");
        //}

        /// <summary>
        /// 网页返回IP
        /// </summary>
        /// <param name="WebAddress">待处理网页地址</param>
        /// <returns></returns>
        public string ReturnIP()
        {
            string WebAddress = webAddress;
            LogicCommon logicCommon = new LogicCommon();
            DataContractJsonSerializer ser1 = new DataContractJsonSerializer(typeof(WebJsonModel));
            using (MemoryStream memoryStream = new MemoryStream(Encoding.Unicode.GetBytes(logicCommon.JsonToString(WebAddress))))
            {
                WebJsonModel webJsonModel = ser1.ReadObject(memoryStream) as WebJsonModel;
                return webJsonModel.ip;
            }
        }

        /// <summary>
        /// 网页返回port
        /// </summary>
        /// <param name="WebAddress">待处理网页地址</param>
        /// <returns></returns>
        public int ReturnPort()
        {
            string WebAddress = webAddress;
            LogicCommon logicCommon = new LogicCommon();
            DataContractJsonSerializer ser1 = new DataContractJsonSerializer(typeof(WebJsonModel));
            using (MemoryStream memoryStream = new MemoryStream(Encoding.Unicode.GetBytes(logicCommon.JsonToString(WebAddress))))
            {
                WebJsonModel webJsonModel = ser1.ReadObject(memoryStream) as WebJsonModel;
                return webJsonModel.port;
            }
        }

        private string webAddress = "http://netbridge.vip/getappsrv.php?cname=" + RouterName;

        private static string RouterName;
        public static void GetRouterName(string routerName)
        {
            RouterName = routerName;
        }

        /// <summary>
        /// 返回主机数量
        /// </summary>
        /// <returns></returns>
        public int MainframeCount()
        {
            LogicCommon logicCommon = new LogicCommon();
            JObject jObject = JObject.Parse(logicCommon.GetMessage());
            string JSONParameters = jObject["DoMain"].ToString();
            var objParameters = JArray.Parse(JSONParameters);
            dynamic parsedObject = objParameters;
            return parsedObject.Count;
        }

        /// <summary>
        /// 返回主机信息（字典格式）
        /// </summary>
        /// <returns>主机信息字典</returns>
        public IDictionary<int, string> MainframeInfo()
        {
            LogicCommon logicCommon = new LogicCommon();
            Dictionary<int, string> dict = new Dictionary<int, string>();
            foreach (var item in MainframeName())
            {
                if (item != null)
                {
                    JObject objJ = JObject.Parse(logicCommon.GetMessage(Msg.NewQueryServiceAddressMsg(item, "rdp")));
                    string ip = objJ.Value<string>("ip");
                    int port = objJ.Value<int>("port");
                    dict.Add(port, ip);
                }
            }
            return dict;
        }

        /// <summary>
        /// 返回Json查询名称
        /// </summary>
        /// <returns>Json查询名称列表</returns>
        public List<string> MainframeName()
        {
            LogicCommon logicCommon = new LogicCommon();
            List<string> list = new List<string>();
            JObject objJ = JObject.Parse(logicCommon.GetMessage());
            JArray JSONParameters = objJ.Value<JArray>("clients");
            JArray jArray = JArray.Parse(JSONParameters.ToString());
            foreach (var jn in jArray)
            {
                if (jn.ToString().Contains("mac"))
                {
                    string name = jn.Value<string>("defaulthost");
                    list.Add(name);
                }
            }
            return list;
        }

        /// <summary>
        /// 返回主机名称
        /// </summary>
        /// <returns>主机名称列表</returns>
        public List<string> MainframeCName()
        {
            LogicCommon logicCommon = new LogicCommon();
            List<string> list = new List<string>();
            JObject objJ = JObject.Parse(logicCommon.GetMessage());
            JArray JSONParameters = objJ.Value<JArray>("clients");
            JArray jArray = JArray.Parse(JSONParameters.ToString());
            foreach (var jn in jArray)
            {
                if (jn.ToString().Contains("mac"))
                {
                    string name = jn.Value<string>("clientname");
                    list.Add(name);
                }
            }
            return list;
        }
    }
}
