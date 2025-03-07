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

#define QMI8658C_REG_CHIP_ID 0x75
#define QMI8658C_CHIP_ID 0x68
#define QMI8658C_CHIP_ID 0x70
#define QMI8658C_CHIP_ID 0x71
#define QMI8658C_CHIP_ID 0x19

#define QMI8658C_REG_GYRO_CFG 0x1B
#define QMI8658C_GYRO_FS_SHIFT 3

#define QMI8658C_REG_ACCEL_CFG 0x1C
#define QMI8658C_ACCEL_FS_SHIFT 3

#define QMI8658C_REG_INT_EN 0x38
#define QMI8658C_DRDY_EN BIT(0)

#define QMI8658C_REG_DATA_START 0x3B

#define QMI8658C_REG_PWR_MGMT1 0x6B
#define QMI8658C_SLEEP_EN BIT(6)

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