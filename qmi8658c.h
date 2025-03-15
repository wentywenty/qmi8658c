/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_QMI8658C_QMI8658C_H_
#define ZEPHYR_DRIVERS_SENSOR_QMI8658C_QMI8658C_H_

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>

// #define QMI8658C_REG_CHIP_ID 0x75
// #define QMI8658C_CHIP_ID 0x68
// #define QMI8658C_CHIP_ID 0x70
// #define QMI8658C_CHIP_ID 0x71
// #define QMI8658C_CHIP_ID 0x19

// #define QMI8658C_REG_GYRO_CFG 0x1B
// #define QMI8658C_GYRO_FS_SHIFT 3

// #define QMI8658C_REG_ACCEL_CFG 0x1C
// #define QMI8658C_ACCEL_FS_SHIFT 3

// #define QMI8658C_REG_INT_EN 0x38
// #define QMI8658C_DRDY_EN BIT(0)

// #define QMI8658C_REG_DATA_START 0x3B

// #define QMI8658C_REG_PWR_MGMT1 0x6B
// #define QMI8658C_SLEEP_EN BIT(6)

/* 芯片ID和寄存器 */
#define QMI8658C_REG_WHO_AM_I      0x00
#define QMI8658C_CHIP_ID           0x05    /* 芯片ID值 */

/* 操作模式和配置寄存器 */
#define QMI8658C_REG_CTRL1         0x02    /* 操作模式控制 */
#define QMI8658C_REG_CTRL2         0x03    /* 加速度配置 */
#define QMI8658C_REG_CTRL3         0x04    /* 陀螺仪配置 */
#define QMI8658C_REG_CTRL7         0x08    /* 使能控制 */
#define QMI8658C_REG_CTRL8         0x09    /* 主要配置 */

/* 数据输出寄存器 */
#define QMI8658C_REG_ACC_X_L       0x35    /* 加速度X轴低字节 */
#define QMI8658C_REG_ACC_X_H       0x36
#define QMI8658C_REG_ACC_Y_L       0x37
#define QMI8658C_REG_ACC_Y_H       0x38
#define QMI8658C_REG_ACC_Z_L       0x39
#define QMI8658C_REG_ACC_Z_H       0x3A

#define QMI8658C_REG_TEMP_L        0x33    /* 温度数据低字节 */
#define QMI8658C_REG_TEMP_H        0x34

#define QMI8658C_REG_GYR_X_L       0x3B    /* 陀螺仪X轴低字节 */
#define QMI8658C_REG_GYR_X_H       0x3C
#define QMI8658C_REG_GYR_Y_L       0x3D
#define QMI8658C_REG_GYR_Y_H       0x3E
#define QMI8658C_REG_GYR_Z_L       0x3F
#define QMI8658C_REG_GYR_Z_H       0x40

/* 中断寄存器 */
#define QMI8658C_REG_INT_STATUS1   0x0A    /* 中断状态1 */
#define QMI8658C_REG_INT_STATUS2   0x0B    /* 中断状态2 */
#define QMI8658C_REG_INT_MASK      0x0D    /* 中断屏蔽 */
#define QMI8658C_REG_INT_CONFIG    0x0E    /* 中断配置 */
#define QMI8658C_REG_INT_MAP       0x19    /* 中断映射 */

/* 控制位定义 */
#define QMI8658C_CTRL7_ACC_ENABLE  BIT(0)  /* 加速度计使能 */
#define QMI8658C_CTRL7_GYR_ENABLE  BIT(1)  /* 陀螺仪使能 */
#define QMI8658C_CTRL7_MAG_ENABLE  BIT(2)  /* 磁力计使能 (如果有) */
#define QMI8658C_CTRL7_AE_ENABLE   BIT(6)  /* 算法引擎使能 */

/* 加速度计配置 */
#define QMI8658C_ACC_RANGE_2G      0       /* ±2g范围 */
#define QMI8658C_ACC_RANGE_4G      1       /* ±4g范围 */
#define QMI8658C_ACC_RANGE_8G      2       /* ±8g范围 */
#define QMI8658C_ACC_RANGE_16G     3       /* ±16g范围 */

/* 陀螺仪配置 */
#define QMI8658C_GYR_RANGE_16DPS   0       /* ±16°/秒范围 */
#define QMI8658C_GYR_RANGE_32DPS   1       /* ±32°/秒范围 */
#define QMI8658C_GYR_RANGE_64DPS   2       /* ±64°/秒范围 */
#define QMI8658C_GYR_RANGE_128DPS  3       /* ±128°/秒范围 */
#define QMI8658C_GYR_RANGE_256DPS  4       /* ±256°/秒范围 */
#define QMI8658C_GYR_RANGE_512DPS  5       /* ±512°/秒范围 */
#define QMI8658C_GYR_RANGE_1024DPS 6       /* ±1024°/秒范围 */
#define QMI8658C_GYR_RANGE_2048DPS 7       /* ±2048°/秒范围 */

/* 输出数据率 */
#define QMI8658C_ODR_1000HZ        0       /* 1000 Hz */
#define QMI8658C_ODR_500HZ         1       /* 500 Hz */
#define QMI8658C_ODR_250HZ         2       /* 250 Hz */
#define QMI8658C_ODR_125HZ         3       /* 125 Hz */
#define QMI8658C_ODR_62_5HZ        4       /* 62.5 Hz */
#define QMI8658C_ODR_31_25HZ       5       /* 31.25 Hz */
#define QMI8658C_ODR_15_625HZ      6       /* 15.625 Hz */
#define QMI8658C_ODR_7_8125HZ      7       /* 7.8125 Hz */

/* measured in degrees/sec x10 to avoid floating point */
static const uint16_t qmi8658c_gyro_sensitivity_x10[] = {1310, 655, 328, 164};

/* Device type, uses the correct offets for a particular device */
enum qmi8658c_device_type {
  DEVICE_TYPE_QMI8658C = 0,
  DEVICE_TYPE_QMI8658C,
};

struct qmi8658c_data {
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  uint16_t accel_sensitivity_shift;

  int16_t temp;

  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
  uint16_t gyro_sensitivity_x10;

  enum qmi8658c_device_type device_type;

#ifdef CONFIG_QMI8658C_TRIGGER
  const struct device *dev;
  struct gpio_callback gpio_cb;

  const struct sensor_trigger *data_ready_trigger;
  sensor_trigger_handler_t data_ready_handler;

#if defined(CONFIG_QMI8658C_TRIGGER_OWN_THREAD)
  K_KERNEL_STACK_MEMBER(thread_stack, CONFIG_QMI8658C_THREAD_STACK_SIZE);
  struct k_thread thread;
  struct k_sem gpio_sem;
#elif defined(CONFIG_QMI8658C_TRIGGER_GLOBAL_THREAD)
  struct k_work work;
#endif

#endif /* CONFIG_QMI8658C_TRIGGER */
};

struct qmi8658c_config {
  struct i2c_dt_spec i2c;
#ifdef CONFIG_QMI8658C_TRIGGER
  struct gpio_dt_spec int_gpio;
#endif /* CONFIG_QMI8658C_TRIGGER */
};

#ifdef CONFIG_QMI8658C_TRIGGER
int qmi8658c_trigger_set(const struct device *dev,
                         const struct sensor_trigger *trig,
                         sensor_trigger_handler_t handler);

int qmi8658c_init_interrupt(const struct device *dev);
#endif

#endif /* __SENSOR_QMI8658C__ */