/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "qmi8658c.h"
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(QMI8658C, CONFIG_SENSOR_LOG_LEVEL);

int qmi8658c_trigger_set(const struct device *dev,
                         const struct sensor_trigger *trig,
                         sensor_trigger_handler_t handler) {
  struct qmi8658c_data *drv_data = dev->data;
  const struct qmi8658c_config *cfg = dev->config;

  if (!cfg->int_gpio.port) {
    return -ENOTSUP;
  }

  if (trig->type != SENSOR_TRIG_DATA_READY) {
    return -ENOTSUP;
  }

  gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_DISABLE);

  drv_data->data_ready_handler = handler;
  if (handler == NULL) {
    return 0;
  }

  drv_data->data_ready_trigger = trig;

  gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_EDGE_TO_ACTIVE);

  return 0;
}

static void qmi8658c_gpio_callback(const struct device *dev,
                                   struct gpio_callback *cb, uint32_t pins) {
  struct qmi8658c_data *drv_data =
      CONTAINER_OF(cb, struct qmi8658c_data, gpio_cb);
  const struct qmi8658c_config *cfg = drv_data->dev->config;

  ARG_UNUSED(pins);

  gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_DISABLE);

#if defined(CONFIG_QMI8658C_TRIGGER_OWN_THREAD)
  k_sem_give(&drv_data->gpio_sem);
#elif defined(CONFIG_QMI8658C_TRIGGER_GLOBAL_THREAD)
  k_work_submit(&drv_data->work);
#endif
}

static void qmi8658c_thread_cb(const struct device *dev) {
  struct qmi8658c_data *drv_data = dev->data;
  const struct qmi8658c_config *cfg = dev->config;

  if (drv_data->data_ready_handler != NULL) {
    drv_data->data_ready_handler(dev, drv_data->data_ready_trigger);
  }

  gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_EDGE_TO_ACTIVE);
}

#ifdef CONFIG_QMI8658C_TRIGGER_OWN_THREAD
static void qmi8658c_thread(void *p1, void *p2, void *p3) {
  ARG_UNUSED(p2);
  ARG_UNUSED(p3);

  struct qmi8658c_data *drv_data = p1;

  while (1) {
    k_sem_take(&drv_data->gpio_sem, K_FOREVER);
    qmi8658c_thread_cb(drv_data->dev);
  }
}
#endif

#ifdef CONFIG_QMI8658C_TRIGGER_GLOBAL_THREAD
static void qmi8658c_work_cb(struct k_work *work) {
  struct qmi8658c_data *drv_data =
      CONTAINER_OF(work, struct qmi8658c_data, work);

  qmi8658c_thread_cb(drv_data->dev);
}
#endif

int qmi8658c_init_interrupt(const struct device *dev) {
  struct qmi8658c_data *drv_data = dev->data;
  const struct qmi8658c_config *cfg = dev->config;

  if (!gpio_is_ready_dt(&cfg->int_gpio)) {
    LOG_ERR("GPIO device not ready");
    return -ENODEV;
  }

  drv_data->dev = dev;

  gpio_pin_configure_dt(&cfg->int_gpio, GPIO_INPUT);

  gpio_init_callback(&drv_data->gpio_cb, qmi8658c_gpio_callback,
                     BIT(cfg->int_gpio.pin));

  if (gpio_add_callback(cfg->int_gpio.port, &drv_data->gpio_cb) < 0) {
    LOG_ERR("Failed to set gpio callback");
    return -EIO;
  }

  /* enable data ready interrupt */
  if (i2c_reg_write_byte_dt(&cfg->i2c, QMI8658C_REG_INT_EN, QMI8658C_DRDY_EN) <
      0) {
    LOG_ERR("Failed to enable data ready interrupt.");
    return -EIO;
  }

#if defined(CONFIG_QMI8658C_TRIGGER_OWN_THREAD)
  k_sem_init(&drv_data->gpio_sem, 0, K_SEM_MAX_LIMIT);

  k_thread_create(&drv_data->thread, drv_data->thread_stack,
                  CONFIG_QMI8658C_THREAD_STACK_SIZE, qmi8658c_thread, drv_data,
                  NULL, NULL, K_PRIO_COOP(CONFIG_QMI8658C_THREAD_PRIORITY), 0,
                  K_NO_WAIT);
#elif defined(CONFIG_QMI8658C_TRIGGER_GLOBAL_THREAD)
  drv_data->work.handler = qmi8658c_work_cb;
#endif

  gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_EDGE_TO_ACTIVE);

  return 0;
}
