#!/usr/bin/env python

#############################################################################
#
# Thermal contains an implementation of SONiC Platform Base API and
# provides the thermal device status which are available in the platform
#
#############################################################################

import os
import re
import os.path

try:
    from sonic_platform_base.thermal_base import ThermalBase
except ImportError as e:
    raise ImportError(str(e) + "- required module not found")


class Thermal(ThermalBase):
    """Platform-specific Thermal class"""

    THERMAL_NAME_LIST = []
    SYSFS_THERMAL_DIR = ["/sys/bus/i2c/devices/3-0049/hwmon/",
                         "/sys/bus/i2c/devices/3-004a/hwmon/",
                         "/sys/bus/i2c/devices/3-004b/hwmon/",
                         "/sys/bus/i2c/devices/3-004f/hwmon/",
                         "/sys/bus/i2c/devices/9-0049/hwmon/"]

    def __init__(self, thermal_index):
        self.index = thermal_index

        # Add thermal name
        self.THERMAL_NAME_LIST.append("TMP75-0x49")
        self.THERMAL_NAME_LIST.append("TMP75-0x4A")
        self.THERMAL_NAME_LIST.append("TMP75-0x4B")
        self.THERMAL_NAME_LIST.append("TMP75-0x4F")
        self.THERMAL_NAME_LIST.append("LM75-0x49")
        ThermalBase.__init__(self)

    def __read_txt_file(self, file_path):
        try:
            with open(file_path, 'r') as fd:
                data = fd.read()
                return data.strip()
        except IOError:
            pass
        return ""

    def __search_hwmon_dir_name(self, directory):
        try:
            dirs = os.listdir(directory)
            for file in dirs:
                if file.startswith("hwmon"):
                    return file
        except:
            pass
        return ''

    def __get_temp(self, temp_file):
        hwmon_dir = self.__search_hwmon_dir_name(self.SYSFS_THERMAL_DIR[self.index])
        temp_file_path = os.path.join(self.SYSFS_THERMAL_DIR[self.index], hwmon_dir, temp_file)
        raw_temp = self.__read_txt_file(temp_file_path)
        temp = float(raw_temp)/1000
        return "{:.3f}".format(temp)

    def __set_threshold(self, file_name, temperature):
        temp_file_path = os.path.join(self.hwmon_path, file_name)
        try:
            with open(temp_file_path, 'w') as fd:
                fd.write(str(temperature))
            return True
        except IOError:
            return False

    def get_temperature(self):
        """
        Retrieves current temperature reading from thermal
        Returns:
            A float number of current temperature in Celsius up to nearest thousandth
            of one degree Celsius, e.g. 30.125
        """
        temp_file = "temp1_input"
        return self.__get_temp(temp_file)

    def get_high_threshold(self):
        """
        Retrieves the high threshold temperature of thermal
        Returns:
            A float number, the high threshold temperature of thermal in Celsius
            up to nearest thousandth of one degree Celsius, e.g. 30.125
        """
        temp_file = "temp1_max"
        return self.__get_temp(temp_file)

    def set_high_threshold(self, temperature):
        """
        Sets the high threshold temperature of thermal
        Args :
            temperature: A float number up to nearest thousandth of one degree Celsius,
            e.g. 30.125
        Returns:
            A boolean, True if threshold is set successfully, False if not
        """
        # Hardware not supported
        return False

    def get_name(self):
        """
        Retrieves the name of the thermal device
            Returns:
            string: The name of the thermal device
        """
        return self.THERMAL_NAME_LIST[self.index]

    def get_presence(self):
        """
        Retrieves the presence of the PSU
        Returns:
            bool: True if PSU is present, False if not
        """
        temp_file = "temp1_input"
        temp_file_path = os.path.join(self.hwmon_path, temp_file)
        return os.path.isfile(temp_file_path)

    def get_status(self):
        """
        Retrieves the operational status of the device
        Returns:
            A boolean value, True if device is operating properly, False if not
        """
        if not self.get_presence():
            return False

        return True

