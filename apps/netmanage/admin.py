# Register your models here.
from django.contrib import admin
from .models import Information
from .models import ip_state
from . import models


class StaffInformation(admin.ModelAdmin):
    list_display = ('staff_name', 'job_number', 'Department', 'mac_address', 'ip_address', 'port_switch')

    pass


class IpInformation(admin.ModelAdmin):
    list_display = ('ip_number', 'ip_state')
    pass


admin.site.register(Information, StaffInformation)
admin.site.register(ip_state, IpInformation)
admin.site.disable_action('delete_selected')
# Register your models here.
