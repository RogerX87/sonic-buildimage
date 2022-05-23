#!/usr/bin/env python

#############################################################################
# Accton
#
# Module contains an implementation of SONiC PSU Base API and
# provides the PSUs status which are available in the platform
#
#############################################################################

import os.path

try:
    from sonic_psu.psu_base import PsuBase
except ImportError as e:
    raise ImportError (str(e) + "- required module not found")

FPGA_SYSFS_DIR = "/sys/bus/i2c/devices/1-0041"

class PsuUtil(PsuBase):
    """Platform-specific PSUutil class"""

    def __init__(self):
        PsuBase.__init__(self)

        self.psu_mapping = {
            1: "6-0058",
            2: "6-0059",
        }

    def get_num_psus(self):
        return len(self.psu_mapping)

    def get_psu_status(self, index):
        if index is None:
            return False

        attr_file ='psu{}_power_good'.format(index - 1)
        attr_path = FPGA_SYSFS_DIR +'/' + attr_file
        status = 0
        try:
            with open(attr_path, 'r') as power_status:
                status = int(power_status.read())
        except IOError:
            return False

        return status == 1

    def get_psu_presence(self, index):
        if index is None:
            return False

        attr_file ='psu{}_present'.format(index - 1)
        attr_path = FPGA_SYSFS_DIR +'/' + attr_file
        status = 0
        try:
            with open(attr_path, 'r') as psu_prs:
                status = int(psu_prs.read())
        except IOError:
            return False

        return status == 1
