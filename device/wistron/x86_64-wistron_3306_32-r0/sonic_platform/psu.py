#!/usr/bin/env python

#############################################################################
# psuutil.py
# Platform-specific PSU status interface for SONiC
#############################################################################

import os.path
import sonic_platform

try:
    from sonic_platform_base.psu_base import PsuBase
    from sonic_platform.fan import Fan
except ImportError as e:
    raise ImportError(str(e) + "- required module not found")

PSU_NAME_LIST = ["PSU-1", "PSU-2"]

PSU_HWMON_PATH = "/sys/bus/i2c/devices/{0}-00{1}/hwmon"
PSU_SYSFS_PATH = "/sys/bus/i2c/devices/{0}-00{1}"
FPGA_SYSFS_DIR = "/sys/bus/i2c/devices/1-0041"

PSU_I2C_MAPPING = {
    0: {
        "bus": 6,
        "addr": "58"
    },
    1: {
        "bus": 6,
        "addr": "59"
    },
}

class Psu(PsuBase):
    """Platform-specific Psu class"""
    def __init__(self, psu_index):
        self.index = psu_index
        PsuBase.__init__(self)
        self.i2c_num = PSU_I2C_MAPPING[self.index]["bus"]
        self.i2c_addr = PSU_I2C_MAPPING[self.index]["addr"]
        self.hwmon_path = PSU_HWMON_PATH.format(self.i2c_num, self.i2c_addr)
        self.sysfs_path = PSU_SYSFS_PATH.format(self.i2c_num, self.i2c_addr)

    def __read_txt_file(self, file_path):
        try:
            with open(file_path, 'r') as fd:
                data = fd.read()
                return data.strip()
        except IOError:
            pass
        return None

    def __search_hwmon_dir_name(self, directory):
        try:
            dirs = os.listdir(directory)
            for file in dirs:
                if file.startswith("hwmon"):
                    return file
        except:
            pass
        return ''

    def get_fan(self):
        """
        Retrieves object representing the fan module contained in this PSU
        Returns:
            An object dervied from FanBase representing the fan module
            contained in this PSU
        """
        # Hardware not supported
        return False

    def get_powergood_status(self):
        """
        Retrieves the powergood status of PSU
        Returns:
            A boolean, True if PSU has stablized its output voltages and passed all
            its internal self-tests, False if not.
        """
        return self.get_status()

    def set_status_led(self, color):
        """
        Sets the state of the PSU status LED
        Args:
            color: A string representing the color with which to set the PSU status LED
                   Note: Only support green and off
        Returns:
            bool: True if status LED state is set successfully, False if not
        """
        # Hardware not supported
        return False

    def get_status_led(self):
        """
        Gets the state of the PSU status LED
        Returns:
            A string, one of the predefined STATUS_LED_COLOR_* strings above
        """
        if self.get_presence():
            if self.get_powergood_status():
                return self.STATUS_LED_COLOR_GREEN
            else:
                return self.STATUS_LED_COLOR_RED
        else:
            return None

    def get_name(self):
        """
        Retrieves the name of the device
            Returns:
            string: The name of the device
        """
        return PSU_NAME_LIST[self.index]

    def get_presence(self):
        """
        Retrieves the presence of the PSU
        Returns:
            bool: True if PSU is present, False if not
        """
        attr_file ='psu{}_present'.format(self.index)
        attr_path = FPGA_SYSFS_DIR +'/' + attr_file
        status = 0
        try:
            with open(attr_path, 'r') as psu_prs:
                status = int(psu_prs.read())
        except IOError:
            return False

        return status == 1

    def get_status(self):
        """
        Retrieves the operational status of the device
        Returns:
            A boolean value, True if device is operating properly, False if not
        """
        attr_file ='psu{}_power_good'.format(self.index)
        attr_path = FPGA_SYSFS_DIR +'/' + attr_file
        status = 0
        if self.get_presence():
            try:
                with open(attr_path, 'r') as power_status:
                    status = int(power_status.read())
            except IOError:
                return False

        return status == 1

    def get_model(self):
        """
        Retrieves the model name of the PSU
        Returns:
            model name
        """
        model = ""
        if self.get_presence():
            file_path = os.path.join(self.sysfs_path, "mfr_model")
            if os.path.exists(file_path):
                model = self.__read_txt_file(file_path)
        return model

    def get_serial(self):
        """
        Retrieves the model name of the PSU
        Returns:
            model serial
        """
        serial = ""
        if self.get_presence():
            file_path = os.path.join(self.sysfs_path, "mfr_serial")
            if os.path.exists(file_path):
                serial = self.__read_txt_file(file_path)
        return serial

    def get_voltage(self):
        """
        Retrieves current PSU voltage output
        Returns:
            A float number, the output voltage in volts,
            e.g. 12.1
        """
        hwmon_dir = self.__search_hwmon_dir_name(self.hwmon_path)
        if hwmon_dir == '':
            return None

        file_path = os.path.join(self.hwmon_path, hwmon_dir,
            "in2_input")
        voltage = self.__read_txt_file(file_path)
        if not voltage:
            return None
        return float(voltage) / 1000

    def get_temperature(self):
        """
        Retrieves current temperature reading from PSU
        Returns:
            A float number of current temperature in Celsius up to nearest thousandth
            of one degree Celsius, e.g. 30.125
        """
        hwmon_dir = self.__search_hwmon_dir_name(self.hwmon_path)
        if hwmon_dir == '':
            return None
        
        file_path = os.path.join(self.hwmon_path, hwmon_dir,
            "temp1_input")
        temp = self.__read_txt_file(file_path)
        if not temp:
            return None
        return float(temp) / 1000

    def get_current(self):
        """
        Retrieves present electric current supplied by PSU
        Returns:
            A float number, the electric current in amperes, e.g 15.4
        """
        hwmon_dir = self.__search_hwmon_dir_name(self.hwmon_path)
        if hwmon_dir == '':
            return None

        file_path = os.path.join(self.hwmon_path, hwmon_dir,
            "curr2_input")
        current = self.__read_txt_file(file_path)
        if not current:
            return None
        return float(current) / 1000

    def get_power(self):
        """
        Retrieves current energy supplied by PSU
        Returns:
            A float number, the power in watts, e.g. 302.6
        """
        if not self.get_status():
            return None

        hwmon_dir = self.__search_hwmon_dir_name(self.hwmon_path)
        if hwmon_dir == '':
            return None

        file_path = os.path.join(self.hwmon_path, hwmon_dir,
            "power2_input")
        power = self.__read_txt_file(file_path)
        if not power:
            return None
        return float(power) / 1000000