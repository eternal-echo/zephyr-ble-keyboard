# ESP32C3 17-Key Numpad Keyboard — Zephyr Project

基于 ESP32-C3 的 17 键小键盘，支持 BLE HID 和每键 WS2812B RGB。  
项目包含三个独立应用和一个公共模块库。

## 硬件参考

ZY-17W 17 键矩阵布局：
| 引脚 | 功能 |
|------|------|
| GPIO0-3   | 矩阵列 (col) |
| GPIO6-10  | 矩阵行 (row) |
| GPIO20    | WS2812B 数据 (SPI2 MOSI) |
| GPIO21    | LED 电源门控 (POW_PIN, 低电平有效) |
| GPIO11    | 深度睡眠物理开关 (低电平=开启) |
| GPIO4-5   | 伪滑条 |
| GPIO12-13 | EC11 编码器 |

## 项目结构

```
apps/bluetooth_keyboard/
├── boards/
│   └── esp32c3_devkitm.overlay       # 共享 devicetree overlay
├── modules/keyboard_common/          # 公共模块
│   ├── key_input/                    # 矩阵扫描 + 去抖 (CONFIG_KEY_INPUT)
│   └── led_control/                  # WS2812B + 电源门控 (CONFIG_LED_CONTROL)
├── apps/
│   ├── keyboard/                     # 蓝牙键盘应用
│   ├── led_demo/                     # 全红点亮 Demo（极简）
│   └── led_control/                  # 按键调亮度 Demo
└── README.md
```

## 构建

所有操作在项目目录下执行：

```bash
cd /home/henry/Workspace/zephyrproject/apps/bluetooth_keyboard
```

### 蓝牙键盘

```bash
west build -b esp32c3_devkitm apps/keyboard --pristine
west flash
```

支持 BLE HID、矩阵扫描、WS2812B RGB、深度睡眠。

### LED Demo — 全红

```bash
west build -b esp32c3_devkitm apps/led_demo --pristine -d build/led_demo
west flash
```

上电 17 颗 WS2812B 全亮红色。无键盘 / BLE / PM，验证硬件的最小用例。

### LED Control

```bash
west build -b esp32c3_devkitm apps/led_control --pristine
west flash
```

按下按键 → 对应位置 LED 亮起，松开熄灭。无 BLE / PM，纯演示。

## 模块说明

| 模块 | Kconfig | 功能 |
|------|---------|------|
| key_input | `CONFIG_KEY_INPUT` | 5x4 矩阵扫描，3 次采样去抖，事件回调 |
| led_control | `CONFIG_LED_CONTROL` | WS2812B SPI 驱动，regulator 电源门控 |

应用通过 `prj.conf` 按需开启：led_demo 只开 `CONFIG_LED_CONTROL`，led_control 只开 `CONFIG_KEY_INPUT + CONFIG_LED_CONTROL`，键盘应用全开 + BT + PM。