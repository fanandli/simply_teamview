using System;

namespace ClientServerBridger.CSBModels
{
    /// <summary>
    /// 客户模型类
    /// </summary>
    [Serializable]
    class ClientModel
    {

        #region 成员属性

        private string _userName; // 用户名
        private string _passWord; // 用户密码
        private string _rUserName; // 远程用户名
        private string _rPassWord; // 远程用户密码

        public string Username
        {
            get => _userName; set => _userName = value;
        }

        public string Password
        {
            get => _passWord; set => _passWord = value;
        }

        public string RUserName
        {
            get => _rUserName; set => _rUserName = value; 
        }

        public string RPassWord
        { 
            get => _rPassWord; set => _rPassWord = value; 
        }
        #endregion

        #region 构造方法

        public ClientModel()
        {

        }
         
        public ClientModel(string username, string password)
        {
            this._userName = username;
            this._passWord = password;
        }

        #endregion

    }
}
