# sfputil.py
#
# Platform-specific SFP transceiver interface for SONiC
#

try:
    import time
    import string
    from ctypes import create_string_buffer
    from sonic_sfp.sfputilbase import SfpUtilBase
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))

#from xcvrd
SFP_STATUS_REMOVED = '0'
SFP_STATUS_INSERTED = '1'

class SfpUtil(SfpUtilBase):
    """Platform-specific SfpUtil class"""

    PORT_START = 0
    PORT_END = 31
#    PORTS_IN_BLOCK = 56
#    QSFP_PORT_START = 49
#    QSFP_PORT_END = 56

    #BASE_VAL_PATH = "/sys/class/i2c-adapter/i2c-{0}/{1}-0050/"

    _port_to_is_present = {}
    _port_to_lp_mode = {}

    _port_to_eeprom_mapping = {}
    _cpld_mapping = {
       1:  "10-0053",
       2:  "26-0053",
           }

    _port_to_i2c_mapping = {
           0:  10,
           1:  11,
           2:  12,
           3:  13,
           4:  14,
           5:  15,
           6:  16,
           7:  17,
           8:  18,
           9:  19,
           10:  20,
           11:  21,
           12:  22,
           13:  23,
           14:  24,
           15:  25,
           16:  26,
           17:  27,
           18:  28,
           19:  29,
           20:  30,
           21:  31,
           22:  32,
           23:  33,
           24:  34,
           25:  35,
           26:  36,
           27:  37,
           28:  38,
           29:  39,
           30:  40,
           31:  41,
           }

    @property
    def port_start(self):
        return self.PORT_START

    @property
    def port_end(self):
        return self.PORT_END

#    @property
#    def qsfp_port_start(self):
#        return self.QSFP_PORT_START
#
#    @property
#    def qsfp_port_end(self):
#        return self.QSFP_PORT_END
#
#    @property
#    def qsfp_ports(self):
#        return range(self.QSFP_PORT_START, self.PORTS_IN_BLOCK + 1)

    @property
    def qsfp_ports(self):
        return range(self.PORT_START, self.PORT_END + 1)

    @property
    def port_to_eeprom_mapping(self):
        return self._port_to_eeprom_mapping

    def __init__(self):
        eeprom_path = '/sys/bus/i2c/devices/{0}-0050/eeprom'
        for x in range(self.port_start, self.port_end+1):
            self.port_to_eeprom_mapping[x] = eeprom_path.format(
                self._port_to_i2c_mapping[x])

        SfpUtilBase.__init__(self)

    # For port 0~23 and 48~51 are at cpld2, others are at cpld3.
    def get_cpld_num(self, port_num):
        cpld_i = 1
        if (port_num > 15):
            cpld_i = 2
        return cpld_i

    def get_presence(self, port_num):
        # Check for invalid port_num
        if port_num < self.port_start or port_num > self.port_end:
            return False

        cpld_i = self.get_cpld_num(port_num)

        cpld_ps = self._cpld_mapping[cpld_i]
        if port_num > 15:
            port_num = port_num - 16
        path = "/sys/bus/i2c/devices/{0}/qsfp{1}_present"
        port_ps = path.format(cpld_ps, port_num)

        try:
            val_file = open(port_ps)
        except IOError as e:
            print("Error: unable to open file: {}".format(e))
            return False

        content = val_file.readline().rstrip()
        val_file.close()

        # content is a string, either "0" or "1"
        if content == "1":
            return True

        return False

    def get_low_power_mode(self, port_num):
        # Check for invalid port_num
        if port_num < self.port_start or port_num > self.port_end:
            return False

        cpld_i = self.get_cpld_num(port_num)

        cpld_ps = self._cpld_mapping[cpld_i]
        if port_num > 15:
            port_num = port_num - 16
        path = "/sys/bus/i2c/devices/{0}/qsfp{1}_lpmode"
        port_ps = path.format(cpld_ps, port_num)

        try:
            val_file = open(port_ps)
        except IOError as e:
            print("Error: unable to open file: {}".format(e))
            return False

        content = val_file.readline().rstrip()
        val_file.close()

        # content is a string, either "0" or "1"
        if content == "1":
            return True

        return False


    def set_low_power_mode(self, port_num, lpmode):
        if port_num < self.port_start or port_num > self.port_end:
            return False

        cpld_i = self.get_cpld_num(port_num)
        cpld_ps = self._cpld_mapping[cpld_i]
        if port_num > 15:
            port_num = port_num - 16
        path = "/sys/bus/i2c/devices/{0}/qsfp{1}_lpmode"

        port_ps = path.format(cpld_ps, port_num)

        self.__port_to_mod_rst = port_ps
        try:
            reg_file = open(self.__port_to_mod_rst, 'r+')
        except IOError as e:
            print("Error: unable to open file: {}".format(e))
            return False

        #toggle reset
        reg_file.seek(0)
        if lpmode == 0:
            reg_file.write('0')
        else:
            reg_file.write('1')
        time.sleep(1)
#        reg_file.seek(0)
#        reg_file.write('0')
        reg_file.close()

        return True

    def reset(self, port_num):
        if port_num < self.port_start or port_num > self.port_end:
            return False

        cpld_i = self.get_cpld_num(port_num)
        cpld_ps = self._cpld_mapping[cpld_i]
        if port_num > 15:
            port_num = port_num - 16
        path = "/sys/bus/i2c/devices/{0}/qsfp{1}_reset"

        port_ps = path.format(cpld_ps, port_num)
        
        self.__port_to_mod_rst = port_ps
        try:
            reg_file = open(self.__port_to_mod_rst, 'r+')
        except IOError as e:
            print("Error: unable to open file: {}".format(e))
            return False

        #toggle reset
        reg_file.seek(0)
        reg_file.write('1')
        time.sleep(1)
        reg_file.seek(0)
        reg_file.write('0')
        reg_file.close()
        
        return True
    @property
    def _get_present_bitmap(self):
        bitmaps = ""
        rev = []
        for port_num in range(self.port_start, self.port_end + 1):
            cpld_i = self.get_cpld_num(port_num)

            cpld_ps = self._cpld_mapping[cpld_i]
            if port_num > 15:
                port_num = port_num - 16
            path = "/sys/bus/i2c/devices/{0}/qsfp{1}_present"
            port_ps = path.format(cpld_ps, port_num)

            try:
                val_file = open(port_ps)
            except IOError as e:
                print("Error: unable to open file: {}".format(e))
                return False

            bitmap = val_file.readline().rstrip()
            rev.append(bitmap)
            val_file.close()

        bitmaps = "".join(rev[::-1])
        bitmaps = hex(int(bitmaps, 2))
        return int(bitmaps, 0)

    data = {'valid':0, 'last':0, 'present':0}
    def get_transceiver_change_event(self, timeout=2000):
        now = time.time()
        port_dict = {}
        port = 0

        if timeout < 1000:
            timeout = 1000
        timeout = (timeout) / float(1000) # Convert to secs

        if now < (self.data['last'] + timeout) and self.data['valid']:
            return True, {}

        reg_value = self._get_present_bitmap
        changed_ports = self.data['present'] ^ reg_value
        if changed_ports:
            for port in range (self.port_start, self.port_end+1):
                # Mask off the bit corresponding to our port
                mask = (1 << port)
                if changed_ports & mask:
                    if (reg_value & mask) == 0:
                        port_dict[port] = SFP_STATUS_REMOVED
                    else:
                        port_dict[port] = SFP_STATUS_INSERTED

            # Update cache
            self.data['present'] = reg_value
            self.data['last'] = now
            self.data['valid'] = 1
            return True, port_dict
        else:
            return True, {}
        return False, {}
