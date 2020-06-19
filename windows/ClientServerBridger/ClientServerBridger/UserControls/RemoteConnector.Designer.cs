namespace ClientServerBridger
{
    partial class RemoteConnector
    {
        /// <summary> 
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region 组件设计器生成的代码

        /// <summary> 
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RemoteConnector));
            this.PicRemoteState = new System.Windows.Forms.PictureBox();
            this.LabelRemoteInfo = new System.Windows.Forms.Label();
            this.BtnConnection = new System.Windows.Forms.Button();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.PicRemoteState)).BeginInit();
            this.SuspendLayout();
            // 
            // PicRemoteState
            // 
            this.PicRemoteState.Dock = System.Windows.Forms.DockStyle.Top;
            this.PicRemoteState.Image = ((System.Drawing.Image)(resources.GetObject("PicRemoteState.Image")));
            this.PicRemoteState.Location = new System.Drawing.Point(0, 0);
            this.PicRemoteState.Name = "PicRemoteState";
            this.PicRemoteState.Size = new System.Drawing.Size(199, 143);
            this.PicRemoteState.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.PicRemoteState.TabIndex = 0;
            this.PicRemoteState.TabStop = false;
            // 
            // LabelRemoteInfo
            // 
            this.LabelRemoteInfo.BackColor = System.Drawing.Color.Transparent;
            this.LabelRemoteInfo.Dock = System.Windows.Forms.DockStyle.Top;
            this.LabelRemoteInfo.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.LabelRemoteInfo.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.LabelRemoteInfo.ForeColor = System.Drawing.Color.RosyBrown;
            this.LabelRemoteInfo.Location = new System.Drawing.Point(0, 143);
            this.LabelRemoteInfo.Name = "LabelRemoteInfo";
            this.LabelRemoteInfo.Size = new System.Drawing.Size(199, 22);
            this.LabelRemoteInfo.TabIndex = 1;
            this.LabelRemoteInfo.Tag = "";
            this.LabelRemoteInfo.Text = "主机名";
            this.LabelRemoteInfo.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.LabelRemoteInfo.Click += new System.EventHandler(this.LabelRemoteInfo_Click);
            // 
            // BtnConnection
            // 
            this.BtnConnection.BackColor = System.Drawing.Color.Transparent;
            this.BtnConnection.Dock = System.Windows.Forms.DockStyle.Top;
            this.BtnConnection.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.BtnConnection.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.BtnConnection.ForeColor = System.Drawing.Color.LightCoral;
            this.BtnConnection.Location = new System.Drawing.Point(0, 165);
            this.BtnConnection.Name = "BtnConnection";
            this.BtnConnection.Size = new System.Drawing.Size(199, 33);
            this.BtnConnection.TabIndex = 5;
            this.BtnConnection.Text = "连  接";
            this.BtnConnection.UseVisualStyleBackColor = false;
            this.BtnConnection.Click += new System.EventHandler(this.BtnConnection_Click);
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.ImageScalingSize = new System.Drawing.Size(19, 19);
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(61, 4);
            // 
            // RemoteConnector
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 14F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.BtnConnection);
            this.Controls.Add(this.LabelRemoteInfo);
            this.Controls.Add(this.PicRemoteState);
            this.Name = "RemoteConnector";
            this.Size = new System.Drawing.Size(199, 198);
            this.Load += new System.EventHandler(this.RemoteConnector_Load);
            ((System.ComponentModel.ISupportInitialize)(this.PicRemoteState)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox PicRemoteState;
        private System.Windows.Forms.Label LabelRemoteInfo;
        private System.Windows.Forms.Button BtnConnection;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
    }
}
