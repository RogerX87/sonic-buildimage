
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _WISTRON_SWITCH_CPLD_H
#define _WISTRON_SWITCH_CPLD_H

#define CPLD_REVISION_REG       0x0

#define CPLD_QSFP56_MODSEL_REG_1         0x10
#define CPLD_QSFP56_MODSEL_REG_2         0x11

#define CPLD_QSFP56_RESET_REG_1          0x20
#define CPLD_QSFP56_RESET_REG_2          0x21

#define CPLD_QSFP56_PRESENT_REG_1        0x30
#define CPLD_QSFP56_PRESENT_REG_2        0x31

#define CPLD_QSFP56_INTR_REG_1           0x40
#define CPLD_QSFP56_INTR_REG_2           0x41

#define CPLD_QSFP56_PRESENT_MASK_REG_1   0x50
#define CPLD_QSFP56_PRESENT_MASK_REG_2   0x51

#define CPLD_QSFP56_INTR_MASK_REG_1      0x60
#define CPLD_QSFP56_INTR_MASK_REG_2      0x61

#define CPLD_QSFP56_LPMODE_REG_1         0x70
#define CPLD_QSFP56_LPMODE_REG_2         0x71

#define CPLD_QSFF56_PLED_EN_REG          0x80

#define CPLD_QSFF56_PLED_EN_REG          0x80
#define CPLD_QSFF56_PLED_REG             0x81


int switch_cpld_read_byte_data(u8 cpld_id, u8 command);
int switch_cpld_write_byte_data(u8 cpld_id, u8 command, u8 value);

int switch_cpld_read_port_present(u8 port);
int switch_cpld_read_port_interrupt(u8 port);
int switch_cpld_read_port_lpmode(u8 port);
int switch_cpld_write_port_lpmode(u8 port, u8 value);
int switch_cpld_read_port_reset(u8 port);
int switch_cpld_write_port_reset(u8 port, u8 value);
#endif /* _WISTRON_SWITCH_CPLD_H */