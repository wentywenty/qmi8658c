# QMI8658C Sensor Driver for Zephyr RTOS

这是一个用于 Zephyr RTOS 的 QMI8658C 6轴传感器驱动程序。

## 特性

- 支持 I2C 接口
- 支持加速度计和陀螺仪数据读取
- 支持数据就绪中断触发
- 支持全局线程和独立线程两种中断处理方式
- 可配置的加速度计和陀螺仪量程

## 硬件支持

- QMI8658C 6轴传感器
- I2C 接口
- 中断引脚(INT)

## 配置选项

主要的 Kconfig 选项:

- `CONFIG_QMI8658C_TRIGGER_NONE`: 禁用中断触发
- `CONFIG_QMI8658C_TRIGGER_GLOBAL_THREAD`: 使用全局线程处理中断
- `CONFIG_QMI8658C_TRIGGER_OWN_THREAD`: 使用独立线程处理中断
- `CONFIG_QMI8658C_ACCEL_FS`: 加速度计量程配置(±2g/±4g/±8g/±16g)
- `CONFIG_QMI8658C_GYRO_FS`: 陀螺仪量程配置(±250/±500/±1000/±2000 dps)

## 使用方法

1. 在设备树中添加传感器节点:

```dts
&i2c0 {
    qmi8658c@6a {
        compatible = "qst,qmi8658c";
        reg = <0x6a>;
        int-gpios = <&gpio0 7 GPIO_ACTIVE_HIGH>;
    };
};
```

2. 在工程配置中启用驱动:

```kconfig
CONFIG_SENSOR=y
CONFIG_QMI8658C=y
```

3. 在应用程序中使用传感器API访问数据

## 许可证

Apache-2.0
