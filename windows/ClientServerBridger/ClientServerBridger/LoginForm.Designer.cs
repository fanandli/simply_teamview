namespace ClientServerBridger
{
    partial class LoginForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(LoginForm));
            this.LabelUserId = new System.Windows.Forms.Label();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.BtnLogin = new System.Windows.Forms.Button();
            this.ComboBoxUserId = new System.Windows.Forms.ComboBox();
            this.PicClose = new System.Windows.Forms.PictureBox();
            this.LabelWarning = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.PicClose)).BeginInit();
            this.SuspendLayout();
            // 
            // LabelUserId
            // 
            this.LabelUserId.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.LabelUserId.AutoSize = true;
            this.LabelUserId.BackColor = System.Drawing.Color.Transparent;
            this.LabelUserId.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.LabelUserId.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(193)))), ((int)(((byte)(255)))));
            this.LabelUserId.Location = new System.Drawing.Point(272, 180);
            this.LabelUserId.Name = "LabelUserId";
            this.LabelUserId.Size = new System.Drawing.Size(94, 20);
            this.LabelUserId.TabIndex = 1;
            this.LabelUserId.Text = "路由器名：";
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.checkBox1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(193)))), ((int)(((byte)(255)))));
            this.checkBox1.Location = new System.Drawing.Point(-183, 673);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(74, 19);
            this.checkBox1.TabIndex = 3;
            this.checkBox1.Text = "记住密码";
            this.checkBox1.UseVisualStyleBackColor = true;
            // 
            // BtnLogin
            // 
            this.BtnLogin.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.BtnLogin.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(123)))), ((int)(((byte)(158)))), ((int)(((byte)(201)))));
            this.BtnLogin.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.BtnLogin.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.BtnLogin.ForeColor = System.Drawing.Color.Black;
            this.BtnLogin.Location = new System.Drawing.Point(276, 269);
            this.BtnLogin.Name = "BtnLogin";
            this.BtnLogin.Size = new System.Drawing.Size(197, 27);
            this.BtnLogin.TabIndex = 4;
            this.BtnLogin.Text = "登  录";
            this.BtnLogin.UseVisualStyleBackColor = false;
            this.BtnLogin.Click += new System.EventHandler(this.BtnLogin_Click);
            // 
            // ComboBoxUserId
            // 
            this.ComboBoxUserId.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ComboBoxUserId.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(129)))), ((int)(((byte)(162)))), ((int)(((byte)(204)))));
            this.ComboBoxUserId.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.ComboBoxUserId.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.ComboBoxUserId.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(193)))), ((int)(((byte)(255)))));
            this.ComboBoxUserId.FormattingEnabled = true;
            this.ComboBoxUserId.Location = new System.Drawing.Point(276, 203);
            this.ComboBoxUserId.Name = "ComboBoxUserId";
            this.ComboBoxUserId.Size = new System.Drawing.Size(197, 25);
            this.ComboBoxUserId.TabIndex = 5;
            this.ComboBoxUserId.SelectedValueChanged += new System.EventHandler(this.ComboBoxUserId_SelectedValueChanged);
            // 
            // PicClose
            // 
            this.PicClose.BackColor = System.Drawing.Color.Transparent;
            this.PicClose.Cursor = System.Windows.Forms.Cursors.Default;
            this.PicClose.Image = ((System.Drawing.Image)(resources.GetObject("PicClose.Image")));
            this.PicClose.Location = new System.Drawing.Point(702, 14);
            this.PicClose.Name = "PicClose";
            this.PicClose.Size = new System.Drawing.Size(24, 24);
            this.PicClose.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.PicClose.TabIndex = 6;
            this.PicClose.TabStop = false;
            this.PicClose.Click += new System.EventHandler(this.PicClose_Click);
            // 
            // LabelWarning
            // 
            this.LabelWarning.AutoSize = true;
            this.LabelWarning.BackColor = System.Drawing.Color.Transparent;
            this.LabelWarning.Location = new System.Drawing.Point(273, 312);
            this.LabelWarning.Name = "LabelWarning";
            this.LabelWarning.Size = new System.Drawing.Size(0, 15);
            this.LabelWarning.TabIndex = 7;
            // 
            // LoginForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 14F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.ClientSize = new System.Drawing.Size(744, 527);
            this.Controls.Add(this.LabelWarning);
            this.Controls.Add(this.PicClose);
            this.Controls.Add(this.ComboBoxUserId);
            this.Controls.Add(this.BtnLogin);
            this.Controls.Add(this.checkBox1);
            this.Controls.Add(this.LabelUserId);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "LoginForm";
            this.Opacity = 0.75D;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "LoginForm";
            this.Load += new System.EventHandler(this.LoginForm_Load);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.LoginForm_Paint);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.LoginForm_MouseDown);
            ((System.ComponentModel.ISupportInitialize)(this.PicClose)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label LabelUserId;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.Button BtnLogin;
        private System.Windows.Forms.ComboBox ComboBoxUserId;
        private System.Windows.Forms.PictureBox PicClose;
        private System.Windows.Forms.Label LabelWarning;
    }
}