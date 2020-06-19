using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Json;
using Newtonsoft.Json.Linq;

using ClientServerBridger.CSBModels;
using ClientServerBridger.Common;

namespace ClientServerBridger
{
    public partial class LoginForm : Form
    {
        Dictionary<string, ClientModel> Clients = new Dictionary<string, ClientModel>();
        public delegate void GetRadiusEventHandler(int Radius);
        public delegate void TransRouterName(string RouterName);

        public LoginForm()
        {
            InitializeComponent();

            // 通过委托传递圆角弧度
            GetRadiusEventHandler getRadiusEventHandler = new GetRadiusEventHandler(FormCommon.GetRadius);
            getRadiusEventHandler(40);
            //base.Refresh();
            FormCommon formCommon = new FormCommon();
            this.Region = formCommon.Round(this.Width, this.Height);
        }

        private void LoginForm_Paint(object sender, PaintEventArgs e)
        {
            //Graphics g = e.Graphics;
            //LinearGradientBrush myBrush = new LinearGradientBrush(this.ClientRectangle, Color.White, Color.Gray, LinearGradientMode.Vertical);
            //g.FillRectangle(myBrush, this.ClientRectangle);
        }
        
        private void LoginForm_MouseDown(object sender, MouseEventArgs e)
        {
            FormCommon.ReleaseCapture();
            FormCommon.SendMessage(this.Handle, 0x0112, 0xF012, 0);
        }
        
        private void LoginForm_Load(object sender, EventArgs e)
        {
            #region 读取bin

            //FormCommon formCommon = new FormCommon();
            //formCommon.ReadBin(this.ComboBoxUserId, this.TextBoxUserPwd, this.CheckBoxRememberPwd);

            FileStream fileStream = new FileStream("data.bin", FileMode.OpenOrCreate);
            if (fileStream.Length > 0)
            {
                BinaryFormatter binaryFormatter = new BinaryFormatter();
                Clients = binaryFormatter.Deserialize(fileStream) as Dictionary<string, ClientModel>;
                foreach (ClientModel clientModel in Clients.Values)
                {
                    this.ComboBoxUserId.Items.Add(clientModel.Username);
                    //comboBox.Items.Add("12323123");
                }
                //for (int i = 0; i < Clients.Count; i++)
                //{
                //    if (this.ComboBoxUserId.Text != "")
                //    {
                //        if (Clients.ContainsKey(this.ComboBoxUserId.Text))
                //        {
                //            this.TextBoxUserPwd.Text = Clients[this.ComboBoxUserId.Text].Password;
                //            this.CheckBoxRememberPwd.Checked = true;
                //        }
                //    }
                //}
            }
            fileStream.Close();
            if (this.ComboBoxUserId.Items.Count > 0)
            {
                this.ComboBoxUserId.SelectedIndex = this.ComboBoxUserId.Items.Count - 1;
                //this.textBox.Text = Clients[comboBox.Text].Password;
            }
            #endregion
        }

        private void ComboBoxUserId_SelectedValueChanged(object sender, EventArgs e)
        {
            //#region 用户文本切换

            ////FormCommon formCommon = new FormCommon();
            ////formCommon.TextChangedHandler(this.ComboBoxUserId, this.TextBoxUserPwd, this.CheckBoxRememberPwd, Clients);

            //FileStream fileStream = new FileStream("data.bin", FileMode.OpenOrCreate, FileAccess.Read, FileShare.ReadWrite);
            //if (fileStream.Length > 0)
            //{
            //    BinaryFormatter binaryFormatter = new BinaryFormatter();
            //    Dictionary<string, ClientModel> Clients = binaryFormatter.Deserialize(fileStream) as Dictionary<string, ClientModel>;

            //    for (int i = 0; i < Clients.Count; i++)
            //    {
            //        if (this.ComboBoxUserId.Text != "")
            //        {
            //            if (Clients.ContainsKey(this.ComboBoxUserId.Text) && Clients[this.ComboBoxUserId.Text].Password != "")
            //            {
            //                this.ComboBoxUserId.Text = Clients[this.ComboBoxUserId.Text].Username;
            //                this.TextBoxUserPwd.Text = Clients[this.ComboBoxUserId.Text].Password;
            //                this.CheckBoxRememberPwd.Checked = true;
            //            }
            //            else
            //            {
            //                this.ComboBoxUserId.Text = "";
            //                this.TextBoxUserPwd.Text = "";
            //                this.CheckBoxRememberPwd.Checked = false;
            //            }
            //        }
            //    }
            //}
            //fileStream.Close();
            //#endregion
        }

        private void BtnLogin_Click(object sender, EventArgs e)
        {
            #region 记录bin

            ClientModel clientModel = new ClientModel
            {
                Username = this.ComboBoxUserId.Text.Trim()
            };

            //2076934ea0e3.wrs.netbridge.vip
            //2076934e94d3.wrs.netbridge.vip
            //2076934fe5d7.wrs.netbridge.vip

            BinaryFormatter binaryFormatter = new BinaryFormatter();
            using (FileStream fileStream = new FileStream("data.bin", FileMode.Create))
            {
                if (Clients.ContainsKey(clientModel.Username))
                {
                    Clients.Remove(clientModel.Username);
                }
                Clients.Add(clientModel.Username, clientModel);
                binaryFormatter.Serialize(fileStream, Clients);
            }
            #endregion

            try
            {
                TransRouterName transRouterName = new TransRouterName(BusinessLogic.GetRouterName);
                transRouterName(this.ComboBoxUserId.Text.ToString());

                this.Hide();

                MainForm mainForm = new MainForm();
                mainForm.ShowDialog();
            }
            catch(Exception ex)
            {
                this.LabelWarning.Text = "请检查路由器远程连接设置！";
                LogHelper.WriteLog(ex.ToString());
            }
            
        }

        private void PicClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
