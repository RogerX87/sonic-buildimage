/*
 * Copyright (C) 2018 Inspur Electronic Information Industry Co.,Ltd
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
/* ----------------------------------------------------------------------------
 * Include files
 * ----------------------------------------------------------------------------
 */
#include <linux/module.h>   // included for all kernel modules
#include <linux/kernel.h>   // included for KERN_INFO
#include <linux/init.h>     // included for __init and __exit macros
#include <linux/i2c.h>      // For I2C_CLIENT_END
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/string.h>
#include "24cXX.h"
#include "wistron_eeprom.h"
/* ----------------------------------------------------------------------------
 * Constant
 * ----------------------------------------------------------------------------
 */
#define _DRIVER_NAME                "wistron_eeprom"
#define TLV_EEPROM_SIZE 1024
/* The TLV type code. */
#define UTL_ONIE_PRODUCT_NAME   0x21
#define UTL_ONIE_PART_NUMBER    0x22
#define UTL_ONIE_SERIAL_NUMBER  0x23
#define UTL_ONIE_MAC_BASE       0x24
#define UTL_ONIE_MANUF_DATE     0x25
#define UTL_ONIE_DEVICE_VERSION 0x26
#define UTL_ONIE_LABEL_REVISION 0x27
#define UTL_ONIE_PLATFORM_NAME  0x28
#define UTL_ONIE_ONIE_VERSION   0x29
#define UTL_ONIE_MAC_SIZE       0x2A
#define UTL_ONIE_MANUF_NAME     0x2B
#define UTL_ONIE_MANUF_COUNTRY  0x2C
#define UTL_ONIE_VENDOR_NAME    0x2D
#define UTL_ONIE_DIAG_VERSION   0x2E
#define UTL_ONIE_SERVICE_TAG    0x2F
#define UTL_ONIE_VENDOR_EXT     0xFD
#define UTL_ONIE_CRC32          0xFE
/* Header field constants. */
#define UTL_ONIE_HEADER_SIGNATURE_LEN   8
#define UTL_ONIE_HEADER_ID_STRING       "TlvInfo"
#define UTL_ONIE_HEADER_VERSION         0x01
#define UTL_ONIE_HEADER_TOTAL_LEN       2
#define UTL_ONIE_TLV_TOTAL_LEN_MAX      (TLV_EEPROM_SIZE - TLV_HEADER_LEN)
#define UTL_ONIE_TLV_VALUE_MAX 255
typedef struct TLV_HEADER
{
    char    signature[UTL_ONIE_HEADER_SIGNATURE_LEN];
    u8    version;
    u8    total_length[UTL_ONIE_HEADER_TOTAL_LEN];
}TLV_HEADER_t ;
#define TLV_HEADER_LEN         sizeof(TLV_HEADER_t)
typedef struct TLV_RECORD
{
    u8                           type;
    u8                           length;
    u8                           value[0];
} TLV_RECORD_T;
#define TLV_RECORD_LEN            sizeof(TLV_RECORD_T)
char TLV_data[TLV_EEPROM_SIZE];
/* ----------------------------------------------------------------------------
 * Local debug
 * ----------------------------------------------------------------------------
 */
//#define _LOCAL_DEBUG
#ifdef _LOCAL_DEBUG
#define _DBGMSG(fmt, args...)   \
         printk(KERN_INFO fmt "\r\n", ##args)
#else
#define _DBGMSG(fmt, args...)
#endif
static ssize_t _eeprom_read(
    struct i2c_client               *client,
    struct device_attribute     *devattr,
    char                        *buf
)
{
    int                 i, memaddr = 0;
    struct eeprom e;
    char c[TLV_EEPROM_SIZE];

    e.client = client;
    e.type = EEPROM_TYPE_16BIT_ADDR;
    _DBGMSG("_eeprom_read EEPROM_TYPE_16BIT_ADDR");

    for (i = 0; i < TLV_EEPROM_SIZE; i ++) {
        c[i] = eeprom_read_byte(&e, memaddr);
          //_DBGMSG("loop %d, byte[%d] = 0x%x", i, memaddr, c[i]);
          memaddr++;
    }
    memcpy(buf, c, TLV_EEPROM_SIZE);
    _DBGMSG("Exit");
    return TLV_EEPROM_SIZE;
}
static bool _utl_onie_check_header(TLV_HEADER_t *eeprom_header)
{
    u16  total_length = 0;
    /* Check the ID string is "TlvInfo" or not. */
    if (strcmp(eeprom_header->signature, UTL_ONIE_HEADER_ID_STRING) != 0)        return false;
    /* Check the header version. */
    if (eeprom_header->version != UTL_ONIE_HEADER_VERSION)        return false;
    /* Check the total length. */
    total_length = (eeprom_header->total_length[0] << 8) | eeprom_header->total_length[1];
    if (total_length > UTL_ONIE_TLV_TOTAL_LEN_MAX)        return false;
    return true;
}
/*------------------------------------------------------------------------------
* Function:    _utl_onie_decode_value
* Purpose:     The function decodes the ONIE EEPROM TLV value.
* Parameters:
*    Input:    tlv                 - ONIE EEPROM TLV.
*    Output:   value               - String value.
*    Returns:  None.
* Notes:       None.
*------------------------------------------------------------------------------ */
static ssize_t _utl_onie_decode_value( TLV_RECORD_T *tlv,char *value)
{
    int i=0, rc=0;
    /* Used for store string value. */
    switch (tlv->type)    {
        case UTL_ONIE_PRODUCT_NAME:
        case UTL_ONIE_PART_NUMBER:
        case UTL_ONIE_SERIAL_NUMBER:
        case UTL_ONIE_MANUF_DATE:
        case UTL_ONIE_LABEL_REVISION:
        case UTL_ONIE_PLATFORM_NAME:
        case UTL_ONIE_ONIE_VERSION:
        case UTL_ONIE_MANUF_NAME:
        case UTL_ONIE_MANUF_COUNTRY:
        case UTL_ONIE_VENDOR_NAME:
        case UTL_ONIE_DIAG_VERSION:
        case UTL_ONIE_SERVICE_TAG:
            value[0] = 0;
            for (i = 0; i < tlv->length; i++)
            {
                rc = sprintf(value, "%s%c", value, tlv->value[i]);
                _DBGMSG("tlv type %d, length %x, value 0x%x", tlv->type, tlv->length,  tlv->value[i] );
            }
            rc = sprintf(value, "%s\n", value);
            break;
        case UTL_ONIE_MAC_BASE:
            value[0] = 0;
            rc = sprintf(value, "%02X:%02X:%02X:%02X:%02X:%02X\n", tlv->value[0], tlv->value[1], tlv->value[2], tlv->value[3], tlv->value[4], tlv->value[5]);
            break;
        case UTL_ONIE_DEVICE_VERSION:
            value[0] = 0;
            rc = sprintf(value, "%u\n", tlv->value[0]);
            break;
        case UTL_ONIE_MAC_SIZE:
            value[0] = 0;
            rc = sprintf(value, "0x%02X%02X\n", tlv->value[0], tlv->value[1]);
            break;
        case UTL_ONIE_VENDOR_EXT:
            memcpy(value, tlv->value, tlv->length);
            value[tlv->length] = 0;
            break;
        case UTL_ONIE_CRC32:
            value[0] = 0;
            rc = sprintf(value, "0x%02X%02X%02X%02X", tlv->value[0], tlv->value[1], tlv->value[2], tlv->value[3]);
            break;
        default:
            memcpy(value, tlv->value, tlv->length);
            value[tlv->length] = 0;
            break;
    }
    return rc;
}
/*------------------------------------------------------------------------------
* Function:    UTL_ONIE_TLV_Find
* Purpose:     This function finds the specific ONIE TLV.
* Parameters:
*    Input:    eeprom              - EEPROM data.
*              type_code           - The type code.
*    Output:   eeprom_index        - EEPROM index.
*    Returns:  If successful, return TRUE, or return FALSE.
* Notes:       None.
*------------------------------------------------------------------------------ */
bool UTL_ONIE_TLV_Find(u8 *eeprom,u8  type_code, int *eeprom_index)
{
    TLV_HEADER_t               *eeprom_header = (TLV_HEADER_t *) eeprom;
    TLV_RECORD_T                  *eeprom_tlv;
    int                            eeprom_end;
    u16                          total_length = 0;
    /* check the ONIE header info. */
    if (_utl_onie_check_header(eeprom_header) != true)
    {
        _DBGMSG("ONIE header not found !");
        return false;
    }
    /* The EEPROM index starts from the end of ONIE header info. */
    *eeprom_index = TLV_HEADER_LEN;
    /* The EEPROM index terminates at the the length of ONIE header info plus total legnth. */
    total_length = (eeprom_header->total_length[0] << 8) | eeprom_header->total_length[1];
    eeprom_end = TLV_HEADER_LEN + total_length;
    /* Find the TLV. */
    while (*eeprom_index < eeprom_end)
    {
        eeprom_tlv = (TLV_RECORD_T *) &eeprom[*eeprom_index];
        if (eeprom_tlv->type == type_code)            return true;
     /* Find the next TLV type code. */
        *eeprom_index += TLV_RECORD_LEN + eeprom_tlv->length;
    }
    _DBGMSG("UTL_ONIE_TLV_Find wrong");
    return false;
}
/*------------------------------------------------------------------------------
* Function:    UTL_ONIE_TLV_Decode
* Purpose:     This function decodes the specific ONIE TLV.
* Parameters:
*    Input:    eeprom              - EEPROM data.
*              type_code           - The type code.
*    Output:   value               - The value.
*    Returns:
* Notes:       None.
*------------------------------------------------------------------------------ */
static ssize_t  UTL_ONIE_TLV_Decode(u8 *eeprom,u8  type_code, char  *value)
{
    TLV_RECORD_T          *eeprom_tlv;
    int                             eeprom_index=0, rc=1;
    /* Find the TLV and then decode it. */
    if (UTL_ONIE_TLV_Find(eeprom, type_code, &eeprom_index) == true)
    {
        eeprom_tlv = (TLV_RECORD_T *) &eeprom[eeprom_index];
        rc = _utl_onie_decode_value(eeprom_tlv, value);
    }
    return rc;
}
/* ----------------------------------------------------------------------------
 * Sysfs hooks function for read operation
 *
 * mgmt_mac
 * manu_date
 * hw_ver
 * product_name
 * serial_no
 * vendor_name
 * switch_mac
 *----------------------------------------------------------------------------
 */
static ssize_t get_mgmt_mac(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    ret = UTL_ONIE_TLV_Decode(TLV_data,UTL_ONIE_MAC_BASE, buf);
    if (ret < 0) return -EIO;
    if (dev == NULL) _DBGMSG("get_mgmt_mac");
    _DBGMSG("%d, %s",ret,  buf);
    return ret;
}
static DEVICE_ATTR(mgmt_mac, S_IRUGO, get_mgmt_mac, NULL);
static ssize_t get_manu_date(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    ret = UTL_ONIE_TLV_Decode(TLV_data,UTL_ONIE_MANUF_DATE, buf);
    if (ret < 0) return -EIO;
    if (dev == NULL) _DBGMSG("get_manu_date");
    _DBGMSG("%d, %s",ret,  buf);
    return ret;
}
static DEVICE_ATTR(manu_date, S_IRUGO, get_manu_date, NULL);
static ssize_t get_hw_ver(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    ret = UTL_ONIE_TLV_Decode(TLV_data,UTL_ONIE_DEVICE_VERSION, buf);
    if (ret < 0) return -EIO;
    if (dev == NULL) _DBGMSG("get_hw_ver");
    _DBGMSG("%d, %s",ret,  buf);
    return ret;
}
static DEVICE_ATTR(hw_ver, S_IRUGO, get_hw_ver, NULL);
static ssize_t get_product_name(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    ret = UTL_ONIE_TLV_Decode(TLV_data,UTL_ONIE_PRODUCT_NAME, buf);
    if (ret < 0) return -EIO;
    if (dev == NULL) _DBGMSG("get_product_name");
    _DBGMSG("%d, %s",ret,  buf);
    return ret;
}
static DEVICE_ATTR(product_name, S_IRUGO, get_product_name, NULL);
static ssize_t get_serial_no(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    ret = UTL_ONIE_TLV_Decode(TLV_data,UTL_ONIE_SERIAL_NUMBER, buf);
    if (ret < 0) return -EIO;
    if (dev == NULL) _DBGMSG("get_serial_no");
    _DBGMSG("%d, %s",ret,  buf);
    return ret;
}
static DEVICE_ATTR(serial_no, S_IRUGO, get_serial_no, NULL);
static ssize_t get_vendor_name(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    ret = UTL_ONIE_TLV_Decode(TLV_data,UTL_ONIE_VENDOR_NAME, buf);
    if (ret < 0) return -EIO;
    if (dev == NULL) _DBGMSG("get_vendor_name");
    _DBGMSG("%d, %s",ret,  buf);
    return ret;
}
static DEVICE_ATTR(vendor_name, S_IRUGO, get_vendor_name, NULL);
static ssize_t _i2c_eeprom_get(
    struct device               *dev,
    struct device_attribute     *devattr,
    char                        *buf
)
{
    struct i2c_client   *client = to_i2c_client(dev);
    _DBGMSG("Enter, i2c_addr = 0x%x", client->addr);
    memcpy(buf, TLV_data, TLV_EEPROM_SIZE);
    _DBGMSG("Exit");
    return TLV_EEPROM_SIZE;
}
static DEVICE_ATTR(cpu_eeprom, S_IRUGO, _i2c_eeprom_get, NULL);
/* ------------------------------------------------------------------------------
 * Register sysfs/i2c
 * ----------------------------------------------------------------------------
 */
static struct attribute *_i2c_eeprom_attr[] = {
    &dev_attr_cpu_eeprom.attr,
    &dev_attr_mgmt_mac.attr,
    &dev_attr_manu_date.attr,
    &dev_attr_hw_ver.attr,
    &dev_attr_product_name.attr,
    &dev_attr_serial_no.attr,
    &dev_attr_vendor_name.attr,
    NULL
};
static const struct attribute_group _i2c_eeprom_attr_grp = {
    .attrs = _i2c_eeprom_attr,
};

/* ----------------------------------------------------------------------------
 * Module probe/remove functions
 * ----------------------------------------------------------------------------
 */
/* Probe I2C driver */
static int _i2c_driver_probe(
    struct i2c_client           *client,
    const struct i2c_device_id  *dev_id
)
{
    int rc;
    _DBGMSG("Enter, i2c_addr = 0x%x", client->addr);
    rc = i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA);
    if (rc == 0) {
        _DBGMSG("Exit, failed on I2C address = 0x%x", client->addr);
        return -EIO;
    }
    dev_info(&client->dev, "chip found - New\n");
    /* Register sysfs hooks */
    rc = sysfs_create_group(&client->dev.kobj, &_i2c_eeprom_attr_grp);
    if (rc) {
        _DBGMSG("Exit, failed to create sysfs, rc = 0x%x", rc);
        return rc;
    }
    /* perform dummy read TLVinfo*/
    _eeprom_read(client, NULL, TLV_data);
    _DBGMSG("Exit");
    return 0;
}
/* Remove I2C driver */
static int _i2c_driver_remove(
    struct i2c_client   *client
)
{
    _DBGMSG("Enter");
    sysfs_remove_group(&client->dev.kobj, &_i2c_eeprom_attr_grp);
    _DBGMSG("Exit");
    return 0;
}
/* ----------------------------------------------------------------------------
 * Module main functions
 * ----------------------------------------------------------------------------
 */
static const struct i2c_device_id _i2c_driver_id[] = {
    {_DRIVER_NAME, 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, _i2c_driver_id);
static struct i2c_driver _i2c_device_driver = {
    .driver = {
        .name     = _DRIVER_NAME,
    },
    .probe        = _i2c_driver_probe,
    .remove       = _i2c_driver_remove,
    .id_table     = _i2c_driver_id,
};
module_i2c_driver(_i2c_device_driver);   // Simply module_init() & module_exit()
MODULE_AUTHOR("switchsupport@wistron.com");
MODULE_DESCRIPTION ("SONiC platform driver for EEPROM");
MODULE_LICENSE("GPL");