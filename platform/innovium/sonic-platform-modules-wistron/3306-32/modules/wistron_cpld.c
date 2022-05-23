/*
 * A hwmon driver for the wistron_switch_cpld
 *
 */

/* ----------------------------------------------------------------------------
 * Include files
 * ----------------------------------------------------------------------------
 */
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dmi.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>

/* ---------------------------------------------------------------------------
 * Constant
 * ----------------------------------------------------------------------------
 */
#include "wistron_cpld.h"

#define CPLD_ADDRESS 0x53

static LIST_HEAD(cpld_client_list);
static struct mutex list_lock;

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};


enum qsfp_id {
    QSFP0 = 0,
    QSFP1,
    QSFP2,
    QSFP3,
    QSFP4,
    QSFP5,
    QSFP6,
    QSFP7,
    QSFP8,
    QSFP9,
    QSFP10,
    QSFP11,
    QSFP12,
    QSFP13,
    QSFP14,
    QSFP15,
};

/* ----------------------------------------------------------------------------
 *
 * Module attribute functions
 * ----------------------------------------------------------------------------
 */

static ssize_t cpld_show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, ver = 0, ver2 = 0;
    u8 command = CPLD_REVISION_REG;
    struct i2c_client *client = to_i2c_client(dev);

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & 0x80);
    ver = (val & 0x7) >> 4;
    ver2 = (val & 0xf);

    return sprintf(buf, "%s%d%d\n", (res == 0) ? "Proto" : "MP" ,ver, ver2);
}


static ssize_t port_modsel_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_MODSEL_REG_2 : CPLD_QSFP56_MODSEL_REG_1;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP7) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t port_modsel_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_MODSEL_REG_2 : CPLD_QSFP56_MODSEL_REG_1;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP7) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(1 << bit);

    if (res == 0)
        val |= (1 << bit);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t port_reset_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);
    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_RESET_REG_2 : CPLD_QSFP56_RESET_REG_1;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP7) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t port_reset_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_RESET_REG_2 : CPLD_QSFP56_RESET_REG_1;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP7) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(1 << bit);

    if (res == 0)
        val |= (1 << bit);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t port_lpmode_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);
    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_LPMODE_REG_2 : CPLD_QSFP56_LPMODE_REG_1;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP7) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    return sprintf(buf, "%d\n", res);
}

static ssize_t port_lpmode_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_LPMODE_REG_2 : CPLD_QSFP56_LPMODE_REG_1;
    
    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP7) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(1 << bit);

    if (res)
        val |= (1 << bit);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t port_present_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_PRESENT_REG_2 : CPLD_QSFP56_PRESENT_REG_1;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP8) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;
    res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t port_interrupt_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = (sda->index > QSFP7) ? CPLD_QSFP56_INTR_REG_2 : CPLD_QSFP56_INTR_REG_1;

    bit = (sda->index > QSFP7) ? (sda->index - QSFP8) % 8 : (sda->index) % 8;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t port_led_enable_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0;
    u8 command = CPLD_QSFF56_PLED_EN_REG;
    struct i2c_client *client = to_i2c_client(dev);

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = val & 0x1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t port_led_enable_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0;
    struct i2c_client *client = to_i2c_client(dev);

    u8 command = CPLD_QSFF56_PLED_EN_REG;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(0x1);

    if(res)
        val |= (res);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_0_led_act_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = (sda->index) * 2;
    bit = 6;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    //res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_1_led_act_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = (sda->index) * 2;
    bit = 7;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    //res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_2_led_act_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = ((sda->index) * 2) + 1;
    bit = 6;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    //res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_3_led_act_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = ((sda->index) * 2) + 1;
    bit = 7;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (val & (1 << bit)) >> bit;

    //res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_0_led_act_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = (sda->index) * 2;
    bit = 6;

    val = kstrtoint(buf, 10, &res);

    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(0x1 << bit);

    val |= (res << bit);

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_1_led_act_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = (sda->index) * 2;
    bit = 7;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(0x1 << bit);

    val |= (res << bit);

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_2_led_act_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = ((sda->index) * 2) + 1;
    bit = 6;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(0x1 << bit);

    val |= (res << bit);

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_3_led_act_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = ((sda->index) * 2) + 1;
    bit = 7;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(0x1 << bit);

    val |= (res << bit);

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_0_led_rgb_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = (sda->index) * 2;
    bit = 0;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (~((val & mask) >> bit)) & 0x7;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_1_led_rgb_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = (sda->index) * 2;
    bit = 3;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (~((val & mask) >> bit)) & 0x7;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_2_led_rgb_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = ((sda->index) * 2) + 1;
    bit = 0;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (~((val & mask) >> bit)) & 0x7;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_3_led_rgb_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    off = ((sda->index) * 2) + 1;
    bit = 3;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    mutex_unlock(&list_lock);

    res = (~((val & mask) >> bit)) & 0x7;

    return sprintf(buf, "%d\n", res);
}

static ssize_t lane_0_led_rgb_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 7)
        return -EINVAL;

    off = (sda->index) * 2;
    bit = 0;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(mask);

    val |= (~res << bit) & mask;

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_1_led_rgb_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 7)
        return -EINVAL;

    off = (sda->index) * 2;
    bit = 3;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(mask);

    val |= (~res << bit) & mask;

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_2_led_rgb_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 7)
        return -EINVAL;

    off = ((sda->index) * 2) + 1;
    bit = 0;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(mask);

    val |= (~res << bit) & mask;

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static ssize_t lane_3_led_rgb_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val = 0, res = 0, off = 0, bit = 0, mask = 0;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    u8 command = CPLD_QSFF56_PLED_REG;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 7)
        return -EINVAL;

    off = ((sda->index) * 2) + 1;
    bit = 3;
    mask = (0x7) << bit;

    mutex_lock(&list_lock);

    val = i2c_smbus_read_byte_data(client, (command + off));
    if (val < 0) {
        mutex_unlock(&list_lock);
        return val;
    }

    val &= ~(mask);

    val |= (~res << bit) & mask;

    i2c_smbus_write_byte_data(client, (command + off), val);

    mutex_unlock(&list_lock);

    return count;
}

static SENSOR_DEVICE_ATTR(cpld_rev, S_IRUGO, cpld_show_version, NULL, 0);

static SENSOR_DEVICE_ATTR(qsfp0_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp1_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp2_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp3_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp4_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp5_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp6_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp7_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp8_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp9_modsel,  S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp10_modsel, S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp11_modsel, S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp12_modsel, S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp13_modsel, S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp14_modsel, S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp15_modsel, S_IRUGO | S_IWUSR, port_modsel_read, port_modsel_write, QSFP15);

static SENSOR_DEVICE_ATTR(qsfp0_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp1_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp2_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp3_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp4_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp5_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp6_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp7_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp8_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp9_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp10_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp11_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp12_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp13_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp14_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp15_reset, S_IRUGO | S_IWUSR, port_reset_read, port_reset_write, QSFP15);

static SENSOR_DEVICE_ATTR(qsfp0_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp1_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp2_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp3_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp4_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp5_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp6_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp7_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp8_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp9_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp10_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp11_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp12_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp13_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp14_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp15_lpmode, S_IRUGO | S_IWUSR, port_lpmode_read, port_lpmode_write, QSFP15);

static SENSOR_DEVICE_ATTR(qsfp0_present, S_IRUGO, port_present_read, NULL, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp1_present, S_IRUGO, port_present_read, NULL, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp2_present, S_IRUGO, port_present_read, NULL, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp3_present, S_IRUGO, port_present_read, NULL, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp4_present, S_IRUGO, port_present_read, NULL, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp5_present, S_IRUGO, port_present_read, NULL, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp6_present, S_IRUGO, port_present_read, NULL, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp7_present, S_IRUGO, port_present_read, NULL, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp8_present, S_IRUGO, port_present_read, NULL, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp9_present, S_IRUGO, port_present_read, NULL, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp10_present, S_IRUGO, port_present_read, NULL, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp11_present, S_IRUGO, port_present_read, NULL, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp12_present, S_IRUGO, port_present_read, NULL, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp13_present, S_IRUGO, port_present_read, NULL, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp14_present, S_IRUGO, port_present_read, NULL, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp15_present, S_IRUGO, port_present_read, NULL, QSFP15);

static SENSOR_DEVICE_ATTR(qsfp0_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp1_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp2_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp3_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp4_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp5_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp6_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp7_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp8_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp9_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp10_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp11_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp12_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp13_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp14_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp15_interrupt, S_IRUGO, port_interrupt_read, NULL, QSFP15);

static SENSOR_DEVICE_ATTR(led_manual_enable, S_IRUGO | S_IWUSR, port_led_enable_read, port_led_enable_write, 0);

static SENSOR_DEVICE_ATTR(qsfp0_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp0_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp0_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp0_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp1_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp1_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp1_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp1_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp2_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp2_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp2_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp2_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp3_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp3_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp3_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp3_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp4_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp4_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp4_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp4_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp5_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp5_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp5_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp5_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp6_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp6_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp6_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp6_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp7_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp7_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp7_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp7_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp8_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp8_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp8_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp8_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp9_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp9_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp9_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp9_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp10_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp10_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp10_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp10_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp11_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp11_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp11_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp11_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp12_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp12_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp12_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp12_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp13_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp13_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp13_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp13_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp14_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp14_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp14_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp14_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp15_lane0_led_act, S_IRUGO | S_IWUSR, lane_0_led_act_read, lane_0_led_act_write, QSFP15);
static SENSOR_DEVICE_ATTR(qsfp15_lane1_led_act, S_IRUGO | S_IWUSR, lane_1_led_act_read, lane_1_led_act_write, QSFP15);
static SENSOR_DEVICE_ATTR(qsfp15_lane2_led_act, S_IRUGO | S_IWUSR, lane_2_led_act_read, lane_2_led_act_write, QSFP15);
static SENSOR_DEVICE_ATTR(qsfp15_lane3_led_act, S_IRUGO | S_IWUSR, lane_3_led_act_read, lane_3_led_act_write, QSFP15);

static SENSOR_DEVICE_ATTR(qsfp0_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp0_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp0_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp0_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP0);
static SENSOR_DEVICE_ATTR(qsfp1_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp1_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp1_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp1_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP1);
static SENSOR_DEVICE_ATTR(qsfp2_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp2_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp2_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp2_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP2);
static SENSOR_DEVICE_ATTR(qsfp3_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp3_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp3_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp3_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP3);
static SENSOR_DEVICE_ATTR(qsfp4_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp4_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp4_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp4_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP4);
static SENSOR_DEVICE_ATTR(qsfp5_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp5_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp5_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp5_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP5);
static SENSOR_DEVICE_ATTR(qsfp6_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp6_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp6_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp6_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP6);
static SENSOR_DEVICE_ATTR(qsfp7_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp7_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp7_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp7_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP7);
static SENSOR_DEVICE_ATTR(qsfp8_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp8_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp8_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp8_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP8);
static SENSOR_DEVICE_ATTR(qsfp9_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp9_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp9_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp9_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP9);
static SENSOR_DEVICE_ATTR(qsfp10_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp10_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp10_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp10_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP10);
static SENSOR_DEVICE_ATTR(qsfp11_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp11_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp11_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp11_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP11);
static SENSOR_DEVICE_ATTR(qsfp12_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp12_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp12_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp12_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP12);
static SENSOR_DEVICE_ATTR(qsfp13_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp13_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp13_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp13_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP13);
static SENSOR_DEVICE_ATTR(qsfp14_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp14_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp14_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp14_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP14);
static SENSOR_DEVICE_ATTR(qsfp15_lane0_led_rgb, S_IRUGO | S_IWUSR, lane_0_led_rgb_read, lane_0_led_rgb_write, QSFP15);
static SENSOR_DEVICE_ATTR(qsfp15_lane1_led_rgb, S_IRUGO | S_IWUSR, lane_1_led_rgb_read, lane_1_led_rgb_write, QSFP15);
static SENSOR_DEVICE_ATTR(qsfp15_lane2_led_rgb, S_IRUGO | S_IWUSR, lane_2_led_rgb_read, lane_2_led_rgb_write, QSFP15);
static SENSOR_DEVICE_ATTR(qsfp15_lane3_led_rgb, S_IRUGO | S_IWUSR, lane_3_led_rgb_read, lane_3_led_rgb_write, QSFP15);

static struct attribute *wistron_switch_cpld_attributes[] = {
    &sensor_dev_attr_cpld_rev.dev_attr.attr,

    &sensor_dev_attr_qsfp0_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp1_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp2_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp3_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp4_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp5_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp6_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp7_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp8_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp9_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp10_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp11_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp12_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp13_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp14_modsel.dev_attr.attr,
    &sensor_dev_attr_qsfp15_modsel.dev_attr.attr,

    &sensor_dev_attr_qsfp0_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp1_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp2_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp3_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp4_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp5_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp6_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp7_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp8_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp9_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp10_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp11_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp12_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp13_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp14_reset.dev_attr.attr,
    &sensor_dev_attr_qsfp15_reset.dev_attr.attr,

    &sensor_dev_attr_qsfp0_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lpmode.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lpmode.dev_attr.attr,

    &sensor_dev_attr_qsfp0_present.dev_attr.attr,
    &sensor_dev_attr_qsfp1_present.dev_attr.attr,
    &sensor_dev_attr_qsfp2_present.dev_attr.attr,
    &sensor_dev_attr_qsfp3_present.dev_attr.attr,
    &sensor_dev_attr_qsfp4_present.dev_attr.attr,
    &sensor_dev_attr_qsfp5_present.dev_attr.attr,
    &sensor_dev_attr_qsfp6_present.dev_attr.attr,
    &sensor_dev_attr_qsfp7_present.dev_attr.attr,
    &sensor_dev_attr_qsfp8_present.dev_attr.attr,
    &sensor_dev_attr_qsfp9_present.dev_attr.attr,
    &sensor_dev_attr_qsfp10_present.dev_attr.attr,
    &sensor_dev_attr_qsfp11_present.dev_attr.attr,
    &sensor_dev_attr_qsfp12_present.dev_attr.attr,
    &sensor_dev_attr_qsfp13_present.dev_attr.attr,
    &sensor_dev_attr_qsfp14_present.dev_attr.attr,
    &sensor_dev_attr_qsfp15_present.dev_attr.attr,

    &sensor_dev_attr_qsfp0_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp1_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp2_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp3_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp4_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp5_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp6_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp7_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp8_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp9_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp10_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp11_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp12_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp13_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp14_interrupt.dev_attr.attr,
    &sensor_dev_attr_qsfp15_interrupt.dev_attr.attr,

    &sensor_dev_attr_led_manual_enable.dev_attr.attr,

    &sensor_dev_attr_qsfp0_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp0_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp0_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp0_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane3_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane0_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane1_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane2_led_act.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane3_led_act.dev_attr.attr,

    &sensor_dev_attr_qsfp0_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp0_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp0_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp0_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp1_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp2_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp3_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp4_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp5_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp6_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp7_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp8_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp9_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp10_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp11_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp12_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp13_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp14_lane3_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane0_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane1_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane2_led_rgb.dev_attr.attr,
    &sensor_dev_attr_qsfp15_lane3_led_rgb.dev_attr.attr,

    NULL
};

static const struct attribute_group wistron_switch_cpld_group = {
    .attrs = wistron_switch_cpld_attributes,
};


/* ----------------------------------------------------------------------------
 * Module probe/remove functions
 * ----------------------------------------------------------------------------
 */
static int _add_client(struct i2c_client *client)
{
    struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);
    if (!node) {
        dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
        return -ENOMEM;
    }

    node->client = client;

    mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
    mutex_unlock(&list_lock);

    return 0;
}

static int _remove_client(struct i2c_client *client)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int found = 0;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(cpld_node);
    }

    mutex_unlock(&list_lock);

    return 0;
}

/* Probe I2C driver */
static int wistron_i2c_cpld_probe(struct i2c_client *client,
                                 const struct i2c_device_id *dev_id)
{
    int status;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
        status = -EIO;
        goto exit;
    }

    status = _add_client(client);
    if (status < 0) {
        dev_err(&client->dev, "Failed to add client\n");
        goto exit;
    }

    status = sysfs_create_group(&client->dev.kobj, &wistron_switch_cpld_group);

    if (status) {
        goto exit;
    }

    dev_info(&client->dev, "chip found\n");

    return 0;

exit:
    return status;
}

static int wistron_i2c_cpld_remove(struct i2c_client *client)
{
    _remove_client(client);

    sysfs_remove_group(&client->dev.kobj, &wistron_switch_cpld_group);

    return 0;
}

/* ----------------------------------------------------------------------------
 * Module main functions
 * ----------------------------------------------------------------------------
 */
static const struct i2c_device_id wistron_i2c_cpld_id[] = {
    {"wistron_cpld",  0},
    {}
};

MODULE_DEVICE_TABLE(i2c, wistron_i2c_cpld_id);

static struct i2c_driver wistron_i2c_cpld_driver = {
    .class      = I2C_CLASS_HWMON,
    .driver = {
        .name = "wistron_cpld",
    },
    .probe      = wistron_i2c_cpld_probe,
    .remove     = wistron_i2c_cpld_remove,
    .id_table   = wistron_i2c_cpld_id,
};

static int __init wistron_i2c_cpld_init(void)
{
    mutex_init(&list_lock);

    return i2c_add_driver(&wistron_i2c_cpld_driver);
}

static void __exit wistron_i2c_cpld_exit(void)
{
    i2c_del_driver(&wistron_i2c_cpld_driver);
}

int switch_cpld_read_byte_data(u8 cpld_id, u8 command)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV;
    unsigned short cpld_addr = CPLD_ADDRESS;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, command);
            if (ret < 0) {
                printk(KERN_ERR "Failed to read CPLD %d\n", cpld_id);
            }
        }
    }

    mutex_unlock(&list_lock);
    return ret;
}
EXPORT_SYMBOL(switch_cpld_read_byte_data);

int switch_cpld_write_byte_data(u8 cpld_id, u8 command, u8 value)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV;
    unsigned short cpld_addr = CPLD_ADDRESS;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_write_byte_data(cpld_node->client, command, value);
            if (ret < 0) {
                printk(KERN_ERR "Failed to write CPLD %d\n", cpld_id);
            }
        }
    }

    mutex_unlock(&list_lock);
    return ret;
}
EXPORT_SYMBOL(switch_cpld_write_byte_data);

int switch_cpld_read_port_present(u8 port)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV, res;
    unsigned short cpld_addr = CPLD_ADDRESS;
    u8 command, bit;

    command = (port > QSFP7) ? CPLD_QSFP56_RESET_REG_2 : CPLD_QSFP56_RESET_REG_1;

    bit = (port > QSFP7) ? (port - QSFP7) % 8 : (port) % 8;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, command);
            if (ret < 0) {
                printk(KERN_ERR "Failed to read CPLD at 0x%x\n", cpld_addr);
            }
        }
    }

    mutex_unlock(&list_lock);

    res = (ret & (1 << bit)) >> bit;

    res = (res) ? 0 : 1;

    return ret;
}
EXPORT_SYMBOL(switch_cpld_read_port_present);

int switch_cpld_read_port_interrupt(u8 port)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV, res;
    unsigned short cpld_addr = CPLD_ADDRESS;
    u8 command, bit;

    command = (port > QSFP7) ? CPLD_QSFP56_INTR_REG_2 : CPLD_QSFP56_INTR_REG_1;

    bit = (port > QSFP7) ? (port - QSFP7) % 8 : (port) % 8;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, command);
            if (ret < 0) {
                printk(KERN_ERR "Failed to read CPLD at 0x%x\n", cpld_addr);
            }
        }
    }

    mutex_unlock(&list_lock);

    res = (ret & (1 << bit)) >> bit;

    res = (res) ? 0 : 1;

    return ret;
}
EXPORT_SYMBOL(switch_cpld_read_port_interrupt);

int switch_cpld_read_port_lpmode(u8 port)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV, res;
    unsigned short cpld_addr = CPLD_ADDRESS;
    u8 command, bit;

    command = (port > QSFP7) ? CPLD_QSFP56_LPMODE_REG_2 : CPLD_QSFP56_LPMODE_REG_1;

    bit = (port > QSFP7) ? (port - QSFP7) % 8 : (port) % 8;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, command);
            if (ret < 0) {
                printk(KERN_ERR "Failed to read CPLD at 0x%x\n", cpld_addr);
            }
        }
    }

    mutex_unlock(&list_lock);

    res = (ret & (1 << bit)) >> bit;

    return ret;
}
EXPORT_SYMBOL(switch_cpld_read_port_lpmode);

int switch_cpld_write_port_lpmode(u8 port, u8 val)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV;
    unsigned short cpld_addr = CPLD_ADDRESS;
    u8 command, bit;

    if(val < 0 || val > 1)
        return -EINVAL;

    command = (port > QSFP7) ? CPLD_QSFP56_LPMODE_REG_2 : CPLD_QSFP56_LPMODE_REG_1;
    bit = (port > QSFP7) ? (port - QSFP7) % 8 : (port) % 8;


    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, command);
            if (ret < 0) {
                printk(KERN_ERR "Failed to read CPLD at 0x%x\n", cpld_addr);
            }
        }
    }

    ret &= ~(1 << bit);

    if (val)
        ret |= (1 << bit);

    i2c_smbus_write_byte_data(cpld_node->client, command, ret);

    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(switch_cpld_write_port_lpmode);

int switch_cpld_read_port_reset(u8 port)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV, res;
    unsigned short cpld_addr = CPLD_ADDRESS;
    u8 command, bit;

    command = (port > QSFP7) ? CPLD_QSFP56_RESET_REG_2 : CPLD_QSFP56_RESET_REG_1;

    bit = (port > QSFP7) ? (port - QSFP7) % 8 : (port) % 8;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, command);
            if (ret < 0) {
                printk(KERN_ERR "Failed to read CPLD at 0x%x\n", cpld_addr);
            }
        }
    }

    mutex_unlock(&list_lock);

    res = (ret & (1 << bit)) >> bit;

    res = (res) ? 0 : 1;

    return ret;
}
EXPORT_SYMBOL(switch_cpld_read_port_reset);

int switch_cpld_write_port_reset(u8 port, u8 val)
{
    struct list_head *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -ENODEV;
    unsigned short cpld_addr = CPLD_ADDRESS;
    u8 command, bit;

    if(val < 0 || val > 1)
        return -EINVAL;

    command = (port > QSFP7) ? CPLD_QSFP56_RESET_REG_2 : CPLD_QSFP56_RESET_REG_1;

    bit = (port > QSFP7) ? (port - QSFP7) % 8 : (port) % 8;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);
        if (cpld_node->client->addr == cpld_addr) {
            ret = i2c_smbus_read_byte_data(cpld_node->client, command);
            if (ret < 0) {
                printk(KERN_ERR "Failed to read CPLD at 0x%x\n", cpld_addr);
            }
        }
    }

    ret &= ~(1 << bit);

    if (val == 0)
        ret |= (1 << bit);

    i2c_smbus_write_byte_data(cpld_node->client, command, ret);

    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(switch_cpld_write_port_reset);

MODULE_AUTHOR("Wistron");
MODULE_DESCRIPTION("SONiC platform driver for Wistron CPLD");
MODULE_LICENSE("GPL");

module_init(wistron_i2c_cpld_init);
module_exit(wistron_i2c_cpld_exit);
