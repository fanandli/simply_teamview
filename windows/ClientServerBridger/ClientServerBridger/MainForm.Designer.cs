namespace ClientServerBridger
{
    partial class MainForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.PanelMenu = new System.Windows.Forms.Panel();
            this.PicMin = new System.Windows.Forms.PictureBox();
            this.PicClose = new System.Windows.Forms.PictureBox();
            this.PanelBody = new System.Windows.Forms.Panel();
            this.PanelPage = new System.Windows.Forms.Panel();
            this.PanelPageMenu = new System.Windows.Forms.Panel();
            this.button2 = new System.Windows.Forms.Button();
            this.PanelMenu.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.PicMin)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PicClose)).BeginInit();
            this.PanelPage.SuspendLayout();
            this.SuspendLayout();
            // 
            // PanelMenu
            // 
            this.PanelMenu.BackColor = System.Drawing.Color.Transparent;
            this.PanelMenu.Controls.Add(this.PicMin);
            this.PanelMenu.Controls.Add(this.PicClose);
            this.PanelMenu.Dock = System.Windows.Forms.DockStyle.Top;
            this.PanelMenu.Location = new System.Drawing.Point(0, 0);
            this.PanelMenu.Name = "PanelMenu";
            this.PanelMenu.Size = new System.Drawing.Size(1390, 44);
            this.PanelMenu.TabIndex = 0;
            // 
            // PicMin
            // 
            this.PicMin.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.PicMin.Cursor = System.Windows.Forms.Cursors.Hand;
            this.PicMin.Image = ((System.Drawing.Image)(resources.GetObject("PicMin.Image")));
            this.PicMin.Location = new System.Drawing.Point(1318, 8);
            this.PicMin.Name = "PicMin";
            this.PicMin.Size = new System.Drawing.Size(27, 30);
            this.PicMin.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.PicMin.TabIndex = 2;
            this.PicMin.TabStop = false;
            this.PicMin.Click += new System.EventHandler(this.PicMin_Click);
            // 
            // PicClose
            // 
            this.PicClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.PicClose.Cursor = System.Windows.Forms.Cursors.Hand;
            this.PicClose.Image = ((System.Drawing.Image)(resources.GetObject("PicClose.Image")));
            this.PicClose.Location = new System.Drawing.Point(1351, 8);
            this.PicClose.Name = "PicClose";
            this.PicClose.Size = new System.Drawing.Size(27, 30);
            this.PicClose.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.PicClose.TabIndex = 2;
            this.PicClose.TabStop = false;
            this.PicClose.Click += new System.EventHandler(this.PicClose_Click);
            // 
            // PanelBody
            // 
            this.PanelBody.BackColor = System.Drawing.Color.Transparent;
            this.PanelBody.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PanelBody.ForeColor = System.Drawing.Color.Firebrick;
            this.PanelBody.Location = new System.Drawing.Point(0, 44);
            this.PanelBody.Name = "PanelBody";
            this.PanelBody.Size = new System.Drawing.Size(1390, 663);
            this.PanelBody.TabIndex = 1;
            this.PanelBody.MouseDown += new System.Windows.Forms.MouseEventHandler(this.PanelBody_MouseDown);
            // 
            // PanelPage
            // 
            this.PanelPage.BackColor = System.Drawing.Color.Transparent;
            this.PanelPage.Controls.Add(this.PanelPageMenu);
            this.PanelPage.Controls.Add(this.button2);
            this.PanelPage.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.PanelPage.ForeColor = System.Drawing.Color.Firebrick;
            this.PanelPage.Location = new System.Drawing.Point(0, 707);
            this.PanelPage.Name = "PanelPage";
            this.PanelPage.Size = new System.Drawing.Size(1390, 80);
            this.PanelPage.TabIndex = 2;
            // 
            // PanelPageMenu
            // 
            this.PanelPageMenu.Location = new System.Drawing.Point(497, 19);
            this.PanelPageMenu.Name = "PanelPageMenu";
            this.PanelPageMenu.Size = new System.Drawing.Size(408, 49);
            this.PanelPageMenu.TabIndex = 4;
            // 
            // button2
            // 
            this.button2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(192)))));
            this.button2.Location = new System.Drawing.Point(582, 518);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(87, 27);
            this.button2.TabIndex = 3;
            this.button2.Text = "临 时";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 14F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.ClientSize = new System.Drawing.Size(1390, 787);
            this.Controls.Add(this.PanelBody);
            this.Controls.Add(this.PanelPage);
            this.Controls.Add(this.PanelMenu);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "MainForm";
            this.Opacity = 0.85D;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.PanelMenu.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.PicMin)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PicClose)).EndInit();
            this.PanelPage.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel PanelMenu;
        private System.Windows.Forms.Panel PanelBody;
        private System.Windows.Forms.PictureBox PicClose;
        private System.Windows.Forms.PictureBox PicMin;
        private System.Windows.Forms.Panel PanelPage;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Panel PanelPageMenu;
    }
}