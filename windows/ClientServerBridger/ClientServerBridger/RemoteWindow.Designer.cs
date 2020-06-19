namespace ClientServerBridger
{
    partial class RemoteWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RemoteWindow));
            this.TimerMenu = new System.Windows.Forms.Timer(this.components);
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.WindowRemote = new AxMSTSCLib.AxMsRdpClient8NotSafeForScripting();
            ((System.ComponentModel.ISupportInitialize)(this.WindowRemote)).BeginInit();
            this.SuspendLayout();
            // 
            // TimerMenu
            // 
            this.TimerMenu.Interval = 25;
            // 
            // WindowRemote
            // 
            this.WindowRemote.Dock = System.Windows.Forms.DockStyle.Fill;
            this.WindowRemote.Enabled = true;
            this.WindowRemote.Location = new System.Drawing.Point(0, 0);
            this.WindowRemote.Margin = new System.Windows.Forms.Padding(0);
            this.WindowRemote.Name = "WindowRemote";
            this.WindowRemote.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("WindowRemote.OcxState")));
            this.WindowRemote.Size = new System.Drawing.Size(1942, 1000);
            this.WindowRemote.TabIndex = 3;
            // 
            // RemoteWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 14F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1942, 1000);
            this.Controls.Add(this.WindowRemote);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "RemoteWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "RemoteWindow";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.Load += new System.EventHandler(this.RemoteWindow_Load);
            ((System.ComponentModel.ISupportInitialize)(this.WindowRemote)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Timer TimerMenu;
        private System.Windows.Forms.Timer timer1;
        private AxMSTSCLib.AxMsRdpClient8NotSafeForScripting WindowRemote;
    }
}