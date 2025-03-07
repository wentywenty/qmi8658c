/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #define DT_DRV_COMPAT qst_qmi8658c

 #include <zephyr/drivers/i2c.h>
 #include <zephyr/init.h>
 #include <zephyr/sys/byteorder.h>
 #include <zephyr/drivers/sensor.h>
 #include <zephyr/logging/log.h>
 
 #include "qmi8658c.h"
 
 LOG_MODULE_REGISTER(QMI8658C, CONFIG_SENSOR_LOG_LEVEL);
 
 /* see "Accelerometer Measurements" section from register map description */
 static void qmi8658c_convert_accel(struct sensor_value *val, int16_t raw_val,
								   uint16_t sensitivity_shift)
 {
	 int64_t conv_val;
 
	 conv_val = ((int64_t)raw_val * SENSOR_G) >> sensitivity_shift;
	 val->val1 = conv_val / 1000000;
	 val->val2 = conv_val % 1000000;
 }
 
 /* see "Gyroscope Measurements" section from register map description */
 static void qmi8658c_convert_gyro(struct sensor_value *val, int16_t raw_val,
								  uint16_t sensitivity_x10)
 {
	 int64_t conv_val;
 
	 conv_val = ((int64_t)raw_val * SENSOR_PI * 10) /
				(sensitivity_x10 * 180U);
	 val->val1 = conv_val / 1000000;
	 val->val2 = conv_val % 1000000;
 }
 
 /* see "Temperature Measurement" section from register map description */
 static inline void qmi8658c_convert_temp(enum qmi8658c_device_type device_type,
										 struct sensor_value *val,
										 int16_t raw_val)
 {
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
								struct sensor_value *val)
 {
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
								 enum sensor_channel chan)
 {
	 struct qmi8658c_data *drv_data = dev->data;
	 const struct qmi8658c_config *cfg = dev->config;
	 int16_t buf[7];
 
	 if (i2c_burst_read_dt(&cfg->i2c, QMI8658C_REG_DATA_START, (uint8_t *)buf,
						   14) < 0) {
		 LOG_ERR("Failed to read data sample.");
		 return -EIO;
	 }
 
	 drv_data->accel_x = sys_be16_to_cpu(buf[0]);
	 drv_data->accel_y = sys_be16_to_cpu(buf[1]);
	 drv_data->accel_z = sys_be16_to_cpu(buf[2]);
	 drv_data->temp = sys_be16_to_cpu(buf[3]);
	 drv_data->gyro_x = sys_be16_to_cpu(buf[4]);
	 drv_data->gyro_y = sys_be16_to_cpu(buf[5]);
	 drv_data->gyro_z = sys_be16_to_cpu(buf[6]);
 
	 return 0;
 }
 
 static DEVICE_API(sensor, qmi8658c_driver_api) = {
 #if CONFIG_QMI8658C_TRIGGER
	 .trigger_set = qmi8658c_trigger_set,
 #endif
	 .sample_fetch = qmi8658c_sample_fetch,
	 .channel_get = qmi8658c_channel_get,
 };
 
 int qmi8658c_init(const struct device *dev)
 {
	 struct qmi8658c_data *drv_data = dev->data;
	 const struct qmi8658c_config *cfg = dev->config;
	 uint8_t id, i;
 
	 if (!device_is_ready(cfg->i2c.bus)) {
		 LOG_ERR("Bus device is not ready");
		 return -ENODEV;
	 }
 
	 /* check chip ID */
	 if (i2c_reg_read_byte_dt(&cfg->i2c, QMI8658C_REG_CHIP_ID, &id) < 0) {
		 LOG_ERR("Failed to read chip ID.");
		 return -EIO;
	 }
 
	 if (id == QMI8658C_CHIP_ID) {
		 LOG_DBG("QMI8658C detected");
		 drv_data->device_type = DEVICE_TYPE_QMI8658C;
	 } else {
		 LOG_ERR("Invalid chip ID.");
		 return -EINVAL;
	 }
 
	 /* wake up chip */
	 if (i2c_reg_update_byte_dt(&cfg->i2c, QMI8658C_REG_PWR_MGMT1,
								QMI8658C_SLEEP_EN, 0) < 0) {
		 LOG_ERR("Failed to wake up chip.");
		 return -EIO;
	 }
 
	 /* set accelerometer full-scale range */
	 for (i = 0U; i < 4; i++) {
		 if (BIT(i+1) == CONFIG_QMI8658C_ACCEL_FS) {
			 break;
		 }
	 }
 
	 if (i == 4U) {
		 LOG_ERR("Invalid value for accel full-scale range.");
		 return -EINVAL;
	 }
 
	 if (i2c_reg_write_byte_dt(&cfg->i2c, QMI8658C_REG_ACCEL_CFG,
							   i << QMI8658C_ACCEL_FS_SHIFT) < 0) {
		 LOG_ERR("Failed to write accel full-scale range.");
		 return -EIO;
	 }
 
	 drv_data->accel_sensitivity_shift = 14 - i;
 
	 /* set gyroscope full-scale range */
	 for (i = 0U; i < 4; i++) {
		 if (BIT(i) * 250 == CONFIG_QMI8658C_GYRO_FS) {
			 break;
		 }
	 }
 
	 if (i == 4U) {
		 LOG_ERR("Invalid value for gyro full-scale range.");
		 return -EINVAL;
	 }
 
	 if (i2c_reg_write_byte_dt(&cfg->i2c, QMI8658C_REG_GYRO_CFG,
							   i << QMI8658C_GYRO_FS_SHIFT) < 0) {
		 LOG_ERR("Failed to write gyro full-scale range.");
		 return -EIO;
	 }
 
	 drv_data->gyro_sensitivity_x10 = qmi8658c_gyro_sensitivity_x10[i];
 
 #ifdef CONFIG_QMI8658C_TRIGGER
	 if (cfg->int_gpio.port) {
		 if (qmi8658c_init_interrupt(dev) < 0) {
			 LOG_DBG("Failed to initialize interrupts.");
			 return -EIO;
		 }
	 }
 #endif
 
	 return 0;
 }
 
 #define QMI8658C_DEFINE(inst)                                                    \
	 static struct qmi8658c_data qmi8658c_data_##inst;                            \
																				  \
	 static const struct qmi8658c_config qmi8658c_config_##inst = {               \
		 .i2c = I2C_DT_SPEC_INST_GET(inst),                                       \
		 IF_ENABLED(CONFIG_QMI8658C_TRIGGER,                                      \
					(.int_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, int_gpios, { 0 }),)) \
	 };                                                                           \
																				  \
	 SENSOR_DEVICE_DT_INST_DEFINE(inst, qmi8658c_init, NULL,                      \
						   &qmi8658c_data_##inst, &qmi8658c_config_##inst,        \
						   POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,              \
						   &qmi8658c_driver_api);                                  \
 
 DT_INST_FOREACH_STATUS_OKAY(QMI8658C_DEFINE)