# QMI8658C 传感器驱动 - Zephyr RTOS

这是一个适用于 Zephyr RTOS 的 QMI8658C 六轴 IMU (陀螺仪+加速度计) 传感器驱动程序。

## 概述

QMI8658C 是 QST 公司生产的高性能六轴传感器，集成三轴加速度计和三轴陀螺仪，通过 I2C 或 SPI 接口通信。该驱动实现了 Zephyr 传感器 API，支持数据读取和中断触发功能。

## 特性

- 完整支持 I2C 接口
- 读取三轴加速度数据
- 读取三轴陀螺仪数据
- 读取温度传感器数据
- 可配置的加速度计量程 (±2g/±4g/±8g/±16g)
- 可配置的陀螺仪量程 (±16°/s 至 ±2048°/s)
- 可配置的输出数据率 (7.8125Hz 至 1000Hz)
- 支持数据就绪中断触发 (全局线程或独立线程)

## 硬件支持

本驱动支持以下硬件功能：

- QMI8658C 传感器芯片
- I2C 接口 (默认地址: 0x6A)
- 可选的中断引脚连接

## 配置选项

在 Kconfig 文件中提供了以下配置选项：

### 基本配置

- `CONFIG_QMI8658C`: 启用 QMI8658C 驱动 (默认: y)

### 加速度计量程配置

- `CONFIG_QMI8658C_ACCEL_RANGE_2G`: 设置加速度计量程为 ±2g
- `CONFIG_QMI8658C_ACCEL_RANGE_4G`: 设置加速度计量程为 ±4g
- `CONFIG_QMI8658C_ACCEL_RANGE_8G`: 设置加速度计量程为 ±8g (默认)
- `CONFIG_QMI8658C_ACCEL_RANGE_16G`: 设置加速度计量程为 ±16g

### 陀螺仪量程配置

- `CONFIG_QMI8658C_GYRO_RANGE_16DPS`: 设置陀螺仪量程为 ±16°/s
- `CONFIG_QMI8658C_GYRO_RANGE_32DPS`: 设置陀螺仪量程为 ±32°/s
- `CONFIG_QMI8658C_GYRO_RANGE_64DPS`: 设置陀螺仪量程为 ±64°/s
- `CONFIG_QMI8658C_GYRO_RANGE_128DPS`: 设置陀螺仪量程为 ±128°/s
- `CONFIG_QMI8658C_GYRO_RANGE_256DPS`: 设置陀螺仪量程为 ±256°/s
- `CONFIG_QMI8658C_GYRO_RANGE_512DPS`: 设置陀螺仪量程为 ±512°/s
- `CONFIG_QMI8658C_GYRO_RANGE_1024DPS`: 设置陀螺仪量程为 ±1024°/s (默认)
- `CONFIG_QMI8658C_GYRO_RANGE_2048DPS`: 设置陀螺仪量程为 ±2048°/s

### 输出数据率配置

- `CONFIG_QMI8658C_ODR_7_8125HZ`: 设置输出数据率为 7.8125 Hz
- `CONFIG_QMI8658C_ODR_15_625HZ`: 设置输出数据率为 15.625 Hz
- `CONFIG_QMI8658C_ODR_31_25HZ`: 设置输出数据率为 31.25 Hz
- `CONFIG_QMI8658C_ODR_62_5HZ`: 设置输出数据率为 62.5 Hz
- `CONFIG_QMI8658C_ODR_125HZ`: 设置输出数据率为 125 Hz (默认)
- `CONFIG_QMI8658C_ODR_250HZ`: 设置输出数据率为 250 Hz
- `CONFIG_QMI8658C_ODR_500HZ`: 设置输出数据率为 500 Hz
- `CONFIG_QMI8658C_ODR_1000HZ`: 设置输出数据率为 1000 Hz

### 触发模式配置

- `CONFIG_QMI8658C_TRIGGER_NONE`: 不使用中断触发 (默认)
- `CONFIG_QMI8658C_TRIGGER_GLOBAL_THREAD`: 使用全局线程处理中断
- `CONFIG_QMI8658C_TRIGGER_OWN_THREAD`: 使用独立线程处理中断

## 使用方法

### 1. 安装驱动

使用提供的 setup.ps1 脚本将驱动安装到您的 Zephyr 项目中：

```powershell
./setup.ps1
```

脚本会询问 Zephyr 根目录，然后将驱动文件复制到正确位置。

### 2. 设备树配置

在您的板子设备树覆盖文件中添加传感器节点：

```dts
&i2c0 {
    status = "okay";
    
    qmi8658c@6a {
        compatible = "invensense,qmi8658c";
        reg = <0x6a>;
        status = "okay";
        /* 可选的中断配置 */
        int-gpios = <&gpio0 7 GPIO_ACTIVE_HIGH>;
    };
};
```

### 3. 项目配置

在项目的 prj.conf 文件中添加必要的配置：

```
CONFIG_I2C=y
CONFIG_SENSOR=y
CONFIG_QMI8658C=y

# 可选: 设置加速度计量程
CONFIG_QMI8658C_ACCEL_RANGE_8G=y

# 可选: 设置陀螺仪量程
CONFIG_QMI8658C_GYRO_RANGE_1024DPS=y

# 可选: 设置输出数据率
CONFIG_QMI8658C_ODR_125HZ=y

# 可选: 启用中断触发
CONFIG_QMI8658C_TRIGGER_GLOBAL_THREAD=y
```

### 4. 应用程序代码

下面是一个简单的示例，演示如何读取传感器数据：

```c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

void main(void)
{
    const struct device *dev = DEVICE_DT_GET_ANY(invensense_qmi8658c);
    
    if (!device_is_ready(dev)) {
        printf("QMI8658C 传感器未找到或未就绪\n");
        return;
    }
    
    printf("QMI8658C 传感器设备已就绪\n");
    
    while (1) {
        struct sensor_value accel[3], gyro[3], temp;
        int rc;
        
        /* 读取传感器数据 */
        rc = sensor_sample_fetch(dev);
        if (rc != 0) {
            printf("读取传感器数据失败: %d\n", rc);
            continue;
        }
        
        /* 获取加速度计数据 */
        rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
        if (rc == 0) {
            printf("加速度 X: %d.%06d, Y: %d.%06d, Z: %d.%06d (m/s^2)\n",
                accel[0].val1, accel[0].val2,
                accel[1].val1, accel[1].val2,
                accel[2].val1, accel[2].val2);
        }
        
        /* 获取陀螺仪数据 */
        rc = sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, gyro);
        if (rc == 0) {
            printf("陀螺仪 X: %d.%06d, Y: %d.%06d, Z: %d.%06d (rad/s)\n",
                gyro[0].val1, gyro[0].val2,
                gyro[1].val1, gyro[1].val2,
                gyro[2].val1, gyro[2].val2);
        }
        
        /* 获取温度数据 */
        rc = sensor_channel_get(dev, SENSOR_CHAN_DIE_TEMP, &temp);
        if (rc == 0) {
            printf("温度: %d.%06d °C\n", temp.val1, temp.val2);
        }
        
        /* 等待一段时间 */
        k_sleep(K_MSEC(1000));
    }
}
```

### 5. 使用中断触发

如果要使用中断触发功能，请确保配置了 `int-gpios` 属性并启用了相应的触发选项：

```c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

static void trigger_handler(const struct device *dev,
                           const struct sensor_trigger *trigger)
{
    /* 在中断触发时读取数据 */
    struct sensor_value accel[3], gyro[3];
    
    sensor_sample_fetch(dev);
    sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
    sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, gyro);
    
    printk("触发事件: 加速度 X=%d.%06d\n", 
           accel[0].val1, accel[0].val2);
}

void main(void)
{
    const struct device *dev = DEVICE_DT_GET_ANY(invensense_qmi8658c);
    
    if (!device_is_ready(dev)) {
        return;
    }
    
    /* 设置传感器触发 */
    struct sensor_trigger trig = {
        .type = SENSOR_TRIG_DATA_READY,
        .chan = SENSOR_CHAN_ALL,
    };
    
    sensor_trigger_set(dev, &trig, trigger_handler);
    
    while (1) {
        k_sleep(K_FOREVER);
    }
}
```

## 故障排除

### 常见问题

1. **驱动未启用**
   检查 prj.conf 中是否包含 `CONFIG_QMI8658C=y` 以及设备树中的兼容性字符串是否为 `invensense,qmi8658c`。

2. **找不到设备**
   确保正确连接了 I2C 电路，并且设备地址设置正确（通常为 0x6A）。

3. **中断不工作**
   检查设备树中的 `int-gpios` 配置是否正确，以及是否启用了适当的触发配置选项。

## 许可证

本驱动程序遵循 Apache-2.0 许可证分发。

## 参考资料

- [QMI8658C 数据手册](https://qstcorp.com/upload/pdf/202202/QMI8658C%20datasheet%20rev%200.9.pdf)
- [Zephyr 传感器 API 文档](https://docs.zephyrproject.org/latest/reference/peripherals/sensor.html)
