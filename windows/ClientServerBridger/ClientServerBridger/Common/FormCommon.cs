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
using System.Xml.Serialization;
using System.Runtime.InteropServices;
using System.Timers;

using ClientServerBridger.CSBModels;

namespace ClientServerBridger.Common
{
    /// <summary>
    /// 窗体通用类
    /// </summary>
    class FormCommon
    {

        #region Form界面优化配置
        
        private static int AcceptRadius; // 获取自定义圆角弧度
        /// <summary>
        /// 设置截断区域
        /// </summary>
        /// <param name="width">宽度</param>
        /// <param name="height">高度</param>
        /// <returns></returns>
        public Region Round(int width, int height)
        {
            int x = 1;
            int y = 1;
            int thisWidth = width;
            int thisHeight = height;
            int angle = AcceptRadius;

            GraphicsPath oPath = new GraphicsPath();
            if (angle > 0)
            {
                //Graphics g = CreateGraphics();
                oPath.AddArc(x, y, angle, angle, 180, 90);                                 // 左上角
                oPath.AddArc(thisWidth - angle, y, angle, angle, 270, 90);                 // 右上角
                oPath.AddArc(thisWidth - angle, thisHeight - angle, angle, angle, 0, 90);  // 右下角
                oPath.AddArc(x, thisHeight - angle, angle, angle, 90, 90);                 // 左下角

                oPath.CloseAllFigures();
                return new Region(oPath);
            }
            else
            {
                oPath.AddLine(x + angle, y, thisWidth - angle, y);                         // 顶端
                oPath.AddLine(thisWidth, y + angle, thisWidth, thisHeight - angle);        // 右边
                oPath.AddLine(thisWidth - angle, thisHeight, x + angle, thisHeight);       // 底边
                oPath.AddLine(x, y + angle, x, thisHeight - angle);                        // 左边
                oPath.CloseAllFigures();
                return new Region(oPath);
            }
        }

        /// <summary>
        /// 委托获取圆角弧度
        /// </summary>
        /// <param name="Radius">圆角弧度</param>
        public static void GetRadius(int Radius)
        {
            AcceptRadius = Radius;
        }
        [DllImport("user32.dll")]
        public static extern bool ReleaseCapture();
        [DllImport("user32.dll")]
        public static extern bool SendMessage(IntPtr hwnd, int wMsg, int wParam, int lParam);
        #endregion

        #region Form自定义动画

        private static Control SetControl;

        /// <summary>
        /// 创建Timer
        /// </summary>
        public void TimerCreate()
        {
            System.Timers.Timer timer = new System.Timers.Timer(10000);
            //timer.Interval = 50;
            timer.Elapsed += new System.Timers.ElapsedEventHandler(TimerTick) ; // Timer类
            //timer.Tick += new System.EventHandler(TimerTick); // Winform的timer控件
            timer.AutoReset = true;
            timer.Enabled = true;
        }

        public void CustomizeAnimate(double Height, double Width, System.Windows.Forms.Timer timer1)
        {
            Rectangle RecScreen = Screen.PrimaryScreen.WorkingArea;

            double cente = (RecScreen.Height - Height) / 2;
            double ce = (RecScreen.Width - Width) / 2;
        }

        /// <summary>
        /// Timer_Tick
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void TimerTick(object sender, System.Timers.ElapsedEventArgs e)
        {
            //Control control = SetControl;
            //SetControl.Height = 20;
            //SetControl.Show();
            
            //int ControlHeight = SetControl.Height;
            //int p = ControlHeight / 50;
            //SetControl.Height = p * i;
            //i += 1;
            //SetControl.Visible = true;
        }

        public static void GetControl(Control control)
        {
            SetControl = control;
        }
        #endregion

        #region Controls显示设置

        /// <summary>
        /// 控件排序
        /// </summary>
        /// <param name="superControl">父控件</param>
        /// <param name="childControl">子控件</param>
        /// <param name="amount">子控件数量</param>
        /// <param name="x_amount">行数量</param>
        public void ControlRange(Control superControl, Control childControl, int amount, int x_amount)
        {
            int x = amount / x_amount;
            int y = amount % x_amount;
            int x_c = childControl.Location.X;
            int y_c = childControl.Location.Y;
            int height = childControl.Height;
            int width = childControl.Width;

            for (int i = 0; i < x; i++)
            {
                for (int j = 0; j < x_amount; j++)
                {
                    childControl.Location = new Point(x_c + j * width, y_c + i * height);
                    superControl.Controls.Add(childControl);
                }
            }
            for (int i = 0; i < y; i++)
            {
                childControl.Location = new Point(x_c + i * width, y_c + x * height);
                superControl.Controls.Add(childControl);
            }
        }
        #endregion

        #region LoginForm用户id存取配置

        /// <summary>
        /// 读取bin
        /// </summary>
        /// <param name="comboBox">组合框</param>
        /// <param name="textBox">文本框</param>
        /// <param name="checkBox">复选框</param>
        public void ReadBin(ComboBox comboBox, TextBox textBox, CheckBox checkBox)
        {
            FileStream fileStream = new FileStream("data.bin", FileMode.OpenOrCreate);
            if (fileStream.Length > 0)
            {
                BinaryFormatter binaryFormatter = new BinaryFormatter();
                Dictionary<string, ClientModel> keyValuePairs = binaryFormatter.Deserialize(fileStream) as Dictionary<string, ClientModel>;
                foreach (ClientModel clientModel in keyValuePairs.Values)
                {
                    comboBox.Items.Add(clientModel.Username);
                }
                for (int i = 0; i < keyValuePairs.Count; i++)
                {
                    if (comboBox.Text != "")
                    {
                        if (keyValuePairs.ContainsKey(comboBox.Text))
                        {
                            textBox.Text = keyValuePairs[comboBox.Text].Password;
                            checkBox.Checked = true;
                        }
                    }
                }
            }
            fileStream.Close();
            if (comboBox.Items.Count > 0)
            {
                comboBox.SelectedIndex = comboBox.Items.Count - 1;
            }
        }

        /// <summary>
        /// 记录bin
        /// </summary>
        /// <param name="checkBox">复选框</param>
        /// <param name="comboBox">用户id</param>
        /// <param name="textBox">用户密码</param>
        public void RememberToBin(CheckBox checkBox, ComboBox comboBox, TextBox textBox)
        {
            ClientModel clientModel = new ClientModel
            {
                Username = comboBox.Text.Trim()
            };
            if (checkBox.Checked)
            {
                clientModel.Password = textBox.Text;
            }
            else
            {
                clientModel.Password = "ckcfpfpa";
            }
            
            FileStream fileStream = new FileStream("data.bin", FileMode.OpenOrCreate);
            BinaryFormatter binaryFormatter = new BinaryFormatter();
            //Dictionary<string, ClientModel> keyValuePairs = new Dictionary<string, ClientModel>();
            Dictionary<string, ClientModel> keyValuePairs = binaryFormatter.Deserialize(fileStream) as Dictionary<string, ClientModel>;
            //if (Clients.ContainsKey(clientModel.Username))
            //{
            //    Clients.Remove(clientModel.Username);
            //}
            keyValuePairs.Add(clientModel.Username, clientModel);
            binaryFormatter.Serialize(fileStream, keyValuePairs);
            
            fileStream.Close();
            //FileStream fileStream1 = new FileStream("data.bin", FileMode.OpenOrCreate,FileAccess.Read);
            //Dictionary<string, ClientModel> keyValuePairs1 = binaryFormatter.Deserialize(fileStream1) as Dictionary<string, ClientModel>;
        }

        /// <summary>
        /// 文本切换
        /// </summary>
        /// <param name="comboBox">组合框</param>
        /// <param name="checkBox">复选框</param>
        public Dictionary<string, ClientModel> TextChangedHandler(ComboBox comboBox,TextBox textBox, CheckBox checkBox, Dictionary<string, ClientModel > keyValuePairs)
        {
            FileStream fileStream = new FileStream("data.bin", FileMode.OpenOrCreate, FileAccess.Read, FileShare.ReadWrite);
            if (fileStream.Length > 0)
            {
                BinaryFormatter binaryFormatter = new BinaryFormatter();
                Dictionary<string, ClientModel> Clients = binaryFormatter.Deserialize(fileStream) as Dictionary<string, ClientModel>;

                for (int i = 0; i < Clients.Count; i++)
                {
                    if (comboBox.Text != "")
                    {
                        if (Clients.ContainsKey(comboBox.Text) && Clients[comboBox.Text].Password != "")
                        {
                            comboBox.Text = Clients[comboBox.Text].Username;
                            textBox.Text = Clients[comboBox.Text].Password;
                            checkBox.Checked = true;
                        }
                        else
                        {
                            comboBox.Text = "";
                            textBox.Text = "";
                            checkBox.Checked = false;
                        }
                    }
                }
            }
            
            fileStream.Close();
            return keyValuePairs;
        }
        #endregion

        #region UserControl通用配置

        /// <summary>
        /// 图片选择
        /// </summary>
        /// <param name="pictureBox">窗口图片</param>
        public void PicSelect(PictureBox pictureBox)
        {
            bool IsRemote = false;
            if (!IsRemote)
            {
                pictureBox.Image = Image.FromFile("C:/Users/10/Documents/ClientServerBridger/ClientServerBridger/Image/1.jpg");
            }
        }
        #endregion

    }
}