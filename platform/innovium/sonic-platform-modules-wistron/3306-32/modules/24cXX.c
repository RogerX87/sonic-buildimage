/***************************************************************************
    copyright            : (C) by 2002-2003 Stefano Barbato
    email                : stefano@codesink.org

    $Id: 24cXX.c 6228 2014-02-20 08:37:15Z khali $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/i2c-dev.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/unistd.h>
#include <linux/file.h>
#include "24cXX.h"

static int i2c_write_1b(struct i2c_client *client, __u8 buf)
{
   int r;
   // we must simulate a plain I2C byte write with SMBus functions
   r = i2c_smbus_write_byte(client, buf);
   if(r < 0)
       printk(KERN_ERR "Error i2c_write_1b\n");
   mdelay(SYS_EEPROM_I2C_WRITE_1B_DELAY);
   return r;
}

static int i2c_write_2b(struct i2c_client *client, __u8 buf[2])
{
   int r;
   // we must simulate a plain I2C byte write with SMBus functions
   r = i2c_smbus_write_byte_data(client, buf[0], buf[1]);
   if(r < 0)
       printk(KERN_ERR "Error i2c_write_2b\n");
   mdelay(SYS_EEPROM_I2C_WRITE_2B_DELAY);
   return r;
}

static int i2c_write_3b(struct i2c_client *client, __u8 buf[3])
{
   int r;
   // we must simulate a plain I2C byte write with SMBus functions
   // the __u16 data field will be byte swapped by the SMBus protocol
   r = i2c_smbus_write_word_data(client, buf[0], buf[2] << 8 | buf[1]);
   if(r < 0)
       printk(KERN_ERR "Error i2c_write_3b\n");
   mdelay(SYS_EEPROM_I2C_WRITE_3B_DELAY);
   return r;
}


#define CHECK_I2C_FUNC( var, label ) \
   do {    if(0 == (var & label)) { \
       printk("[CC] CHECK_I2C_FUNC fail !!!\n"); \
       return; } \
   } while(0);

#if 0
int eeprom_open(char *dev_fqn, int addr, int type, struct eeprom* e)
{
   int fd, r;
   unsigned long funcs;
   e->fd = e->addr = 0;
   e->dev = 0;

   //fd = open(dev_fqn, O_RDWR);
   fd = filp_open(dev_fqn, O_RDWR, 0);
   if(fd <= 0)
       return -1;

   // get funcs list
   if((r = ioctl(fd, I2C_FUNCS, &funcs) < 0))
       return r;


   // check for req funcs
   CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE );
   CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE );
   CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE_DATA );
   CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE_DATA );
   CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_WORD_DATA );
   CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_WORD_DATA );

   // set working device
   if( ( r = ioctl(fd, I2C_SLAVE, addr)) < 0)
       return r;
   e->fd = fd;
   e->addr = addr;
   e->dev = dev_fqn;
   e->type = type;
   return 0;
}

int eeprom_close(struct eeprom *e)
{
   //close(e->fd);
   filp_close(e->fd, NULL);
   e->fd = -1;
   e->dev = 0;
   e->type = EEPROM_TYPE_UNKNOWN;
   return 0;
}
#endif


__u8 eeprom_read_byte(struct eeprom *e , __u16 mem_addr)
{
   __u8 r = 0;

   if(e->type == EEPROM_TYPE_8BIT_ADDR)
   {
       __u8 buf =  mem_addr & 0x0ff;
       r = i2c_write_1b(e->client, buf);
   } else if(e->type == EEPROM_TYPE_16BIT_ADDR) {
       __u8 buf[2] = { (mem_addr >> 8) & 0x0ff, mem_addr & 0x0ff };
       r = i2c_write_2b(e->client, buf);
   } else {
       printk(KERN_ERR "eeprom_read_byte unknown eeprom type\n");
       return -1;
   }
   if (r < 0)
       return r;
   r = i2c_smbus_read_byte(e->client);
   //printk(KERN_DEBUG "eeprom_read_byte : %x\n", r);
   return r;
}

__u8 eeprom_read_current_byte(struct eeprom* e)
{
   return i2c_smbus_read_byte(e->client);
}

__u8 eeprom_write_byte(struct eeprom *e, __u16 mem_addr, __u8 data)
{
   if(e->type == EEPROM_TYPE_8BIT_ADDR) {
       __u8 buf[2] = { mem_addr & 0x00ff, data };
       return i2c_write_2b(e->client, buf);
   } else if(e->type == EEPROM_TYPE_16BIT_ADDR) {
       __u8 buf[3] =
           { (mem_addr >> 8) & 0x00ff, mem_addr & 0x00ff, data };
       return i2c_write_3b(e->client, buf);
   } else {
       printk(KERN_ERR "eeprom_write_byte unknown eeprom type\n");
       return -1;
   }
}