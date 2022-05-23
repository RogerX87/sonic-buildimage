
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _WISTRON_PSU_H_
#define _WISTRON_PSU_H_
#include <linux/types.h>

#define PSU_ID_0    0
#define PSU_ID_1    1
#define WISTRON_PSU_TYPE_REG 0xd8

enum wistron_pmbus_regs {
    WISTRON_PMBUS_REG_READ_VIN = 0,
    WISTRON_PMBUS_REG_READ_IIN,
    WISTRON_PMBUS_REG_READ_VOUT,
    WISTRON_PMBUS_REG_READ_IOUT,
    WISTRON_PMBUS_REG_READ_TEMPERATURE_1,
    WISTRON_PMBUS_REG_READ_FAN_SPEED_1,
    WISTRON_PMBUS_REG_READ_POUT,
    WISTRON_PMBUS_REG_READ_PIN,
    WISTRON_PMBUS_REG_READ_MAX_V,
    WISTRON_PMBUS_REG_READ_MAX_I,
    WISTRON_PMBUS_REG_READ_MAX_P,
    WISTRON_PMBUS_REG_READ_TEMPERATURE_2,
    WISTRON_PMBUS_REG_READ_TEMPERATURE_3,
};

extern int wistron_psu_get_alarm(unsigned int psu_id, int *alarm);
extern long wistron_psu_get_data(unsigned int psu_id, u8 reg);
extern u8 wistron_psu_get_byte_raw(unsigned int psu_id, u8 reg);

#endif
