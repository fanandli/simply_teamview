using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ClientServerBridger
{
    public partial class RemoteWindowMenu : UserControl
    {
        public RemoteWindowMenu()
        {
            InitializeComponent();
        }

        private void RemoteWindowMenu_Load(object sender, EventArgs e)
        {
           
        }

        private void button1_MouseMove(object sender, MouseEventArgs e)
        {
            this.panel1.Visible = true;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.panel1.Height = 10;
            timer.Start();
            
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            //double x = this.panel1.Size.Width;
            //double y = this.panel1.Size.Height;

            //this.panel1.Height += (int) y / 100;
        }
    }
}
