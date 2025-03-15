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

很抱歉，我无法看到您提到的PDF文件，因为它没有成功上传或附加到消息中。要帮助您基于QMI8658C数据手册修改驱动程序，我需要了解该传感器的具体规格。

以下是为QMI8658C传感器创建Zephyr驱动程序的通用步骤，您可以结合实际的数据手册信息进行调整：

1. **创建驱动目录结构**
   ```
   zephyr/drivers/sensor/qst/qmi8658c/
   ```

2. **创建或修改以下文件**：
   - qmi8658c.h - 包含寄存器定义和数据结构
   - qmi8658c.c - 主驱动功能实现
   - qmi8658c_trigger.c - 中断触发功能实现
   - CMakeLists.txt - 构建配置
   - Kconfig - 驱动配置选项

3. **关键寄存器信息**
   通常IMU传感器需要以下寄存器：
   - WHO_AM_I寄存器 (芯片ID)
   - 配置寄存器(范围设置、采样率等)
   - 数据输出寄存器(X,Y,Z轴加速度和陀螺仪数据)
   - 中断配置寄存器

4. **设备树绑定文件**
   创建 `qmi8658c.yaml` 文件，定义设备树绑定

5. **在prj.conf中启用**
   ```
   CONFIG_I2C=y
   CONFIG_SENSOR=y
   CONFIG_QMI8658C=y
   ```

如果您能提供QMI8658C的关键寄存器地址、数据格式和配置信息，或者成功上传PDF文件，我可以更具体地帮助您修改代码以匹配实际的硬件规格。
