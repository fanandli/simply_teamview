# Generated by Django 2.1.7 on 2021-01-04 20:06

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('netmanage', '0003_auto_20210104_1900'),
    ]

    operations = [
        migrations.AddField(
            model_name='switch',
            name='port_number',
            field=models.CharField(default='', max_length=20, verbose_name='端口号'),
        ),
        migrations.AlterField(
            model_name='switch',
            name='state_ip',
            field=models.CharField(blank=True, default='', max_length=20, verbose_name='所绑定的ip'),
        ),
        migrations.AlterField(
            model_name='switch',
            name='state_mac',
            field=models.CharField(blank=True, default='', max_length=20, verbose_name='所绑定的mac'),
        ),
    ]