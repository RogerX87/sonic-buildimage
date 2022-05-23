/*
 * A hwmon driver for the wistron_fpga
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
#include "wistron_fpga.h"

struct wistron_switch_fpga_data {
    struct mutex lock;
    struct i2c_client *client;
};

static struct wistron_switch_fpga_data switch_fpga_data;

/* Addresses scanned for wistron_switch_fpga */
static const unsigned short normal_i2c[] = {0x41, I2C_CLIENT_END};

/* ----------------------------------------------------------------------------
 *
 * Module attribute functions
 * ----------------------------------------------------------------------------
 */

static ssize_t fpga_show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, ten = 0, dig = 0;
    u8 command = FPGA_REVISION_REG;
    struct i2c_client *client = to_i2c_client(dev);

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = (val & 0x80) >> 7;
    ten = (val & 0x70) >> 4;
    dig = (val & 0xf);

    return sprintf(buf, "%s %d.%d\n", (res == 0) ? "Proto" : "MP" ,ten, dig);
}

static ssize_t fpga_show_hw_rev(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0;
    u8 command = FPGA_HW_REV_REG;
    struct i2c_client *client = to_i2c_client(dev);

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = (val & 0x3);

    return sprintf(buf, "%s %d\n", (val & 0x10) ? "MP" : "Proto", res);
}

static ssize_t fpga_show_model_rev(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0;
    u8 command = FPGA_MODEL_REV_REG;
    struct i2c_client *client = to_i2c_client(dev);

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = (val & 0x3);

    return sprintf(buf, "%s\n", model_str[res]);
}

static ssize_t fpga_show_power_good(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, i = 0;
    u8 command = FPGA_POWER_GOOD_REG;
    struct i2c_client *client = to_i2c_client(dev);

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    for (i = 0; i < 7; i++) {
        sprintf(buf + strlen(buf), "%s: %s\n", power_good_str[i], ((val & (1 << i)) >> i) ? "OK" : "Failed");
    }

    return (ssize_t)strlen(buf);;
}

static ssize_t fpga_read_power_good(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0;
    u8 command = FPGA_POWER_GOOD_REG;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    bit = sda->index;
    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = (val & (1 << bit)) >> bit;

    //res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t fpga_read_psu_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0;
    u8 command = FPGA_PSU_CS_REG;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    bit = sda->index;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = (val & (1 << bit)) >> bit;

    res = (res) ? 0 : 1;

    return sprintf(buf, "%d\n", res);
}

static ssize_t fpga_write_psu_status(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);
    u8 command = FPGA_PSU_CS_REG;
    int val = 0, res = 0, bit = 0;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    bit = sda->index;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    val &= ~(1 << bit);

    if (res == 0)
        val |= (1 << bit);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&switch_fpga_data.lock);

    return count;
}

static ssize_t fpga_led_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0, mask = 0;
    u8 command = FPGA_SYS_LED1_REG;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);

    bit = sda->index;
    mask = (0x3) << bit;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = (val & mask) >> bit;

    return sprintf(buf, "%d\n", res);
}

static ssize_t fpga_led_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);
    u8 command = FPGA_SYS_LED1_REG;
    int val = 0, res = 0, bit = 0, mask = 0;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 3)
        return -EINVAL;

    bit = sda->index;
    mask = (0x3) << bit;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    val &= ~(mask);

    val |= (res << bit);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&switch_fpga_data.lock);

    return count;
}

static ssize_t fpga_fan_led_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0, bit = 0, mask = 0;
    u8 command = FPGA_SYS_LED2_REG;
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);
    
    bit = sda->index;
    mask = (0x3) << bit;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = (val & mask) >> bit;

    return sprintf(buf, "%d\n", res);
}

static ssize_t fpga_fan_led_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);
    u8 command = FPGA_SYS_LED2_REG;
    int val = 0, res = 0, bit = 0, mask = 0;

    val = kstrtoint(buf, 10, &res);

    if (val)
        return val;

    if (res < 0 || res > 3)
        return -EINVAL;

    bit = sda->index;
    mask = (0x3) << bit;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    val &= ~(mask << bit);

    val |= (res << bit);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&switch_fpga_data.lock);

    return count;
}

static ssize_t fpga_wdt_en_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0, res = 0;
    u8 command = FPGA_WDT_REG;
    struct i2c_client *client = to_i2c_client(dev);

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    mutex_unlock(&switch_fpga_data.lock);

    res = val & (1 << BIT_WDT_EN);

    return sprintf(buf, "%d\n", res);
}

static ssize_t fpga_wdt_en_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    u8 command = FPGA_WDT_REG;
    int val = 0, res = 0;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    if (res < 0 || res > 1)
        return -EINVAL;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    val &= ~(1 << BIT_WDT_EN);

    val |= (res << BIT_WDT_EN);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&switch_fpga_data.lock);

    return count;
}

static ssize_t fpga_wdt_kick(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    u8 command = FPGA_WDT_REG;
    int val = 0, res = 0;

    val = kstrtoint(buf, 10, &res);
    if (val)
        return val;

    mutex_lock(&switch_fpga_data.lock);

    val = i2c_smbus_read_byte_data(client, command);
    if (val < 0) {
        mutex_unlock(&switch_fpga_data.lock);
        return val;
    }

    val &= ~(1 << BIT_WDT_KICK);

    val |= (1 << BIT_WDT_KICK);

    i2c_smbus_write_byte_data(client, command, val);

    mutex_unlock(&switch_fpga_data.lock);

    return count;
}

static SENSOR_DEVICE_ATTR(fpga_rev, S_IRUGO, fpga_show_version, NULL, 0);
static SENSOR_DEVICE_ATTR(hw_rev, S_IRUGO, fpga_show_hw_rev, NULL, 0);
static SENSOR_DEVICE_ATTR(model_rev, S_IRUGO, fpga_show_model_rev, NULL, 0);
static SENSOR_DEVICE_ATTR(power_good, S_IRUGO, fpga_show_power_good, NULL, 0);

static SENSOR_DEVICE_ATTR(psu0_power_good, S_IRUGO, fpga_read_power_good, NULL, PSU0_PWRGD);
static SENSOR_DEVICE_ATTR(psu1_power_good, S_IRUGO, fpga_read_power_good, NULL, PSU1_PWRGD);

static SENSOR_DEVICE_ATTR(psu1_alert, S_IRUGO, fpga_read_psu_status, NULL, PSU1_ALERT);
static SENSOR_DEVICE_ATTR(psu0_alert, S_IRUGO, fpga_read_psu_status, NULL, PSU0_ALERT);
static SENSOR_DEVICE_ATTR(psu1_present, S_IRUGO, fpga_read_psu_status, NULL, PSU1_PRESENT);
static SENSOR_DEVICE_ATTR(psu0_present, S_IRUGO, fpga_read_psu_status, NULL, PSU0_PRESENT);
static SENSOR_DEVICE_ATTR(psu1_12v_on, S_IRUGO | S_IWUSR, fpga_read_psu_status, fpga_write_psu_status, PSU1_12V_ON);
static SENSOR_DEVICE_ATTR(psu0_12v_on, S_IRUGO | S_IWUSR, fpga_read_psu_status, fpga_write_psu_status, PSU0_12V_ON);

static SENSOR_DEVICE_ATTR(fan_led, S_IRUGO | S_IWUSR, fpga_fan_led_read, fpga_fan_led_write, FAN_LED);

static SENSOR_DEVICE_ATTR(psu1_led, S_IRUGO | S_IWUSR, fpga_led_read, fpga_led_write, PSU1_LED);
static SENSOR_DEVICE_ATTR(psu0_led, S_IRUGO | S_IWUSR, fpga_led_read, fpga_led_write, PSU0_LED);
static SENSOR_DEVICE_ATTR(diag_led, S_IRUGO | S_IWUSR, fpga_led_read, fpga_led_write, DIAG_LED);
static SENSOR_DEVICE_ATTR(loc_led, S_IRUGO | S_IWUSR, fpga_led_read, fpga_led_write, LOC_LED);

static SENSOR_DEVICE_ATTR(watchdog_en, S_IRUGO | S_IWUSR, fpga_wdt_en_read, fpga_wdt_en_write, 0);
static SENSOR_DEVICE_ATTR(watchdog_kick, S_IWUSR, NULL, fpga_wdt_kick, 0);

static struct attribute *wistron_switch_fpga_attributes[] = {
    &sensor_dev_attr_fpga_rev.dev_attr.attr,
    &sensor_dev_attr_hw_rev.dev_attr.attr,
    &sensor_dev_attr_model_rev.dev_attr.attr,

    &sensor_dev_attr_power_good.dev_attr.attr,

    &sensor_dev_attr_psu0_power_good.dev_attr.attr,
    &sensor_dev_attr_psu1_power_good.dev_attr.attr,

    &sensor_dev_attr_psu0_alert.dev_attr.attr,
    &sensor_dev_attr_psu1_alert.dev_attr.attr,
    &sensor_dev_attr_psu0_present.dev_attr.attr,
    &sensor_dev_attr_psu1_present.dev_attr.attr,
    &sensor_dev_attr_psu0_12v_on.dev_attr.attr,
    &sensor_dev_attr_psu1_12v_on.dev_attr.attr,

    &sensor_dev_attr_fan_led.dev_attr.attr,

    &sensor_dev_attr_psu1_led.dev_attr.attr,
    &sensor_dev_attr_psu0_led.dev_attr.attr,
    &sensor_dev_attr_diag_led.dev_attr.attr,
    &sensor_dev_attr_loc_led.dev_attr.attr,

    &sensor_dev_attr_watchdog_en.dev_attr.attr,
    &sensor_dev_attr_watchdog_kick.dev_attr.attr,
    NULL
};

static const struct attribute_group wistron_switch_fpga_group = {
    .attrs = wistron_switch_fpga_attributes,
};

/* ----------------------------------------------------------------------------
 * Module probe/remove functions
 * ----------------------------------------------------------------------------
 */

/* Probe I2C driver */
static int wistron_i2c_fpga_probe(struct i2c_client *client,
                                 const struct i2c_device_id *dev_id)
{
    int status;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
        status = -EIO;
        goto exit;
    }

    status = sysfs_create_group(&client->dev.kobj, &wistron_switch_fpga_group);

    if (status) {
        goto exit;
    }

    dev_info(&client->dev, "chip found\n");

    mutex_lock(&switch_fpga_data.lock);
    switch_fpga_data.client = client;
    mutex_unlock(&switch_fpga_data.lock);

    return 0;

exit:
    return status;
}

static int wistron_i2c_fpga_remove(struct i2c_client *client)
{
    mutex_lock(&switch_fpga_data.lock);
    switch_fpga_data.client = NULL;
    mutex_unlock(&switch_fpga_data.lock);

    sysfs_remove_group(&client->dev.kobj, &wistron_switch_fpga_group);
    return 0;
}

/* ----------------------------------------------------------------------------
 * Module main functions
 * ----------------------------------------------------------------------------
 */
static const struct i2c_device_id wistron_i2c_fpga_id[] = {
    {"wistron_fpga",  0},
    {}
};
MODULE_DEVICE_TABLE(i2c, wistron_i2c_fpga_id);

static struct i2c_driver wistron_i2c_fpga_driver = {
    .class      = I2C_CLASS_HWMON,
    .driver = {
        .name = "wistron_fpga",
    },
    .probe      = wistron_i2c_fpga_probe,
    .remove     = wistron_i2c_fpga_remove,
    .id_table   = wistron_i2c_fpga_id,
    .address_list = normal_i2c,
};

int switch_fpga_read_byte_data(u8 fpga_id, u8 command)
{
    int ret;

    mutex_lock(&switch_fpga_data.lock);

    if (switch_fpga_data.client == NULL) {
        ret = -ENODEV;
        goto error;
    }

    ret = i2c_smbus_read_byte_data(switch_fpga_data.client, command);

    mutex_unlock(&switch_fpga_data.lock);
    return ret;

error:

    mutex_unlock(&switch_fpga_data.lock);
    return ret;
}
EXPORT_SYMBOL(switch_fpga_read_byte_data);

int switch_fpga_write_byte_data(u8 fpga_id, u8 command, u8 value)
{
    int ret;

    mutex_lock(&switch_fpga_data.lock);

    if (switch_fpga_data.client == NULL) {
        ret = -ENODEV;
        goto error;
    }

    ret = i2c_smbus_write_byte_data(switch_fpga_data.client, command, value);

    mutex_unlock(&switch_fpga_data.lock);
    return ret;

error:
    mutex_unlock(&switch_fpga_data.lock);
    return ret;
}
EXPORT_SYMBOL(switch_fpga_write_byte_data);

static int __init wistron_i2c_fpga_init(void)
{
    mutex_init(&switch_fpga_data.lock);

    return i2c_add_driver(&wistron_i2c_fpga_driver);
}

static void __exit wistron_i2c_fpga_exit(void)
{
    i2c_del_driver(&wistron_i2c_fpga_driver);
}

MODULE_AUTHOR("Wistron");
MODULE_DESCRIPTION("Wistron sunnyvale fpga driver");
MODULE_LICENSE("GPL");

module_init(wistron_i2c_fpga_init);
module_exit(wistron_i2c_fpga_exit);
