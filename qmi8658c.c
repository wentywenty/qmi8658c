/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT qst_qmi8658c

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include "qmi8658c.h"

LOG_MODULE_REGISTER(QMI8658C, CONFIG_SENSOR_LOG_LEVEL);

/* see "Accelerometer Measurements" section from register map description */
static void qmi8658c_convert_accel(struct sensor_value *val, int16_t raw_val,
                                   uint16_t sensitivity_shift) {
  int64_t conv_val;

  conv_val = ((int64_t)raw_val * SENSOR_G) >> sensitivity_shift;
  val->val1 = conv_val / 1000000;
  val->val2 = conv_val % 1000000;
}

/* see "Gyroscope Measurements" section from register map description */
static void qmi8658c_convert_gyro(struct sensor_value *val, int16_t raw_val,
                                  uint16_t sensitivity_x10) {
  int64_t conv_val;

  conv_val = ((int64_t)raw_val * SENSOR_PI * 10) / (sensitivity_x10 * 180U);
  val->val1 = conv_val / 1000000;
  val->val2 = conv_val % 1000000;
}

/* see "Temperature Measurement" section from register map description */
static inline void qmi8658c_convert_temp(enum qmi8658c_device_type device_type,
                                         struct sensor_value *val,
                                         int16_t raw_val) {
  int64_t tmp_val = (int64_t)raw_val * 1000000;

  switch (device_type) {
  case DEVICE_TYPE_QMI8658C:
    tmp_val = (tmp_val / 340) + 36000000;
    break;
  default:
    tmp_val = (tmp_val / 340) + 36000000;
  };

  val->val1 = tmp_val / 1000000;
  val->val2 = tmp_val % 1000000;
}

static int qmi8658c_channel_get(const struct device *dev,
                                enum sensor_channel chan,
                                struct sensor_value *val) {
  struct qmi8658c_data *drv_data = dev->data;

  switch (chan) {
  case SENSOR_CHAN_ACCEL_XYZ:
    qmi8658c_convert_accel(val, drv_data->accel_x,
                           drv_data->accel_sensitivity_shift);
    qmi8658c_convert_accel(val + 1, drv_data->accel_y,
                           drv_data->accel_sensitivity_shift);
    qmi8658c_convert_accel(val + 2, drv_data->accel_z,
                           drv_data->accel_sensitivity_shift);
    break;
  case SENSOR_CHAN_ACCEL_X:
    qmi8658c_convert_accel(val, drv_data->accel_x,
                           drv_data->accel_sensitivity_shift);
    break;
  case SENSOR_CHAN_ACCEL_Y:
    qmi8658c_convert_accel(val, drv_data->accel_y,
                           drv_data->accel_sensitivity_shift);
    break;
  case SENSOR_CHAN_ACCEL_Z:
    qmi8658c_convert_accel(val, drv_data->accel_z,
                           drv_data->accel_sensitivity_shift);
    break;
  case SENSOR_CHAN_GYRO_XYZ:
    qmi8658c_convert_gyro(val, drv_data->gyro_x,
                          drv_data->gyro_sensitivity_x10);
    qmi8658c_convert_gyro(val + 1, drv_data->gyro_y,
                          drv_data->gyro_sensitivity_x10);
    qmi8658c_convert_gyro(val + 2, drv_data->gyro_z,
                          drv_data->gyro_sensitivity_x10);
    break;
  case SENSOR_CHAN_GYRO_X:
    qmi8658c_convert_gyro(val, drv_data->gyro_x,
                          drv_data->gyro_sensitivity_x10);
    break;
  case SENSOR_CHAN_GYRO_Y:
    qmi8658c_convert_gyro(val, drv_data->gyro_y,
                          drv_data->gyro_sensitivity_x10);
    break;
  case SENSOR_CHAN_GYRO_Z:
    qmi8658c_convert_gyro(val, drv_data->gyro_z,
                          drv_data->gyro_sensitivity_x10);
    break;
  case SENSOR_CHAN_DIE_TEMP:
    qmi8658c_convert_temp(drv_data->device_type, val, drv_data->temp);
    break;
  default:
    return -ENOTSUP;
  }

  return 0;
}

static int qmi8658c_sample_fetch(const struct device *dev,
                                 enum sensor_channel chan) {
  struct qmi8658c_data *drv_data = dev->data;
  const struct qmi8658c_config *cfg = dev->config;
  uint8_t buf[12];

  if (chan != SENSOR_CHAN_ALL) {
    return -ENOTSUP;
  }

  /* 读取加速度计数据 - 6个字节 (X,Y,Z) */
  if (i2c_burst_read_dt(&cfg->i2c, QMI8658C_REG_ACC_X_L, buf, 6) < 0) {
    LOG_ERR("Failed to read accelerometer data");
    return -EIO;
  }

  drv_data->accel_x = (buf[1] << 8) | buf[0]; /* X轴 */
  drv_data->accel_y = (buf[3] << 8) | buf[2]; /* Y轴 */
  drv_data->accel_z = (buf[5] << 8) | buf[4]; /* Z轴 */

  /* 读取温度数据 - 2字节 */
  if (i2c_burst_read_dt(&cfg->i2c, QMI8658C_REG_TEMP_L, buf, 2) < 0) {
    LOG_ERR("Failed to read temperature data");
    return -EIO;
  }

  drv_data->temp = (buf[1] << 8) | buf[0];

  /* 读取陀螺仪数据 - 6字节 (X,Y,Z) */
  if (i2c_burst_read_dt(&cfg->i2c, QMI8658C_REG_GYR_X_L, buf, 6) < 0) {
    LOG_ERR("Failed to read gyroscope data");
    return -EIO;
  }

  drv_data->gyro_x = (buf[1] << 8) | buf[0]; /* X轴 */
  drv_data->gyro_y = (buf[3] << 8) | buf[2]; /* Y轴 */
  drv_data->gyro_z = (buf[5] << 8) | buf[4]; /* Z轴 */

  return 0;
}

static DEVICE_API(sensor, qmi8658c_driver_api) = {
#if CONFIG_QMI8658C_TRIGGER
    .trigger_set = qmi8658c_trigger_set,
#endif
    .sample_fetch = qmi8658c_sample_fetch,
    .channel_get = qmi8658c_channel_get,
};
int qmi8658c_init(const struct device *dev) {
  struct qmi8658c_data *drv_data = dev->data;
  const struct qmi8658c_config *cfg = dev->config;
  uint8_t id;

  /* 检查I2C总线是否就绪 */
  if (!device_is_ready(cfg->i2c.bus)) {
    LOG_ERR("Bus device not ready");
    return -ENODEV;
  }

  /* 检查芯片ID */
  if (i2c_reg_read_byte_dt(&cfg->i2c, QMI8658C_REG_WHO_AM_I, &id) < 0) {
    LOG_ERR("Failed to read chip ID");
    return -EIO;
  }

  if (id != QMI8658C_CHIP_ID) {
    LOG_ERR("Invalid chip ID: 0x%02x", id);
    return -EINVAL;
  }

  LOG_DBG("Device ID: 0x%02x", id);

  /* 配置传感器 - 使能加速度计和陀螺仪 */
  if (i2c_reg_write_byte_dt(&cfg->i2c, QMI8658C_REG_CTRL7,
                            QMI8658C_CTRL7_ACC_ENABLE |
                                QMI8658C_CTRL7_GYR_ENABLE) < 0) {
    LOG_ERR("Failed to enable sensors");
    return -EIO;
  }

  /* 配置加速度计 - 设置范围和ODR */
  uint8_t acc_cfg =
      (CONFIG_QMI8658C_ACCEL_RANGE << 4) | CONFIG_QMI8658C_ACCEL_ODR;
  if (i2c_reg_write_byte_dt(&cfg->i2c, QMI8658C_REG_CTRL2, acc_cfg) < 0) {
    LOG_ERR("Failed to configure accelerometer");
    return -EIO;
  }

  /* 配置陀螺仪 - 设置范围和ODR */
  uint8_t gyro_cfg =
      (CONFIG_QMI8658C_GYRO_RANGE << 4) | CONFIG_QMI8658C_GYRO_ODR;
  if (i2c_reg_write_byte_dt(&cfg->i2c, QMI8658C_REG_CTRL3, gyro_cfg) < 0) {
    LOG_ERR("Failed to configure gyroscope");
    return -EIO;
  }

  /* 设置加速度计灵敏度 */
  switch (CONFIG_QMI8658C_ACCEL_RANGE) {
  case QMI8658C_ACC_RANGE_2G:
    drv_data->accel_sensitivity_shift = 14; /* 16384 LSB/g */
    break;
  case QMI8658C_ACC_RANGE_4G:
    drv_data->accel_sensitivity_shift = 13; /* 8192 LSB/g */
    break;
  case QMI8658C_ACC_RANGE_8G:
    drv_data->accel_sensitivity_shift = 12; /* 4096 LSB/g */
    break;
  case QMI8658C_ACC_RANGE_16G:
    drv_data->accel_sensitivity_shift = 11; /* 2048 LSB/g */
    break;
  }

  /* 设置陀螺仪灵敏度 */
  float gyro_sensitivity = 0.0;
  switch (CONFIG_QMI8658C_GYRO_RANGE) {
  case QMI8658C_GYR_RANGE_16DPS:
    gyro_sensitivity = 2048.0; /* 2048 LSB/°/s */
    break;
  case QMI8658C_GYR_RANGE_32DPS:
    gyro_sensitivity = 1024.0; /* 1024 LSB/°/s */
    break;
  case QMI8658C_GYR_RANGE_64DPS:
    gyro_sensitivity = 512.0; /* 512 LSB/°/s */
    break;
  case QMI8658C_GYR_RANGE_128DPS:
    gyro_sensitivity = 256.0; /* 256 LSB/°/s */
    break;
  case QMI8658C_GYR_RANGE_256DPS:
    gyro_sensitivity = 128.0; /* 128 LSB/°/s */
    break;
  case QMI8658C_GYR_RANGE_512DPS:
    gyro_sensitivity = 64.0; /* 64 LSB/°/s */
    break;
  case QMI8658C_GYR_RANGE_1024DPS:
    gyro_sensitivity = 32.0; /* 32 LSB/°/s */
    break;
  case QMI8658C_GYR_RANGE_2048DPS:
    gyro_sensitivity = 16.0; /* 16 LSB/°/s */
    break;
  }
  drv_data->gyro_sensitivity_x10 = (uint16_t)(gyro_sensitivity * 10);

#ifdef CONFIG_QMI8658C_TRIGGER
  if (cfg->int_gpio.port) {
    if (qmi8658c_init_interrupt(dev) < 0) {
      LOG_ERR("Failed to initialize interrupts");
      return -EIO;
    }
  }
#endif

  return 0;
}

#define QMI8658C_DEFINE(inst)                                                  \
  static struct qmi8658c_data qmi8658c_data_##inst;                            \
                                                                               \
  static const struct qmi8658c_config qmi8658c_config_##inst = {               \
      .i2c = I2C_DT_SPEC_INST_GET(inst),                                       \
      IF_ENABLED(                                                              \
          CONFIG_QMI8658C_TRIGGER,                                             \
          (.int_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, int_gpios, {0}), ))};    \
                                                                               \
  SENSOR_DEVICE_DT_INST_DEFINE(inst, qmi8658c_init, NULL,                      \
                               &qmi8658c_data_##inst, &qmi8658c_config_##inst, \
                               POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,       \
                               &qmi8658c_driver_api);

DT_INST_FOREACH_STATUS_OKAY(QMI8658C_DEFINE)