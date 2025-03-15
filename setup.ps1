<#
.SYNOPSIS
    将QMI8658C驱动部署到Zephyr项目中
.DESCRIPTION
    此脚本会询问Zephyr根目录，然后将QMI8658C驱动文件复制到正确的位置。
    如果目标位置已存在文件，将直接替换。
#>

# 设置当前工作目录
$sourceDir = $PSScriptRoot
Write-Host "源文件目录: $sourceDir" -ForegroundColor Cyan

# 询问Zephyr根目录
$zephyrRootPath = Read-Host "请输入Zephyr项目的根目录路径 (例如: D:\zephyrproject\zephyrproject\zephyr)"

# 验证路径是否存在
if (-not (Test-Path $zephyrRootPath)) {
    Write-Host "错误: 指定的路径不存在!" -ForegroundColor Red
    exit 1
}

# 验证是否为Zephyr根目录
if (-not (Test-Path "$zephyrRootPath\include\zephyr") -or -not (Test-Path "$zephyrRootPath\drivers")) {
    Write-Host "警告: 指定的路径可能不是Zephyr根目录。请确认这是正确的路径。" -ForegroundColor Yellow
    $confirmation = Read-Host "是否继续? (y/n)"
    if ($confirmation -ne "y") {
        exit 1
    }
}

# 创建驱动程序目录结构
$driverPath = "$zephyrRootPath\drivers\sensor\qst\qmi8658c"
if (-not (Test-Path "$zephyrRootPath\drivers\sensor\qst")) {
    Write-Host "创建目录: $zephyrRootPath\drivers\sensor\qst"
    New-Item -ItemType Directory -Force -Path "$zephyrRootPath\drivers\sensor\qst" | Out-Null
}
if (-not (Test-Path $driverPath)) {
    Write-Host "创建目录: $driverPath"
    New-Item -ItemType Directory -Force -Path $driverPath | Out-Null
}

# 创建设备树绑定目录
$dtsBindingPath = "$zephyrRootPath\dts\bindings\sensor\qst"
if (-not (Test-Path "$zephyrRootPath\dts\bindings\sensor")) {
    Write-Host "创建目录: $zephyrRootPath\dts\bindings\sensor"
    New-Item -ItemType Directory -Force -Path "$zephyrRootPath\dts\bindings\sensor" | Out-Null
}
if (-not (Test-Path $dtsBindingPath)) {
    Write-Host "创建目录: $dtsBindingPath"
    New-Item -ItemType Directory -Force -Path $dtsBindingPath | Out-Null
}

# 复制文件到目标位置
Write-Host "复制驱动文件..." -ForegroundColor Green

# 复制源代码文件
Copy-Item -Path "$sourceDir\qmi8658c.h" -Destination "$driverPath\" -Force
Copy-Item -Path "$sourceDir\qmi8658c.c" -Destination "$driverPath\" -Force
Copy-Item -Path "$sourceDir\qmi8658c_trigger.c" -Destination "$driverPath\" -Force
Copy-Item -Path "$sourceDir\CMakeLists.txt" -Destination "$driverPath\" -Force
Copy-Item -Path "$sourceDir\Kconfig" -Destination "$driverPath\" -Force

# 复制设备树绑定文件
Copy-Item -Path "$sourceDir\invensense,qmi8658c.yaml" -Destination "$dtsBindingPath\qmi8658c.yaml" -Force

# 更新主Kconfig和CMakeLists.txt文件
$sensorKconfigPath = "$zephyrRootPath\drivers\sensor\Kconfig"
$sensorCMakeListsPath = "$zephyrRootPath\drivers\sensor\CMakeLists.txt"

# 检查并添加到Kconfig
$sensorKconfig = Get-Content -Path $sensorKconfigPath
if (-not ($sensorKconfig -match "source.*qst\/qmi8658c\/Kconfig")) {
    Write-Host "更新 $sensorKconfigPath" -ForegroundColor Green
    Add-Content -Path $sensorKconfigPath -Value "`nsource `"sensor/qst/qmi8658c/Kconfig`""
}

# 检查并添加到CMakeLists.txt
$sensorCMakeLists = Get-Content -Path $sensorCMakeListsPath
if (-not ($sensorCMakeLists -match "add_subdirectory_ifdef.*qst\/qmi8658c")) {
    Write-Host "更新 $sensorCMakeListsPath" -ForegroundColor Green
    Add-Content -Path $sensorCMakeListsPath -Value "`nadd_subdirectory_ifdef(CONFIG_QMI8658C qst/qmi8658c)"
}

# 创建QST主目录的Kconfig和CMakeLists.txt
$qstDirPath = "$zephyrRootPath\drivers\sensor\qst"
$qstKconfigPath = "$qstDirPath\Kconfig"
$qstCMakeListsPath = "$qstDirPath\CMakeLists.txt"

if (-not (Test-Path $qstKconfigPath)) {
    Write-Host "创建 $qstKconfigPath" -ForegroundColor Green
    Set-Content -Path $qstKconfigPath -Value "# QST Sensor drivers`n`nsource `"sensor/qst/qmi8658c/Kconfig`""
}

if (-not (Test-Path $qstCMakeListsPath)) {
    Write-Host "创建 $qstCMakeListsPath" -ForegroundColor Green
    Set-Content -Path $qstCMakeListsPath -Value "# QST Sensor drivers`n`nadd_subdirectory_ifdef(CONFIG_QMI8658C qmi8658c)"
}

Write-Host "QMI8658C驱动程序已成功部署到Zephyr项目!" -ForegroundColor Green
Write-Host "您现在可以在设备树中添加以下内容来使用QMI8658C传感器:" -ForegroundColor Cyan
Write-Host @"

&i2c0 {
    status = "okay";

    qmi8658c@6a {
        compatible = "qst,qmi8658c";
        reg = <0x6a>;
        status = "okay";
    };
};

"@ -ForegroundColor White

Write-Host "然后在您的项目配置文件(prj.conf)中添加:" -ForegroundColor Cyan
Write-Host @"

CONFIG_I2C=y
CONFIG_SENSOR=y
CONFIG_QMI8658C=y

"@ -ForegroundColor White
