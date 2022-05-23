#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/platform_data/pca954x.h>

#define MAX_I2C_DEVICE_NR   100

struct platform_i2c_board_info {
    int bus;
    int size;
    struct i2c_board_info *board_info;
};

static struct i2c_client *client[MAX_I2C_DEVICE_NR] = {NULL};
static int nr_i2c = 0;

static struct pca954x_platform_mode mux_modes_1[] = {
    {.adap_id = 2,},    {.adap_id = 3,},
    {.adap_id = 4,},    {.adap_id = 5,},
    {.adap_id = 6,},    {.adap_id = 7,},
    {.adap_id = 8,},    {.adap_id = 9,},
};

static struct pca954x_platform_mode mux_modes_9_0[] = {
    {.adap_id = 10,},    {.adap_id = 11,},
    {.adap_id = 12,},    {.adap_id = 13,},
    {.adap_id = 14,},    {.adap_id = 15,},
    {.adap_id = 16,},    {.adap_id = 17,},
};

static struct pca954x_platform_mode mux_modes_9_1[] = {
    {.adap_id = 18,},    {.adap_id = 19,},
    {.adap_id = 20,},    {.adap_id = 21,},
    {.adap_id = 22,},    {.adap_id = 23,},
    {.adap_id = 24,},    {.adap_id = 25,},
};

static struct pca954x_platform_mode mux_modes_9_2[] = {
    {.adap_id = 26,},    {.adap_id = 27,},
    {.adap_id = 28,},    {.adap_id = 29,},
    {.adap_id = 30,},    {.adap_id = 31,},
    {.adap_id = 32,},    {.adap_id = 33,},
};

static struct pca954x_platform_mode mux_modes_9_3[] = {
    {.adap_id = 34,},    {.adap_id = 35,},
    {.adap_id = 36,},    {.adap_id = 37,},
    {.adap_id = 38,},    {.adap_id = 39,},
    {.adap_id = 40,},    {.adap_id = 41,},
};

static struct pca954x_platform_data mux_data_1 = {
    .modes          = mux_modes_1,
    .num_modes      = 8,
};

static struct pca954x_platform_data mux_data_9_0 = {
    .modes          = mux_modes_9_0,
    .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_9_1 = {
    .modes          = mux_modes_9_1,
    .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_9_2 = {
    .modes          = mux_modes_9_2,
    .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_9_3 = {
    .modes          = mux_modes_9_3,
    .num_modes      = 8,
};


// layer 0
static struct i2c_board_info i2c_device_info1[] __initdata = {
    {"wistron_fpga",    0, 0x41, NULL,  0, 0, 0},
    {"pca9548",         0, 0x75, NULL,  &mux_data_1, 0, 0},
};

//layer 1
static struct i2c_board_info i2c_device_info3[] __initdata ={
    {"tmp75",           0, 0x49, NULL,  0, 0 ,0},
    {"tmp75",           0, 0x4A, NULL,  0, 0 ,0},
    {"tmp75",           0, 0x4B, NULL,  0, 0 ,0},
    {"tmp75",           0, 0x4F, NULL,  0, 0 ,0},
};

static struct i2c_board_info i2c_device_info4[] __initdata ={
    {"tps53688",         0, 0x62, NULL,  0, 0, 0},
    {"tps53688",         0, 0x64, NULL,  0, 0, 0},
};

static struct i2c_board_info i2c_device_info6[] __initdata ={
    {"ps2551",          0, 0x58, NULL,  0, 0, 0},
    {"ps2551",          0, 0x59, NULL,  0, 0, 0},
};

static struct i2c_board_info i2c_device_info9[] __initdata ={
    {"pca9548",         0, 0x70, NULL,  &mux_data_9_0, 0, 0},
    {"pca9548",         0, 0x71, NULL,  &mux_data_9_1, 0, 0},
    {"pca9548",         0, 0x72, NULL,  &mux_data_9_2, 0, 0},
    {"pca9548",         0, 0x73, NULL,  &mux_data_9_3, 0, 0},
    {"pca9539",         0, 0x76, NULL,  0, 0, 0},
    {"pca9539",         0, 0x77, NULL,  0, 0, 0},
    {"lm75",            0, 0x49, NULL,  0, 0, 0},
    {"max31790",        0, 0x2C, NULL,  0, 0 ,0},
    {"max31790",        0, 0x23, NULL,  0, 0 ,0},
};

//layer 2
static struct i2c_board_info i2c_device_info10[] __initdata ={
    {"wistron_cpld",        0, 0x53, NULL,  0, 0 ,0},
};

static struct i2c_board_info i2c_device_info26[] __initdata ={
    {"wistron_cpld",        0, 0x53, NULL,  0, 0 ,0},
};

static struct i2c_board_info i2c_device_info27[] __initdata ={
    {"optoe2",        0, 0x50, NULL,  0, 0 ,0},
};

// platform
static struct platform_i2c_board_info i2cdev_list[] = {
    { 1, ARRAY_SIZE(i2c_device_info1), i2c_device_info1},
    { 3, ARRAY_SIZE(i2c_device_info3), i2c_device_info3},
    { 4, ARRAY_SIZE(i2c_device_info4), i2c_device_info4},
    { 6, ARRAY_SIZE(i2c_device_info6), i2c_device_info6},
    { 9, ARRAY_SIZE(i2c_device_info9), i2c_device_info9},
    {10, ARRAY_SIZE(i2c_device_info10), i2c_device_info10},
    {26, ARRAY_SIZE(i2c_device_info26), i2c_device_info26},
};

static int __init wistron_platform_init(void)
{
    struct i2c_adapter *adap = NULL;
    struct i2c_client *e = NULL;
    int ret = 0;
    int i,j,k;
    printk(KERN_INFO "wistron_platform_init!\n");
    printk(KERN_INFO "i = %d\n", ARRAY_SIZE(i2cdev_list));
    for(i = 0; i < ARRAY_SIZE(i2cdev_list); i++) {
        adap = i2c_get_adapter(i2cdev_list[i].bus);
        if (adap == NULL) {
            ret = -ENODEV;
            goto exit;
        }
        printk(KERN_INFO "i = %d j = %d\n",i , i2cdev_list[i].size);
        for(j = 0; j < i2cdev_list[i].size; j++) {
            printk(KERN_INFO "j = %d\n",j);
            for(k = 0; k < 3; k++) {
                printk(KERN_INFO "k = %d\n",k);
                e = i2c_new_device(adap, &i2cdev_list[i].board_info[j]);
                if(!e){
                    printk(KERN_INFO "[%d]\n",__LINE__);
                    msleep(10);
                }else {
                    printk(KERN_INFO "[%d]\n",__LINE__);
                    client[nr_i2c++] = e;
                    break;
                }
            }
        }

        i2c_put_adapter(adap);
    }
    printk(KERN_INFO "[%d]\n",__LINE__);
    for(i = 10 ; i < 42 ; i++) {    //port adap id
        adap = i2c_get_adapter(i);
        for(k = 0; k < 3; k++) {
            e = i2c_new_device(adap, &i2c_device_info27[0]);
            if(!e)
                msleep(10);
            else {
                client[nr_i2c++] = e;
                break;
            }
        }
    }
    printk(KERN_INFO "[%d]\n",__LINE__);
    return ret;
exit:
    for(i = nr_i2c; i > 0; i--)
        i2c_unregister_device(client[i - 1]);

    nr_i2c = 0;
    return ret;
}

static void __exit wistron_platform_exit(void)
{
    int i = 0;
    printk(KERN_INFO "[%d] nr_i2c = %d\n",__LINE__, nr_i2c);
    for(i = nr_i2c; i > 0; i--){
        printk(KERN_INFO "[%d] i = %d\n",__LINE__, i);
        i2c_unregister_device(client[i - 1]);
    }

    nr_i2c = 0;
}

module_init(wistron_platform_init);
module_exit(wistron_platform_exit);

MODULE_AUTHOR("Wistron");
MODULE_DESCRIPTION("Wistron sunnyvale platform driver");
MODULE_LICENSE("GPL");
