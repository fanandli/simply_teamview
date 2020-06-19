using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.InteropServices;


namespace ClientServerBridger.CSBModels
{
    class CustomizeFormModel
    {

        #region 成员属性

        private int _Radius;  // 圆角弧度
        private double _Height; // 高度
        private double _Width; // 宽度

        /// <summary>
        /// 圆角弧度(0为不要圆角)
        /// </summary>
        [Browsable(true)]
        [Description("圆角弧度(0为不要圆角)")]
        public int Radius  // 设置圆角弧度
        {
            get
            {
                return _Radius;
            }
            set
            {
                if (value < 0)
                {
                    _Radius = 0;
                }
                else
                {
                    _Radius = value;
                }

            }
        }

        public double Height { get => _Height; set => _Height = value; }
        public double Width { get => _Width; set => _Width = value; }
        #endregion

        #region 构造方法

        #endregion

    }
}
