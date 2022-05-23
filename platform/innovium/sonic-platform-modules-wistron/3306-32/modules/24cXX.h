/***************************************************************************
    copyright            : (C) by 2002-2003 Stefano Barbato
    email                : stefano@codesink.org

    $Id: 24cXX.h 6048 2012-04-26 10:10:22Z khali $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _24CXX_H_
#define _24CXX_H_
#include <linux/types.h>
#include <linux/i2c.h>

enum
{
    EEPROM_TYPE_UNKNOWN=0,
    EEPROM_TYPE_8BIT_ADDR,
    EEPROM_TYPE_16BIT_ADDR
};
/*
 * SYS_EEPROM_I2C_WRITE_1B_DELAY: delay number of miniseconds after writing 1 byte
 * SYS_EEPROM_I2C_WRITE_2B_DELAY: delay number of miniseconds after writing 2 bytes
 * SYS_EEPROM_I2C_WRITE_3B_DELAY: delay number of miniseconds after writing 3 bytes
 */
#define SYS_EEPROM_I2C_WRITE_1B_DELAY   1
#define SYS_EEPROM_I2C_WRITE_2B_DELAY   1
#define SYS_EEPROM_I2C_WRITE_3B_DELAY   1

struct eeprom
{
   char *dev;  // device file i.e. /dev/i2c-N
   int addr;   // i2c address
   struct i2c_client *client;
   int type;   // eeprom type
};

/*
 * opens the eeprom device at [dev_fqn] (i.e. /dev/i2c-N) whose address is
 * [addr] and set the eeprom_24c32 [e]
 */
int eeprom_open(char *dev_fqn, int addr, int type, struct eeprom*);
/*
 * closees the eeprom device [e]
 */
int eeprom_close(struct eeprom *e);
/*
 * read and returns the eeprom byte at memory address [mem_addr]
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address)
 */
__u8 eeprom_read_byte(struct eeprom* e, __u16 mem_addr);
/*
 * read the current byte
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address)
 */
__u8 eeprom_read_current_byte(struct eeprom *e);
/*
 * writes [data] at memory address [mem_addr]
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address)
 */
__u8 eeprom_write_byte(struct eeprom *e, __u16 mem_addr, __u8 data);

#endif