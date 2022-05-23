#-------------------------------------------------------------------------------
# * Copyright (c) Wistron, Inc., 2021
# *
# * This material is proprietary to Wistron. All rights reserved.
# * The methods and techniques described herein are considered trade secrets
# * and/or confidential. Reproduction or distribution, in whole or in part, is
# * forbidden except by express written permission of Wistron.
# *-----------------------------------------------------------------------------

"""SONiC monitor script to control LED on/off/blinking when link-up/link-down/flowing-traffic
"""
from __future__ import print_function
import time
import sys
import subprocess

CPLD1_PATH = '/sys/bus/i2c/devices/10-0053/'
CPLD2_PATH = '/sys/bus/i2c/devices/26-0053/'

port_led_ctrl_dict = {
'_PORT_CPLD_LED_CTRL_ISG0_0' : '/sys/bus/i2c/devices/26-0053/qsfp14_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG0_1' : '/sys/bus/i2c/devices/26-0053/qsfp14_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG0_2' : '/sys/bus/i2c/devices/26-0053/qsfp14_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG0_3' : '/sys/bus/i2c/devices/26-0053/qsfp14_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG0_4' : '/sys/bus/i2c/devices/26-0053/qsfp15_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG0_5' : '/sys/bus/i2c/devices/26-0053/qsfp15_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG0_6' : '/sys/bus/i2c/devices/26-0053/qsfp15_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG0_7' : '/sys/bus/i2c/devices/26-0053/qsfp15_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG1_0' : '/sys/bus/i2c/devices/26-0053/qsfp12_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG1_1' : '/sys/bus/i2c/devices/26-0053/qsfp12_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG1_2' : '/sys/bus/i2c/devices/26-0053/qsfp12_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG1_3' : '/sys/bus/i2c/devices/26-0053/qsfp12_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG1_4' : '/sys/bus/i2c/devices/26-0053/qsfp13_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG1_5' : '/sys/bus/i2c/devices/26-0053/qsfp13_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG1_6' : '/sys/bus/i2c/devices/26-0053/qsfp13_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG1_7' : '/sys/bus/i2c/devices/26-0053/qsfp13_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG2_0' : '/sys/bus/i2c/devices/26-0053/qsfp10_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG2_1' : '/sys/bus/i2c/devices/26-0053/qsfp10_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG2_2' : '/sys/bus/i2c/devices/26-0053/qsfp10_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG2_3' : '/sys/bus/i2c/devices/26-0053/qsfp10_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG2_4' : '/sys/bus/i2c/devices/26-0053/qsfp11_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG2_5' : '/sys/bus/i2c/devices/26-0053/qsfp11_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG2_6' : '/sys/bus/i2c/devices/26-0053/qsfp11_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG2_7' : '/sys/bus/i2c/devices/26-0053/qsfp11_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG3_0' : '/sys/bus/i2c/devices/26-0053/qsfp8_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG3_1' : '/sys/bus/i2c/devices/26-0053/qsfp8_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG3_2' : '/sys/bus/i2c/devices/26-0053/qsfp8_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG3_3' : '/sys/bus/i2c/devices/26-0053/qsfp8_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG3_4' : '/sys/bus/i2c/devices/26-0053/qsfp9_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG3_5' : '/sys/bus/i2c/devices/26-0053/qsfp9_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG3_6' : '/sys/bus/i2c/devices/26-0053/qsfp9_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG3_7' : '/sys/bus/i2c/devices/26-0053/qsfp9_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG4_0' : '/sys/bus/i2c/devices/26-0053/qsfp6_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG4_1' : '/sys/bus/i2c/devices/26-0053/qsfp6_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG4_2' : '/sys/bus/i2c/devices/26-0053/qsfp6_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG4_3' : '/sys/bus/i2c/devices/26-0053/qsfp6_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG4_4' : '/sys/bus/i2c/devices/26-0053/qsfp7_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG4_5' : '/sys/bus/i2c/devices/26-0053/qsfp7_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG4_6' : '/sys/bus/i2c/devices/26-0053/qsfp7_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG4_7' : '/sys/bus/i2c/devices/26-0053/qsfp7_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG5_0' : '/sys/bus/i2c/devices/26-0053/qsfp4_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG5_1' : '/sys/bus/i2c/devices/26-0053/qsfp4_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG5_2' : '/sys/bus/i2c/devices/26-0053/qsfp4_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG5_3' : '/sys/bus/i2c/devices/26-0053/qsfp4_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG5_4' : '/sys/bus/i2c/devices/26-0053/qsfp5_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG5_5' : '/sys/bus/i2c/devices/26-0053/qsfp5_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG5_6' : '/sys/bus/i2c/devices/26-0053/qsfp5_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG5_7' : '/sys/bus/i2c/devices/26-0053/qsfp5_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG6_0' : '/sys/bus/i2c/devices/26-0053/qsfp2_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG6_1' : '/sys/bus/i2c/devices/26-0053/qsfp2_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG6_2' : '/sys/bus/i2c/devices/26-0053/qsfp2_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG6_3' : '/sys/bus/i2c/devices/26-0053/qsfp2_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG6_4' : '/sys/bus/i2c/devices/26-0053/qsfp3_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG6_5' : '/sys/bus/i2c/devices/26-0053/qsfp3_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG6_6' : '/sys/bus/i2c/devices/26-0053/qsfp3_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG6_7' : '/sys/bus/i2c/devices/26-0053/qsfp3_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG7_0' : '/sys/bus/i2c/devices/26-0053/qsfp0_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG7_1' : '/sys/bus/i2c/devices/26-0053/qsfp0_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG7_2' : '/sys/bus/i2c/devices/26-0053/qsfp0_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG7_3' : '/sys/bus/i2c/devices/26-0053/qsfp0_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG7_4' : '/sys/bus/i2c/devices/26-0053/qsfp1_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG7_5' : '/sys/bus/i2c/devices/26-0053/qsfp1_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG7_6' : '/sys/bus/i2c/devices/26-0053/qsfp1_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG7_7' : '/sys/bus/i2c/devices/26-0053/qsfp1_lane3_led_rgb',

###########################################################################################
'_PORT_CPLD_LED_CTRL_ISG8_0' : '/sys/bus/i2c/devices/10-0053/qsfp14_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG8_1' : '/sys/bus/i2c/devices/10-0053/qsfp14_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG8_2' : '/sys/bus/i2c/devices/10-0053/qsfp14_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG8_3' : '/sys/bus/i2c/devices/10-0053/qsfp14_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG8_4' : '/sys/bus/i2c/devices/10-0053/qsfp15_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG8_5' : '/sys/bus/i2c/devices/10-0053/qsfp15_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG8_6' : '/sys/bus/i2c/devices/10-0053/qsfp15_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG8_7' : '/sys/bus/i2c/devices/10-0053/qsfp15_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG9_0' : '/sys/bus/i2c/devices/10-0053/qsfp12_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG9_1' : '/sys/bus/i2c/devices/10-0053/qsfp12_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG9_2' : '/sys/bus/i2c/devices/10-0053/qsfp12_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG9_3' : '/sys/bus/i2c/devices/10-0053/qsfp12_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG9_4' : '/sys/bus/i2c/devices/10-0053/qsfp13_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG9_5' : '/sys/bus/i2c/devices/10-0053/qsfp13_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG9_6' : '/sys/bus/i2c/devices/10-0053/qsfp13_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG9_7' : '/sys/bus/i2c/devices/10-0053/qsfp13_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG10_0' : '/sys/bus/i2c/devices/10-0053/qsfp10_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG10_1' : '/sys/bus/i2c/devices/10-0053/qsfp10_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG10_2' : '/sys/bus/i2c/devices/10-0053/qsfp10_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG10_3' : '/sys/bus/i2c/devices/10-0053/qsfp10_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG10_4' : '/sys/bus/i2c/devices/10-0053/qsfp11_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG10_5' : '/sys/bus/i2c/devices/10-0053/qsfp11_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG10_6' : '/sys/bus/i2c/devices/10-0053/qsfp11_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG10_7' : '/sys/bus/i2c/devices/10-0053/qsfp11_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG11_0' : '/sys/bus/i2c/devices/10-0053/qsfp8_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG11_1' : '/sys/bus/i2c/devices/10-0053/qsfp8_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG11_2' : '/sys/bus/i2c/devices/10-0053/qsfp8_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG11_3' : '/sys/bus/i2c/devices/10-0053/qsfp8_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG11_4' : '/sys/bus/i2c/devices/10-0053/qsfp9_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG11_5' : '/sys/bus/i2c/devices/10-0053/qsfp9_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG11_6' : '/sys/bus/i2c/devices/10-0053/qsfp9_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG11_7' : '/sys/bus/i2c/devices/10-0053/qsfp9_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG12_0' : '/sys/bus/i2c/devices/10-0053/qsfp6_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG12_1' : '/sys/bus/i2c/devices/10-0053/qsfp6_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG12_2' : '/sys/bus/i2c/devices/10-0053/qsfp6_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG12_3' : '/sys/bus/i2c/devices/10-0053/qsfp6_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG12_4' : '/sys/bus/i2c/devices/10-0053/qsfp7_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG12_5' : '/sys/bus/i2c/devices/10-0053/qsfp7_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG12_6' : '/sys/bus/i2c/devices/10-0053/qsfp7_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG12_7' : '/sys/bus/i2c/devices/10-0053/qsfp7_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG13_0' : '/sys/bus/i2c/devices/10-0053/qsfp4_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG13_1' : '/sys/bus/i2c/devices/10-0053/qsfp4_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG13_2' : '/sys/bus/i2c/devices/10-0053/qsfp4_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG13_3' : '/sys/bus/i2c/devices/10-0053/qsfp4_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG13_4' : '/sys/bus/i2c/devices/10-0053/qsfp5_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG13_5' : '/sys/bus/i2c/devices/10-0053/qsfp5_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG13_6' : '/sys/bus/i2c/devices/10-0053/qsfp5_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG13_7' : '/sys/bus/i2c/devices/10-0053/qsfp5_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG14_0' : '/sys/bus/i2c/devices/10-0053/qsfp2_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG14_1' : '/sys/bus/i2c/devices/10-0053/qsfp2_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG14_2' : '/sys/bus/i2c/devices/10-0053/qsfp2_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG14_3' : '/sys/bus/i2c/devices/10-0053/qsfp2_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG14_4' : '/sys/bus/i2c/devices/10-0053/qsfp3_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG14_5' : '/sys/bus/i2c/devices/10-0053/qsfp3_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG14_6' : '/sys/bus/i2c/devices/10-0053/qsfp3_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG14_7' : '/sys/bus/i2c/devices/10-0053/qsfp3_lane3_led_rgb',

'_PORT_CPLD_LED_CTRL_ISG15_0' : '/sys/bus/i2c/devices/10-0053/qsfp0_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG15_1' : '/sys/bus/i2c/devices/10-0053/qsfp0_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG15_2' : '/sys/bus/i2c/devices/10-0053/qsfp0_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG15_3' : '/sys/bus/i2c/devices/10-0053/qsfp0_lane3_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG15_4' : '/sys/bus/i2c/devices/10-0053/qsfp1_lane0_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG15_5' : '/sys/bus/i2c/devices/10-0053/qsfp1_lane1_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG15_6' : '/sys/bus/i2c/devices/10-0053/qsfp1_lane2_led_rgb',
'_PORT_CPLD_LED_CTRL_ISG15_7' : '/sys/bus/i2c/devices/10-0053/qsfp1_lane3_led_rgb'
}

port_speed_led_ctrl_dict = {
'_PORT_CPLD_COLOR_CTRL_200G_50G' : 0x5,
'_PORT_CPLD_COLOR_CTRL_100G_50G' : 0x2,
'_PORT_CPLD_COLOR_CTRL_50G_50G'  : 0x7,
'_PORT_CPLD_COLOR_CTRL_100G_25G' : 0x6,
'_PORT_CPLD_COLOR_CTRL_25G_25G'  : 0x1
}

def platform_configure_switch_led_control_from_TL5_to_CPU():
    # set led control from MAC to manual
    cmd_list = "echo 0 >" + CPLD1_PATH + "led_manual_enable;"
    cmd_list += "echo 0 >" + CPLD2_PATH + "led_manual_enable;"
    '''
    for i in range(0,16):
      for j in range(0,4):
        cmd_list += "echo 0 >" + CPLD1_PATH + "qsfp{0}_lane{1}_led_act;".format(i,j)
        cmd_list += "echo 0 >" + CPLD1_PATH + "qsfp{0}_lane{1}_led_rgb;".format(i,j)
    for i in range(0,16):
      for j in range(0,4):
        cmd_list += "echo 0 >" + CPLD2_PATH + "qsfp{0}_lane{1}_led_act;".format(i,j)
        cmd_list += "echo 0 >" + CPLD2_PATH + "qsfp{0}_lane{1}_led_rgb;".format(i,j)
    '''
    return cmd_list

#oper up -> check speed -> check act
def platform_configure_led(isg, start_lane, oper, speed, act_status):
    _speed = speed.split('/')
    port_led_cpld_fs =  '_PORT_CPLD_LED_CTRL_' + isg + '_' + start_lane
    port_led_act_cpld_fs = port_led_ctrl_dict[port_led_cpld_fs].rsplit('_', 1)[0] + '_act'
    port_led_color_ctrl_fs = '_PORT_CPLD_COLOR_CTRL_' + _speed[0] + '_' + _speed[1]
    color_code = port_speed_led_ctrl_dict[port_led_color_ctrl_fs]
    if oper == 'U':
        cmd_list = "echo " + color_code + " > " + port_led_ctrl_dict[port_led_cpld_fs] + ";"
        if act_status == 1:
            cmd_list += "echo 1 > " + port_led_act_cpld_fs
        else:
            cmd_list += "echo 0 > " + port_led_act_cpld_fs
    else:
        cmd_list = "echo 0 > " + port_led_ctrl_dict[port_led_cpld_fs] + ";"
    return cmd_list
