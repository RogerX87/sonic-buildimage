
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _WISTRON_FPGA_H
#define _WISTRON_FPGA_H

#define FPGA_REVISION_REG           0x0
#define FPGA_HW_REV_REG             0x1
#define FPGA_MODEL_REV_REG          0x2
//#define FPGA_VENDOR_REG         0x3
//#define FPGA_TYPE_REG           0x4

#define FPGA_POWER_GOOD_REG         0x10
//#define FPGA_POWER_GOOD2_REG    0x11
//#define FPGA_POWER_GOOD3_REG    0x12
#define FPGA_POWER_CYCLE_REG        0x20

#define FPGA_SYSTEM_RESET1_REG      0x30
#define FPGA_SYSTEM_RESET2_REG      0x31

#define FPGA_INTERRUPT_REG          0x40
#define FPGA_INTERRUPT_MASK_REG     0x41

//#define FPGA_REBOOT_CAUSE1_REG  0x53
//#define FPGA_REBOOT_CAUSE2_REG  0x54

#define FPGA_PSU_CS_REG             0x60
#define FPGA_MISC_CS_REG            0x70

#define FPGA_SYS_LED1_REG           0x80
#define FPGA_SYS_LED2_REG           0x81

#define FPGA_WDT_REG                0x90
//#define FPGA_WDT_TIMER_REG      0x91

#define BIT_WDT_EN      0
#define BIT_WDT_KICK    1

enum psu_status_bit {
    PSU0_12V_ON = 0,
    PSU1_12V_ON,
    PSU0_PRESENT,
    PSU1_PRESENT,
    PSU0_ALERT,
    PSU1_ALERT,
};

enum led_status_shift_bit {
    LOC_LED = 0,
    DIAG_LED = 2,
    PSU0_LED = 4,
    PSU1_LED = 6,
};

enum fan_led_status_shift_bit {
    FAN_LED = 0,
};

static char* model_str[] = {
    "32x200G",
    "32x100G",
    "RSVD",
    "RSVD",
};

enum psu_pwrgood_bit {
    PSU0_PWRGD = 0,
    PSU1_PWRGD,
};
/*
static char* vendor_str[] = {
    "NA", // 0
    "Intel",
};

static char* type_str[] = {
    "NA", // 0
    "10M04SAU324I7G-GP",
};
*/
static char* power_good_str[] = {
    "PSU0_PWRGD", // 0
    "PSU1_PWRGD",
    "PWRGD_5V",
    "PWRGD_P3V3",
    "PWRGD_P0V84",
    "PWRGD_P0V8",
    "PWRGD_P1V8",   // 6
    "RSVD",

};
/*
static char* reboot_cause_str[] = {
    "MB_THERMALTRIP_N", // 0
    "Device_RST_Fail_INT_N",
    "Non_HW_System_Reboot_N",
    "CPU_Warm_Reset_N",
    "CPU_Cold_Reset_N",
    "CPU_BIOS_Reset_N",
    "TL7_THERMTRIP_N",
    "FAN_ALERT_DETECT_N", // 7
    "SYSTEM_PWR_LOSS_N",
    "CPU_THERMALTRIP_N",
    "SYSTEM_WD_N",
    "PSU_INT_N",
    "BMC_SHUTDOWN_N",
    "RST_BUTTON_COLD_BOOT_N",
    "SYS_VR_PWR_Fault_N",
    "RSVD", // 15
};
*/
int switch_fpga_read_byte_data(u8 fpga_id, u8 command);
int switch_fpga_write_byte_data(u8 fpga_id, u8 command, u8 value);

#endif /* _WISTRON_FPGA_H */
