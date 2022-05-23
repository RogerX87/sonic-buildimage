/*
 * Wistron Generic PMBUS driver
 *
 *
 * Based on pmbus_core driver and ltc2978 driver
 *
 * Author:
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/pmbus.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/dmi.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include "pmbus.h"
#include "wistron_psu.h"

enum alarm {
    PSU_ALARM_NO_POWER        = 0x1,
    PSU_ALARM_TERMAL_ERROR    = 0x2,
    PSU_ALARM_FAN_ERROR       = 0x4,
    PSU_ALARM_VOL_ERROR       = 0x8,
};

enum id_name {
    ID_NAME_PS2551,
	ID_NAME_FSH082,
};

enum psu_address {
    PSU_ADDRESS_0        = 0x58,
    PSU_ADDRESS_1        = 0x59,
};

static LIST_HEAD(psu_client_list);
static struct mutex	list_lock;

struct psu_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

static int _map_to_pmbus_reg(u8 reg)
{
    int retval;

    switch(reg) {
        case WISTRON_PMBUS_REG_READ_VIN:
            retval = PMBUS_READ_VIN;
            break;
        case WISTRON_PMBUS_REG_READ_IIN:
            retval = PMBUS_READ_IIN;
            break;
        case WISTRON_PMBUS_REG_READ_VOUT:
            retval = PMBUS_READ_VOUT;
            break;
        case WISTRON_PMBUS_REG_READ_IOUT:
            retval = PMBUS_READ_IOUT;
            break;
        case WISTRON_PMBUS_REG_READ_TEMPERATURE_1:
            retval = PMBUS_READ_TEMPERATURE_1;
            break;
        case WISTRON_PMBUS_REG_READ_FAN_SPEED_1:
            retval = PMBUS_READ_FAN_SPEED_1;
            break;
        case WISTRON_PMBUS_REG_READ_POUT:
            retval = PMBUS_READ_POUT;
            break;
        case WISTRON_PMBUS_REG_READ_PIN:
            retval = PMBUS_READ_PIN;
            break;
        case WISTRON_PMBUS_REG_READ_MAX_V:
            retval = PMBUS_VIN_OV_FAULT_LIMIT;
            break;
        case WISTRON_PMBUS_REG_READ_MAX_I:
            retval = PMBUS_IIN_OC_FAULT_LIMIT;
            break;
        case WISTRON_PMBUS_REG_READ_MAX_P:
            retval = PMBUS_POUT_OP_FAULT_LIMIT;
            break;
        case WISTRON_PMBUS_REG_READ_TEMPERATURE_2:
            retval = PMBUS_READ_TEMPERATURE_2;
            break;
        case WISTRON_PMBUS_REG_READ_TEMPERATURE_3:
            retval = PMBUS_READ_TEMPERATURE_3;
            break;
        default:
            retval = -EINVAL;
            break;
    }

    return retval;
}

static int _add_client(struct i2c_client *client)
{
    struct psu_client_node *node = kzalloc(sizeof(struct psu_client_node), GFP_KERNEL);
    if (!node) {
        dev_dbg(&client->dev, "Can't allocate psu_client_node (0x%x)\n", client->addr);
        return -ENOMEM;
    }

    node->client = client;

    mutex_lock(&list_lock);
    list_add(&node->list, &psu_client_list);
    mutex_unlock(&list_lock);

    return 0;
}

static int _remove_client(struct i2c_client *client)
{
    struct list_head *list_node = NULL;
    struct psu_client_node *psu_node = NULL;
    int found = 0;

    mutex_lock(&list_lock);

    list_for_each(list_node, &psu_client_list)
    {
        psu_node = list_entry(list_node, struct psu_client_node, list);

        if (psu_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(psu_node);
    }

    mutex_unlock(&list_lock);

    return 0;
}

static long _pmbus_reg2data_linear(int data, u8 reg)
{
    s16 exponent;
    s32 mantissa;
    long val;

    if (reg == PMBUS_READ_VOUT) { /* LINEAR16 */
        exponent =-9;
        mantissa = (u16)data;
    } else {                /* LINEAR11 */
        exponent = ((s16)data) >> 11;
        mantissa = ((s16)((data & 0x7ff) << 5)) >> 5;
    }

    val = mantissa;

    /* scale result to milli-units for all sensors except fans */
    if (reg != PMBUS_READ_FAN_SPEED_1)
        val = val * 1000L;

    /* scale result to micro-units for power sensors */
    if ((reg == PMBUS_READ_PIN) || (reg == PMBUS_READ_POUT))
        val = val * 1000L;

    if (exponent >= 0)
        val <<= exponent;
    else
        val >>= -exponent;

    return val;
}

static int pmbus_read_status_data(struct i2c_client *client, int page, int reg)
{
    int ret = 0;
    int mfg_status;

    if (page > 0)
        return -ENXIO;

    switch (reg) {
        case PMBUS_STATUS_VOUT:
            mfg_status = i2c_smbus_read_word_data(client, PMBUS_STATUS_VOUT);
            if (mfg_status < 0)
                return mfg_status;
            if (mfg_status & PB_VOLTAGE_UV_FAULT)
                ret |= PB_VOLTAGE_UV_FAULT;
            if (mfg_status & PB_VOLTAGE_OV_FAULT)
                ret |= PB_VOLTAGE_OV_FAULT;
            break;
        case PMBUS_STATUS_IOUT:
            mfg_status = i2c_smbus_read_word_data(client, PMBUS_STATUS_IOUT);
            if (mfg_status < 0)
                return mfg_status;
            if (mfg_status & PB_IOUT_OC_FAULT)
                ret |= PB_IOUT_OC_FAULT;
            if (mfg_status & PB_IOUT_OC_WARNING)
                ret |= PB_IOUT_OC_WARNING;
            if (mfg_status & PB_POUT_OP_FAULT)
                ret |= PB_POUT_OP_FAULT;
            if (mfg_status & PB_POUT_OP_WARNING)
                ret |= PB_POUT_OP_WARNING;
            break;
        case PMBUS_STATUS_INPUT:
            mfg_status = i2c_smbus_read_word_data(client, PMBUS_STATUS_INPUT);
            if (mfg_status < 0)
                return mfg_status;
            if (mfg_status & PB_PIN_OP_WARNING)
                ret |= PB_PIN_OP_WARNING;
            if (mfg_status & PB_IIN_OC_WARNING)
                ret |= PB_IIN_OC_WARNING;
            if (mfg_status & BIT(3))    // low input voltage
                ret |= BIT(3);
            if (mfg_status & PB_VOLTAGE_UV_FAULT)
                ret |= PB_VOLTAGE_UV_FAULT;
            if (mfg_status & PB_VOLTAGE_UV_WARNING)
                ret |= PB_VOLTAGE_UV_WARNING;
            if (mfg_status & PB_VOLTAGE_OV_FAULT)
                ret |= PB_VOLTAGE_OV_FAULT;
            break;
        case PMBUS_STATUS_TEMPERATURE:
            mfg_status = i2c_smbus_read_word_data(client, PMBUS_STATUS_TEMPERATURE);
            if (mfg_status < 0)
                return mfg_status;
            if (mfg_status & PB_TEMP_OT_FAULT)
                ret |= PB_TEMP_OT_FAULT;
            if (mfg_status & PB_TEMP_OT_WARNING)
                ret |= PB_TEMP_OT_WARNING;
            break;
        case PMBUS_STATUS_FAN_12:
            mfg_status = i2c_smbus_read_word_data(client, PMBUS_STATUS_FAN_12);
            if (mfg_status < 0)
                return mfg_status;
            if (mfg_status & PB_FAN_FAN1_FAULT)
                ret |= PB_FAN_FAN1_FAULT;
            if (mfg_status & PB_FAN_FAN1_WARNING)
                ret |= PB_FAN_FAN1_WARNING;
            if (mfg_status & PB_FAN_FAN1_SPEED_OVERRIDE)
                ret |= PB_FAN_FAN1_SPEED_OVERRIDE;
            break;
        default:
            ret = -ENODATA;
            break;
    }

    return ret;
}

static int pmbus_read_block_data(struct i2c_client *client, u8 command, u8 *data)
{
    int result = 0;
    int retry_count = 3;

    while (retry_count) {
        retry_count--;

        result = i2c_smbus_read_i2c_block_data(client, command, I2C_SMBUS_BLOCK_MAX, data);

        if (result < 0) {
            msleep(10);
            continue;
        }

        result = 0;
        break;
    }

    return result;
}

/* ----------------------------------------------------------------------------
 * export function for specified use
 * ----------------------------------------------------------------------------
 */

int wistron_psu_get_alarm(unsigned int psu_id, int *alarm)
{
    struct list_head *list_node = NULL;
    struct psu_client_node *psu_node = NULL;
    int ret;
    unsigned short psu_addr;

    if (psu_id == PSU_ID_0) {
        psu_addr = PSU_ADDRESS_0;
    } else if (psu_id == PSU_ID_1) {
        psu_addr = PSU_ADDRESS_1;
    } else {
        printk(KERN_ERR "No such device or address");
        return -ENODEV;
    }

    mutex_lock(&list_lock);

    list_for_each(list_node, &psu_client_list)
    {
        psu_node = list_entry(list_node, struct psu_client_node, list);

        if (psu_node->client->addr == psu_addr) {
            ret = pmbus_read_status_data(psu_node->client, 0, PMBUS_STATUS_TEMPERATURE);
            if (ret < 0) {
                goto error;
            } else if (ret > 0) {
                *alarm |= PSU_ALARM_TERMAL_ERROR;
            }

            ret = pmbus_read_status_data(psu_node->client, 0, PMBUS_STATUS_FAN_12);
            if (ret < 0) {
                goto error;
            } else if (ret > 0) {
                *alarm |= PSU_ALARM_FAN_ERROR;
            }

            ret = pmbus_read_status_data(psu_node->client, 0, PMBUS_STATUS_VOUT);
            if (ret < 0) {
                goto error;
            } else if (ret > 0) {
                *alarm |= PSU_ALARM_VOL_ERROR;
            }

            ret = pmbus_read_status_data(psu_node->client, 0, PMBUS_STATUS_IOUT);
            if (ret < 0) {
                goto error;
            } else if (ret > 0) {
                *alarm |= PSU_ALARM_VOL_ERROR;
            }

            ret = pmbus_read_status_data(psu_node->client, 0, PMBUS_STATUS_INPUT);
            if (ret < 0) {
                goto error;
            } else if (ret > 0) {
                *alarm |= PSU_ALARM_VOL_ERROR;
            }
        }
    }

    mutex_unlock(&list_lock);

    return 0;

error:
    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(wistron_psu_get_alarm);


static ssize_t wistron_psu_get_vendor(struct device *dev, struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = pmbus_read_block_data(client, PMBUS_MFR_ID, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer ID\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);

}

static ssize_t wistron_psu_get_model(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = pmbus_read_block_data(client, PMBUS_MFR_MODEL, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer Model\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static ssize_t wistron_psu_get_version(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = pmbus_read_block_data(client, PMBUS_MFR_REVISION, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer Revision\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static ssize_t wistron_psu_get_location(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = pmbus_read_block_data(client, PMBUS_MFR_LOCATION, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacture Location\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static ssize_t wistron_psu_get_serial(struct device *dev,
                struct device_attribute *da, char *buf)
{
    int ret, len;
    u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1], *str;
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);

    ret = pmbus_read_block_data(client, PMBUS_MFR_SERIAL, block_buffer);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Manufacturer Serial\n");
        return ret;
    }
    len = block_buffer[0];
    block_buffer[(len+1)] = '\0';
    str = &(block_buffer[1]);

    return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static DEVICE_ATTR(mfr_id, S_IRUGO, wistron_psu_get_vendor, NULL);
static DEVICE_ATTR(mfr_model, S_IRUGO, wistron_psu_get_model, NULL);
static DEVICE_ATTR(mfr_revision, S_IRUGO, wistron_psu_get_version, NULL);
static DEVICE_ATTR(mfr_location, S_IRUGO, wistron_psu_get_location, NULL);
static DEVICE_ATTR(mfr_serial, S_IRUGO, wistron_psu_get_serial, NULL);

static struct attribute *wistron_pmbus_attributes[] = {
    &dev_attr_mfr_id.attr,
    &dev_attr_mfr_model.attr,
    &dev_attr_mfr_revision.attr,
    &dev_attr_mfr_location.attr,
    &dev_attr_mfr_serial.attr,
    NULL
};

static const struct attribute_group wistron_pmbus_group = {
    .attrs = wistron_pmbus_attributes,
};

static const struct i2c_device_id pmbus_id[] = {
    {"ps2551", ID_NAME_PS2551},
    {"fsh082", ID_NAME_FSH082},
    {}
};
MODULE_DEVICE_TABLE(i2c, pmbus_id);

struct pmbus_driver_info ps2551_info = {
    .pages = 1,
    .func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_IIN |
        PMBUS_HAVE_PIN | PMBUS_HAVE_STATUS_INPUT |
        PMBUS_HAVE_FAN12 | PMBUS_HAVE_STATUS_FAN12 |
        PMBUS_HAVE_TEMP | PMBUS_HAVE_TEMP2 |
        PMBUS_HAVE_TEMP3 | PMBUS_HAVE_STATUS_TEMP |
        PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT |
        PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT |
        PMBUS_HAVE_POUT,
};

struct pmbus_driver_info fsh082_info = {
    .pages = 1,
    .func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_IIN |
        PMBUS_HAVE_PIN | PMBUS_HAVE_STATUS_INPUT |
        PMBUS_HAVE_FAN12 | PMBUS_HAVE_STATUS_FAN12 |
        PMBUS_HAVE_TEMP | PMBUS_HAVE_TEMP2 |
        PMBUS_HAVE_TEMP3 | PMBUS_HAVE_STATUS_TEMP |
        PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT |
        PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT |
        PMBUS_HAVE_POUT,
};

static int pmbus_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
    struct device *dev = &client->dev;
    int ret;

    dev_info(dev, "wistron-psu pmbus_probe\n");

    if (!i2c_check_functionality(client->adapter,
            I2C_FUNC_SMBUS_READ_WORD_DATA))
        return -ENODEV;

    ret = _add_client(client);
    if (ret < 0) {
        dev_err(dev, "Failed to add client\n");
        goto exit;
    }

    /* Register sysfs hooks */
    ret = sysfs_create_group(&client->dev.kobj, &wistron_pmbus_group);
    if (ret) {
        dev_err(dev, "Failed to create sysfs entries\n");
        goto exit;
    }

	if (id->driver_data == ID_NAME_PS2551)
		return pmbus_do_probe(client, id, &ps2551_info);
	if (id->driver_data == ID_NAME_FSH082)
		return pmbus_do_probe(client, id, &fsh082_info);

exit:
    return ret;
}

static int pmbus_remove(struct i2c_client *client)
{
    _remove_client(client);
    return pmbus_do_remove(client);
}

static struct i2c_driver pmbus_driver = {
    .driver = {
        .name = "wistron-psu",
    },
    .probe = pmbus_probe,
    .remove = pmbus_remove,
    .id_table = pmbus_id,
};

static int __init pmbus_driver_init(void)
{
    mutex_init(&list_lock);

    return i2c_add_driver(&pmbus_driver);
}

static void __exit pmbus_driver_exit(void)
{
    i2c_del_driver(&pmbus_driver);
}

module_init(pmbus_driver_init);
module_exit(pmbus_driver_exit);

MODULE_AUTHOR("Wistron");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("SONiC platform driver for Wistron PSU");
MODULE_LICENSE("GPL");
