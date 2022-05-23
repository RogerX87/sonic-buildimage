/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _WISTRON_EEPROM_H_
#define _WISTRON_EEPROM_H_
#include <linux/types.h>
#include <linux/i2c.h>
extern  int get_mgmt_mac_baidu(char *buf);
extern  int get_switch_mac_baidu(char *buf);
extern  int get_manu_date_baidu(char *buf);
extern  int get_hw_ver_baidu(char *buf);
extern  int get_product_name_baidu(char *buf);
extern  int get_serial_no_baidu(char *buf);
extern  int get_vendor_name_baidu(char *buf);
struct eeprom_platform_data {
    u8 eeprom_type;
};
#endif