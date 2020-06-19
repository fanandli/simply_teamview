using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ClientServerBridger
{
    public partial class PageTurn : UserControl, IMessageFilter
    {
        private const int SIZE_HEIGHT = 40;
        private const int SIZE_MINWIDTH = 84;
        private Label labPageInfo;
        private PictureBox pbPage_Begin;
        private PictureBox pbPage_Next;
        private PictureBox pbPage_Prev;
        private PictureBox pbPage_End;
        private TextBox txtPageInfo;
        private const int SIZE_INFO_MINWIDTH = 188;

        public PageTurn()
        {
            Application.AddMessageFilter(this);

            InitializeComponent();

            this.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.MinimumSize = new Size(SIZE_MINWIDTH, 0);
            this.Disposed += new EventHandler(Pages_Disposed);

            //this.MouseClick += new MouseEventHandler(ucKeyboard_Close);

            PageChanged += new PageChangedHandle(new PageChangedHandle((oldPage, newPage, e) => { }));
            InputGotFocus += new InputFocusHandle(new InputFocusHandle((sender, e) => { }));
            //InputLostFocus += new InputFocusHandle(new InputFocusHandle((sender, e) => { }));

            InputGotFocus += new InputFocusHandle(new InputFocusHandle((sender, e) => { }));
            InputLostFocus += new InputFocusHandle(new InputFocusHandle((sender, e) => { }));

            InputMouseDown += new InputMouseHandle(new InputMouseHandle((sender, e) => { }));
            InputMouseUp += new InputMouseHandle(new InputMouseHandle((sender, e) => { }));
            InputTextClick += new EventHandler(new EventHandler((sender, e) => { }));
            InputKeyDown += new InputKeyHandle(new InputKeyHandle((sender, e) => { }));
            InputKeyUp += new InputKeyHandle(new InputKeyHandle((sender, e) => { }));
            InputKeyPress += new InputKeyPressHandle(new InputKeyPressHandle((sender, e) => { }));
            InputTextChanged += new EventHandler(new EventHandler((sender, e) => { }));

            this.BackColor = Color.White;
            labPageInfo.BackColor = this.BackColor;

            this.Resize += new EventHandler(Pages_Resize);

            //labPageInfo.MouseDoubleClick += new MouseEventHandler(labPageInfo_MouseDoubleClick);

            pbPage_Prev.MouseDown += new MouseEventHandler(Page_Prev_MouseDown);
            pbPage_Prev.MouseUp += new MouseEventHandler(Page_Prev_MouseUp);

            pbPage_Next.MouseDown += new MouseEventHandler(Page_Next_MouseDown);
            pbPage_Next.MouseUp += new MouseEventHandler(Page_Next_MouseUp);

            pbPage_Begin.MouseDown += new MouseEventHandler(Page_Begin_MouseDown);
            pbPage_Begin.MouseUp += new MouseEventHandler(Page_Begin_MouseUp);

            pbPage_End.MouseDown += new MouseEventHandler(Page_End_MouseDown);
            pbPage_End.MouseUp += new MouseEventHandler(Page_End_MouseUp);

            txtPageInfo.TextChanged += new EventHandler(txtPageInfo_TextChanged);
            txtPageInfo.GotFocus += new EventHandler(txtPageInfo_GotFocus);
            txtPageInfo.Click += new EventHandler(txtPageInfo_Click);
            txtPageInfo.Text = m_strText;
            txtPageInfo.Visible = m_blnShowTxtPageInfo;

            m_blnIsAutoJump = false;
            m_timerAutoPage.Enabled = false;
            m_timerAutoPage.Interval = WAIT_FOR_AUTOJUMP;
            m_timerAutoPage.Tick += new EventHandler(timerAutoPage_Tick);
        }

        private void Pages_Load(object sender, EventArgs e)
        {
            setStatus();
        }

        private void Pages_Disposed(object sender, EventArgs e)
        {
            Application.RemoveMessageFilter(this);
        }

        public bool PreFilterMessage(ref System.Windows.Forms.Message MyMessage)
        {
            if (MyMessage.Msg == 0x204 || MyMessage.Msg == 0x205)
            {
                return true;
            }
            return false;
        }

        //设置控件的自适应大小
        private void Pages_Resize(object sender, EventArgs e)
        {
            this.Height = SIZE_HEIGHT;

            pbPage_Begin.Location = new Point(0, 0);
            pbPage_Begin.Size = new Size(SIZE_HEIGHT, SIZE_HEIGHT);

            pbPage_Prev.Location = new Point(pbPage_Begin.Width + 2, pbPage_Begin.Top);
            pbPage_Prev.Size = pbPage_Begin.Size;

            pbPage_End.Location = new Point(this.Width - pbPage_End.Width, pbPage_Begin.Top);
            pbPage_End.Size = pbPage_Begin.Size;

            pbPage_Next.Location = new Point(this.Width - pbPage_Next.Width - pbPage_End.Width - 2, pbPage_Begin.Top);
            pbPage_Next.Size = pbPage_Begin.Size;

            if (this.Width < SIZE_INFO_MINWIDTH)
            {
                labPageInfo.Visible = false;
                txtPageInfo.Visible = false;
            }
            else
            {
                labPageInfo.Location = new Point(pbPage_Prev.Width + pbPage_Prev.Width + 3, 2);
                labPageInfo.Size = new Size(pbPage_Next.Left - labPageInfo.Left - 3, pbPage_Prev.Height);

                txtPageInfo.Location = new Point(pbPage_Prev.Left + pbPage_Prev.Width + 5, 11);
                //txtPageInfo.Size = new Size(pbPage_Next.Left - labPageInfo.Left , 15);

                if (m_blnShowLabel && !labPageInfo.Visible) labPageInfo.Visible = true;
                if (m_blnShowLabel && !txtPageInfo.Visible) txtPageInfo.Visible = true;
            }

            if (m_blnShowTxtPageInfo)
            {
                txtPageInfo.Size = new Size(79, labPageInfo.Height);
            }
            else
            {
                txtPageInfo.Size = new Size(0, labPageInfo.Height);
            }
        }

        //点击lablelabPageInfo  显示txtPageInfo
        private void labPageInfo_Click(object sender, EventArgs e)
        {
            if (!txtPageInfo.Visible)
            {
                showJumpPageStatus(true);
                InputTextClick(txtPageInfo, new EventArgs());
            }
            else
            {
                showJumpPageStatus(false);
            }
        }

        public void showJumpPageStatus(Boolean isShow)
        {
            if (isShow)
            {
                txtPageInfo.Visible = true;
                txtPageInfo.Text = string.Empty;
                txtPageInfo.Focus();
            }
            else
            {
                txtPageInfo.Visible = false;
            }
        }

        //上一页
        private void Page_Prev_MouseDown(object sender, MouseEventArgs e)
        {
            //pbPage_Prev.Image = global::Pku.CFM.Controls.Properties.Resources.Page_Prev_D;
            m_blnIsPrevDown = true;

            m_timerAutoPage.Enabled = true;
        }

        private void Page_Prev_MouseUp(object sender, MouseEventArgs e)
        {
            //pbPage_Prev.Image = global::Pku.CFM.Controls.Properties.Resources.Page_Prev_N;
            m_blnIsPrevDown = false;

            if (m_blnIsAutoJump)
            {
                leaveAutoJumpMode();
                return;
            }
            m_timerAutoPage.Enabled = false;

            if (1 == m_intCurPageIndex) return;
            int intOldPage = m_intCurPageIndex;
            m_intCurPageIndex--;
            setStatus();
            PageChanged(intOldPage, m_intCurPageIndex, new EventArgs());
        }

        //下一页
        private void Page_Next_MouseDown(object sender, MouseEventArgs e)
        {

            //pbPage_Next.Image = global::Pku.CFM.Controls.Properties.Resources.Page_Next_D;
            m_blnIsNextDown = true;

            m_timerAutoPage.Enabled = true;
        }

        private void Page_Next_MouseUp(object sender, MouseEventArgs e)
        {
            //bPage_Next.Image = global::Pku.CFM.Controls.Properties.Resources.Page_Next_N;
            m_blnIsNextDown = false;

            if (m_blnIsAutoJump)
            {
                leaveAutoJumpMode();
                return;
            }
            m_timerAutoPage.Enabled = false;

            if (m_intPageCount == m_intCurPageIndex) return;
            int intOldPage = m_intCurPageIndex;
            m_intCurPageIndex++;
            setStatus();
            PageChanged(intOldPage, m_intCurPageIndex, new EventArgs());
        }

        //首页
        private void Page_Begin_MouseDown(object sender, MouseEventArgs e)
        {

            //pbPage_Begin.Image = global::Pku.CFM.Controls.Properties.Resources.PageView_Begin_D;
            m_blnIsBeginDown = false;
            m_timerAutoPage.Enabled = true;
        }

        private void Page_Begin_MouseUp(object sender, MouseEventArgs e)
        {
            //pbPage_Begin.Image = global::Pku.CFM.Controls.Properties.Resources.PageView_Begin_N;
            m_blnIsBeginDown = false;

            int intOldPage = m_intCurPageIndex;

            if (1 == m_intCurPageIndex) return;
            m_intCurPageIndex = 1;


            setStatus();
            PageChanged(intOldPage, m_intCurPageIndex, new EventArgs());

            m_blnIsAutoJump = false;
            m_timerAutoPage.Stop();
        }

        //尾页
        private void Page_End_MouseDown(object sender, MouseEventArgs e)
        {

            //pbPage_End.Image = global::Pku.CFM.Controls.Properties.Resources.PageView_End_N;
            m_blnIsEndDown = true;

            m_timerAutoPage.Enabled = true;
        }

        private void Page_End_MouseUp(object sender, MouseEventArgs e)
        {
            
            m_blnIsEndDown = false;

            int intOldPage = m_intCurPageIndex;

            if (m_intCurPageIndex == m_intPageCount) return;
            m_intCurPageIndex = m_intPageCount;

            setStatus();
            PageChanged(intOldPage, m_intCurPageIndex, new EventArgs());

            m_blnIsAutoJump = false;
            m_timerAutoPage.Stop();

        }

        //翻页按钮的状态
        private void setStatus()
        {
            //如果页数为0，翻页的按钮全部不显示
            if (m_intPageCount <= 0)
            {
                labPageInfo.Text = "";
                pbPage_Prev.Visible = false;
                pbPage_Next.Visible = false;
                pbPage_Begin.Visible = false;
                pbPage_End.Visible = false;
                txtPageInfo.Visible = false;
            }
            else
            {
                //如果页数为1，翻页的按钮不显示
                if (1 == m_intPageCount)
                {
                    labPageInfo.Text = "";
                    pbPage_Prev.Visible = false;
                    pbPage_Next.Visible = false;
                    pbPage_Begin.Visible = false;
                    pbPage_End.Visible = false;
                    txtPageInfo.Visible = false;
                }
                else
                {
                    //只显示下一页和最后一页的按钮
                    if (m_intCurPageIndex <= 1)
                    {
                        m_intCurPageIndex = 1;

                        pbPage_Prev.Visible = false;
                        pbPage_Next.Visible = true;
                        pbPage_Begin.Visible = false;
                        pbPage_End.Visible = true;
                        txtPageInfo.Visible = false;

                    }
                    //只显示上一页和首页的按钮
                    else if (m_intCurPageIndex >= m_intPageCount)
                    {
                        m_intCurPageIndex = m_intPageCount;

                        pbPage_Prev.Visible = true;
                        pbPage_Next.Visible = false;
                        pbPage_Begin.Visible = true;
                        pbPage_End.Visible = false;
                        txtPageInfo.Visible = false;

                    }
                    //否则按钮全部显示
                    else
                    {
                        pbPage_Prev.Visible = true;
                        pbPage_Next.Visible = true;
                        pbPage_Begin.Visible = true;
                        pbPage_End.Visible = true;
                        txtPageInfo.Visible = false;
                    }

                    labPageInfo.Text = String.Format("第{0}页   /共{1}页", m_intCurPageIndex, m_intPageCount);
                    txtPageInfo.Text = String.Format("{0}", m_intCurPageIndex);
                }
            }
        }

        private void timerAutoPage_Tick(object sender, EventArgs e)
        {
            if ((m_blnIsNextDown && m_intCurPageIndex >= m_intPageCount) || (m_blnIsPrevDown && m_intCurPageIndex <= 1) || (m_blnIsEndDown && m_intCurPageIndex >= m_intPageCount) || (m_blnIsBeginDown && m_intCurPageIndex <= 1))
            {
                leaveAutoJumpMode();
                return;
            }

            if (!m_blnIsAutoJump) m_timerAutoPage.Interval = AUTOJUMP_INV;

            int intOldPage = m_intCurPageIndex;

            if (m_blnIsNextDown) m_intCurPageIndex++;
            if (m_blnIsPrevDown) m_intCurPageIndex--;

            setStatus();

            PageChanged(intOldPage, m_intCurPageIndex, new EventArgs());

            if ((m_blnIsNextDown && m_intCurPageIndex >= m_intPageCount) || (m_blnIsPrevDown && m_intCurPageIndex <= 1) || (m_blnIsEndDown && m_intCurPageIndex >= m_intPageCount) || (m_blnIsBeginDown && m_intCurPageIndex <= 1))
            {
                leaveAutoJumpMode();
            }
            else
            {
                m_blnIsAutoJump = true;
            }
        }

        private void leaveAutoJumpMode()
        {
            m_blnIsAutoJump = false;
            m_timerAutoPage.Stop();
            m_timerAutoPage.Enabled = false;
            m_timerAutoPage.Interval = WAIT_FOR_AUTOJUMP;
        }

        private int m_intCurPageIndex = 0;

        //当前页面
        public int CurPageIndex
        {
            get { return m_intCurPageIndex; }
            set
            {
                if (value < 0 || (m_intPageCount > 0 && value > m_intPageCount)) return;

                int intOldPage = m_intCurPageIndex;
                m_intCurPageIndex = value;

                setStatus();
                if (!m_blnIgnoreChange) PageChanged(intOldPage, m_intCurPageIndex, new EventArgs());

                m_blnIsAutoJump = false;
                m_timerAutoPage.Stop();
                m_timerAutoPage.Enabled = false;
            }
        }

        //计算总页数
        public int PageCount
        {
            get { return m_intPageCount; }
            set
            {
                m_intPageCount = value;

                if (m_intPageCount > 0)
                {
                    if (m_intCurPageIndex <= 0) m_intCurPageIndex = 0;

                    if (m_intCurPageIndex > m_intPageCount)
                    {
                        m_intCurPageIndex = m_intPageCount;
                    }
                }
                else
                {
                    m_intCurPageIndex = 0;
                }

                setStatus();
            }
        }

        private Boolean m_blnIgnoreChange = false;
        public Boolean IgnoreChange
        {
            get { return m_blnIgnoreChange; }
            set { m_blnIgnoreChange = value; }
        }

        private Boolean m_blnShowLabel = true;
        public Boolean ShowLabel
        {
            get { return m_blnShowLabel; }
            set { m_blnShowLabel = value; labPageInfo.Visible = value; }
        }

        private void txtPageInfo_TextChanged(object sender, EventArgs e)
        {
            if (m_blnIgnTextChange) return;
            m_blnIgnTextChange = true;
            InputTextChanged(sender, e);
        }

        private void txtPageInfo_GotFocus(object sender, EventArgs e)
        {
            InputGotFocus(this, e);
        }

        private void txtPageInfo_LostFocus(object sender, EventArgs e)
        {
            InputLostFocus(this, e);
        }

        private void txtPageInfo_MouseDown(object sender, MouseEventArgs e)
        {
            InputMouseDown(sender, e);
        }

        private void txtPageInfo_MouseUp(object sender, MouseEventArgs e)
        {
            InputMouseUp(sender, e);
        }

        private void txtPageInfo_Click(object sender, EventArgs e)
        {
            InputTextClick(sender, e);
        }

        private void txtPageInfo_KeyDown(object sender, KeyEventArgs e)
        {
            InputKeyDown(sender, e);
        }

        private void txtPageInfo_KeyUp(object sender, KeyEventArgs e)
        {
            InputKeyUp(sender, e);
        }

        private void txtPageInfo_KeyPress(object sender, KeyPressEventArgs e)
        {
            InputKeyPress(sender, e);
        }

        private string m_strText = "";
        public String Text
        {
            get { return m_strText; }
            set { m_strText = value; }
        }

        private System.Windows.Forms.Timer m_timerAutoPage = new Timer();
        private bool m_blnIsPrevDown = false;
        private bool m_blnIsNextDown = false;

        private bool m_blnIsBeginDown = false;
        private bool m_blnIsEndDown = false;

        private bool m_blnIsAutoJump = false;
        public delegate void PageChangedHandle(int oldPage, int newPage, EventArgs e);
        public event PageChangedHandle PageChanged;

        private const int WAIT_FOR_AUTOJUMP = 500;
        private const int AUTOJUMP_INV = 500;

        private int m_intPageCount = 0;
        protected String m_strErrorText = "";
        public String ErrorText
        {
            get { return m_strErrorText; }
            set { m_strErrorText = value; }
        }

        /// <summary>
        /// 键盘控件的父对象
        /// </summary>
        private Control m_keyboardParent = null;
        public Control KeyboardParent
        {
            get { return m_keyboardParent; }
            set { m_keyboardParent = value; }
        }

        /// <summary>
        /// 是否显示输入翻页框按钮
        /// </summary>
        private Boolean m_blnShowTxtPageInfo = true;
        public Boolean ShowTxtPageInfo
        {
            get { return m_blnShowTxtPageInfo; }
            set { m_blnShowTxtPageInfo = value; txtPageInfo.Visible = value; }
        }

        //public bool IsPages { get; set; }

        private bool m_blnIgnTextChange = false;
        private Boolean m_blnIsInputFocus = false;

        //public event EventHandler InputEnterPressed;
        //public event EventHandler InputClearPressed;

        public delegate void InputFocusHandle(object sender, EventArgs e);
        public event InputFocusHandle InputGotFocus;
        public event InputFocusHandle InputLostFocus;

        public delegate void InputMouseHandle(object sender, MouseEventArgs e);
        public event InputMouseHandle InputMouseDown;
        public event InputMouseHandle InputMouseUp;

        public event EventHandler InputTextClick;

        public delegate void InputKeyHandle(object sender, KeyEventArgs e);
        public event InputKeyHandle InputKeyDown;
        public event InputKeyHandle InputKeyUp;
        //public event InputKeyHandle InputTextKeyBoardEnter;

        public delegate void InputKeyPressHandle(object sender, KeyPressEventArgs e);
        public event InputKeyPressHandle InputKeyPress;

        public event EventHandler InputTextChanged;


        public TextBox InputTextBox
        {
            set { txtPageInfo = value; }
            get { return txtPageInfo; }
        }

        public String InputText
        {
            get
            {
                if (m_strText == txtPageInfo.Text || String.IsNullOrWhiteSpace(txtPageInfo.Text))
                {
                    return String.Empty;
                }
                else
                {
                    return txtPageInfo.Text;
                }
            }
            set
            {
                if (m_blnIsInputFocus)
                {
                    txtPageInfo.Text = value;
                }
                else
                {
                    if (String.IsNullOrWhiteSpace(value))
                    {
                        m_blnIgnTextChange = true;
                        txtPageInfo.Text = m_strText;
                        m_blnIgnTextChange = false;
                    }
                }
            }
        }

        public void setInputText(String text)
        {
            txtPageInfo.Text = text;
        }

        private void InitializeComponent()
        {
            this.labPageInfo = new System.Windows.Forms.Label();
            this.pbPage_Begin = new System.Windows.Forms.PictureBox();
            this.pbPage_Next = new System.Windows.Forms.PictureBox();
            this.pbPage_Prev = new System.Windows.Forms.PictureBox();
            this.pbPage_End = new System.Windows.Forms.PictureBox();
            this.txtPageInfo = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_Begin)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_Next)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_Prev)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_End)).BeginInit();
            this.SuspendLayout();
            // 
            // labPageInfo
            // 
            this.labPageInfo.AutoSize = true;
            this.labPageInfo.Location = new System.Drawing.Point(502, 52);
            this.labPageInfo.Name = "labPageInfo";
            this.labPageInfo.Size = new System.Drawing.Size(55, 15);
            this.labPageInfo.TabIndex = 0;
            this.labPageInfo.Text = "label1";
            // 
            // pbPage_Begin
            // 
            this.pbPage_Begin.Location = new System.Drawing.Point(140, 30);
            this.pbPage_Begin.Name = "pbPage_Begin";
            this.pbPage_Begin.Size = new System.Drawing.Size(100, 50);
            this.pbPage_Begin.TabIndex = 1;
            this.pbPage_Begin.TabStop = false;
            // 
            // pbPage_Next
            // 
            this.pbPage_Next.Location = new System.Drawing.Point(623, 30);
            this.pbPage_Next.Name = "pbPage_Next";
            this.pbPage_Next.Size = new System.Drawing.Size(100, 50);
            this.pbPage_Next.TabIndex = 2;
            this.pbPage_Next.TabStop = false;
            this.pbPage_Next.Click += new System.EventHandler(this.pbPage_Next_Click);
            // 
            // pbPage_Prev
            // 
            this.pbPage_Prev.Location = new System.Drawing.Point(315, 30);
            this.pbPage_Prev.Name = "pbPage_Prev";
            this.pbPage_Prev.Size = new System.Drawing.Size(100, 50);
            this.pbPage_Prev.TabIndex = 3;
            this.pbPage_Prev.TabStop = false;
            // 
            // pbPage_End
            // 
            this.pbPage_End.Location = new System.Drawing.Point(751, 30);
            this.pbPage_End.Name = "pbPage_End";
            this.pbPage_End.Size = new System.Drawing.Size(100, 50);
            this.pbPage_End.TabIndex = 4;
            this.pbPage_End.TabStop = false;
            // 
            // txtPageInfo
            // 
            this.txtPageInfo.Location = new System.Drawing.Point(475, 79);
            this.txtPageInfo.Name = "txtPageInfo";
            this.txtPageInfo.Size = new System.Drawing.Size(100, 23);
            this.txtPageInfo.TabIndex = 5;
            // 
            // PageTurn
            // 
            this.Controls.Add(this.txtPageInfo);
            this.Controls.Add(this.pbPage_End);
            this.Controls.Add(this.pbPage_Prev);
            this.Controls.Add(this.pbPage_Next);
            this.Controls.Add(this.pbPage_Begin);
            this.Controls.Add(this.labPageInfo);
            this.Name = "PageTurn";
            this.Size = new System.Drawing.Size(1136, 238);
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_Begin)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_Next)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_Prev)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbPage_End)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private void pbPage_Next_Click(object sender, EventArgs e)
        {

        }
    }
}