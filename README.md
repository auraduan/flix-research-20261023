# Flix ESP32 迷你无人机飞控（MOS3）

基于 [okalachev/flix](https://github.com/okalachev/flix) 的 ESP32 四旋翼固件，面向 **ESP32-S3 SuperMini + MOSFET 驱动有刷电机** 的一体板（ESP32-UAV-MOS3）。

## 功能

- 串口 CLI（`help` / `mfr` / `ca` / `cr` / `ps` / `arm` 等）
- 姿态估计（陀螺仪积分 + 落地加速度修正）
- 增稳 / Acro / RAW / Auto 飞行模式
- 有刷电机 MOSFET 输出（LEDC PWM，LEDC 失败时可退回数字电平）
- 可选 Wi-Fi SoftAP + MAVLink（QGroundControl）
- 可选 RC（默认关闭）

## 硬件

| 项目 | 说明 |
|------|------|
| 主控 | ESP32-S3 SuperMini |
| 电机 | 有刷空心杯（如 8520），经 MOSFET 低边驱动 |
| IMU | GY-91 / SPI MPU9250（`FlixPeriph`） |
| 电池 | 1S 锂电，**电机测试不要用 USB 供电** |

### 电机引脚（本板）

| 命令 | 角色 | GPIO | 网络 | 座子 |
|------|------|------|------|------|
| `mfr` | 前右 | 9  | S4 → Q3 | CN4 |
| `mfl` | 前左 | 13 | S3 → Q2 | CN3 |
| `mrr` | 后右 | 12 | S2 → Q1 | CN2 |
| `mrl` | 后左 | 10 | S1 → Q4 | CN5 |

> 测试电机时 **务必拆下桨叶**。

## 编译与烧录

1. 安装 [Arduino IDE 2.x](https://www.arduino.cc/en/software)（建议 2.3+）
2. 开发板管理安装 **esp32 by Espressif Systems**（需 ≥ 3.0，使用 `ledcAttach`）
3. 安装库：
   - **MAVLink**
   - **FlixPeriph**
4. 打开 `Flix_Main20260521.ino`
5. 板型选择：**ESP32S3 Dev Module**
   - USB CDC On Boot：Enabled（用板载 Type-C 串口时）
   - USB Mode：Hardware CDC and JTAG
6. 上传后打开串口监视器，波特率 **115200**，行尾建议 **Both NL & CR** 或 **Newline**

## 首次使用

1. 接电池（不要只靠 USB 驱动电机）
2. 按 EN，确认启动日志中 `Motorx pin...: 1`（LEDC 挂载成功）
3. `mfr` / `mfl` / `mrr` / `mrl` 测试电机
4. `ca` 校准加速度计（六面，水平放置要稳）
5. 有遥控时再 `cr` 校准 RC
6. 油门拉低、方向打满可 arm / disarm（手势）或用串口 `arm` / `disarm`

### 常用串口命令

| 命令 | 作用 |
|------|------|
| `help` | 帮助 |
| `p` / `p <name> [value]` | 查看/设置参数 |
| `mfr` `mfl` `mrr` `mrl` | 测试单电机（3 秒） |
| `ca` | 校准加速度计 |
| `cr` | 校准 RC |
| `ps` / `psq` / `imu` | 姿态 / 四元数 / IMU |
| `mot` | 电机输出 |
| `arm` / `disarm` | 解锁 / 上锁 |
| `stab` / `acro` / `raw` / `auto` | 飞行模式 |
| `reset` / `reboot` | 姿态复位 / 重启 |

## Wi-Fi / QGC

默认热点：

- SSID：`Drone_WiFi`
- 密码：`12345678`
- UDP：`14550`

不需要 Wi-Fi 时，将主程序中的 `WIFI_ENABLED` 改为 `0`。

## 目录结构

```
Flix_Main20260521/
├── Flix_Main20260521.ino  # setup / loop
├── motors.ino             # PWM 与电机测试
├── control.ino            # 姿态/角速率控制
├── estimate.ino           # 姿态估计
├── imu.ino                # IMU
├── cli.ino                # 串口命令
├── mavlink.ino            # MAVLink
├── wifi.ino               # SoftAP
├── parameters.ino         # 参数
├── rc.ino / safety.ino    # 遥控与失效保护
└── *.h                    # 数学与滤波
```

## 安全

- **无桨**再测电机；电机测试必须用电池供电  
- 调参、上锁前不要装桨试飞  
- 参数整定失败可能导致姿态发散，先 `disarm` / 断电  

## 致谢

- 原项目：[okalachev/flix](https://github.com/okalachev/flix)  
- 中文注释与板级适配参考开源 ESP32 迷你无人机相关工程  

## 许可

上游 flix 使用其原仓库许可；本仓库二次分发前请自行确认并保留原作者署名。
