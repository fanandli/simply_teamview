# Register your models here.
from .models import Information
from .models import ip_state
from .models import switch
from .models import Department
from django.utils.safestring import mark_safe

from extra_apps.ExecuteSwitchCommds import execSwitchCmd
import xadmin

#global aclNumber = 1000   #ACL全局号，防止表冲突 

class DepartmentInformation(object):
    list_display = ('department', 'department_detail', 'revise_date')
    search_fields = ('department', 'department_detail')
    list_filter = ('department', 'department_detail')
    pass


class IpInformation(object):
    list_display = ('ip_number', 'ip_state', 'ip_department', 'revise_date')
    search_fields = ('ip_number', 'ip_state', 'ip_department')
    list_filter = ('ip_number', 'ip_state', 'ip_department')
    pass


class SwitchInformation(object):
    list_display = ('switch_number', 'seat_number', 'department', 'type_switch', 'state_ip', 'state_mac', 'revise_date')
    search_fields = ('switch_number', 'seat_number', 'department',  'type_switch', 'factory',  'state_mac',
                     'revise_date')
    list_filter = ('switch_number', 'seat_number', 'department',  'type_switch', 'factory',  'state_mac', 'revise_date')


class StaffInformation(object):
    list_display = ('staff_name', 'job_number', 'Department', 'seat_number', 'mac_address', 'ip_address', 'port_switch',
                    'revise_date')
    search_fields = ('staff_name', 'job_number', 'Department', 'port_switch', 'ip_address')
    list_filter = ('Department', 'port_switch', 'staff_name', 'ip_address', 'port_switch')

    def getCommands(self, obj, flag):
        """
        根据交换机品牌，产生对应的命令集
        """
        brand = obj.port_switch_id.factory
        commands = []
        try:
            if brand == 'Huawei':
                commands.append('system-view')
                if flag:
                    commands.append('user-bind static ip-address {0} mac-address {1}'.format(obj.ip_address_id, obj.mac_address))
                    commands.append('mac number 4115')
                    commands.append('rule 5 permit souce-mac {0}'.obj.mac_address)
                    commands.append('rule 10 deny')
                    commands.append('interface {0}'.format(obj.port_switch))  #GigabitEthernet0/0/15
                else:
                    commands.append('undo user-bind static ip-address {0} mac-address {1}'.format(obj.ip_address_id, obj.mac_address))
                    """
                    解绑命令
                    commands.append('mac number 4115')
                    commands.append('rule 5 permit souce-mac {0}'.obj.mac_address)
                    commands.append('rule 10 deny')
                    commands.append('trcffic-filter inbound acl 4115')  #acl number for mac:4000 ~ 4999!
                    """
            elif brand == 'Cisco':
                commands.append('config terminal')
                if flag:
                    commands.appned('arp {0} {1} ARPA'.format(obj.ip_address_id, obj.mac_address))
                    commands.append('mac access-list extended mac01')
                    commands.append('permit host {0} any'.format(obj.mac_address))
                    commands.append('deny any any')
                    commands.append('interface {0}'.format(obj.port_switch))  #FastEthernet0/1
                    commands.append('mac access-group mac01 in')
                else:
                    commands.appned('no arp {0} {1} ARPA'.format(obj.ip_address_id, obj.mac_address))
                    """
                    解绑命令
                    commands.append('mac access-list extended mac01')
                    commands.append('permit host {0} any'.format(obj.mac_address))
                    commands.append('deny any any')
                    commands.append('interface {0}'.format(obj.port_switch))  #FastEthernet0/1
                    commands.append('mac access-group mac01 in')
                    """
        except BrandError as e:
            print('---------->', e)
        return commands

    def save_models(self):
        """
        在保存员工信息时，调用mac和交换机绑定的功能，同时改变ip和switch的状态为已分配。
        :return:
        """
        obj = self.new_obj
        request = self.request
        obj.save()
        # bind_ip_mac()
        # bind_mac_switch()
        # 必须确定存在
        obj.ip_address.ip_state = 0
        obj.port_switch.state_ip = obj.ip_address.ip_number
        obj.port_switch.state_mac = obj.mac_address
        # 执行交换机命令 绑定用户ip和mac 绑定交换机和mac
        
        flag = true
        commands = getCommands(obj, flag)
        """
        commands.append('system-view')
        commands.append('ip-mac binding enable')
        commands.append('ip-mac binding ipv4 {0} mac-address {1}'.format(obj.ip_address.ip_number, obj.mac_address))
        commands.append('am user-bind mac-address {0} interface {1}'.format(obj.mac_address, obj.port_switch.port_number))
        """
        execSwitchCmd(obj.port_switch.state_ip, obj.port_switch.account_switch, obj.port_switch.password_switch,
                      commands)
        
        obj.ip_address.save()
        obj.port_switch.save()

    def delete_model(self):
        """
        在删除员工信息时，调用mac和交换机解绑的功能，同时，改变ip和switch的状态为未分配。
        :return:
        """
        obj = self.obj
        
        flag = false
        commands = getCommands(obj, flag)
        """
        commands.append('system-view')
        commands.append('ip-mac binding enable')
        commands.append('undo ip-mac binding ipv4 {0} mac-address {1}'.format(obj.ip_address.ip_number, obj.mac_address))
        commands.append(
            'undo am user-bind mac-address {0} interface {1}'.format(obj.mac_address, obj.port_switch.port_number))
        """
        # 执行用户ip和mac绑定
        execSwitchCmd(obj.port_switch.state_ip, obj.port_switch.account_switch, obj.port_switch.password_switch,
                      commands)
        obj.ip_address.ip_state = 1
        obj.port_switch.state_ip = " "
        obj.port_switch.state_mac = " "

        obj.ip_address.save()
        obj.port_switch.save()
        obj.delete()
        pass


class GlobalSettings(object):
    site_title = "网络后台管理系统"
    site_footer = "版权所有"
    # menu_style = "accordion"


class BaseSettings(object):
    enable_themes = True
    use_bootswatch = True


xadmin.site.register(Department, DepartmentInformation)
xadmin.site.register(ip_state, IpInformation)
xadmin.site.register(switch, SwitchInformation)
xadmin.site.register(Information, StaffInformation)
xadmin.site.register(xadmin.views.CommAdminView, GlobalSettings)
xadmin.site.register(xadmin.views.BaseAdminView, BaseSettings)
# Register your models here.
