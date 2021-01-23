import logging
from .TelnetClienter import TelnetClient


def execSwitchCmd(host, username, password, commands):
    """
    :param host:
    :param username:
    :param password:
    :param commands:
    :return:
    """

    logging.info('开始执行交换机命令【'+repr(commands)+'】')
    # return True
    telnet_client = TelnetClient()
    msg = ''
    # 如果登录结果返加True，则执行命令，然后退出
    if telnet_client.login(host, username, password):
        #msg += telnet_client.execute_some_command('system-view')
        for command in commands:
            msg += telnet_client.execute_some_command(command)
        telnet_client.logout()
    return msg

