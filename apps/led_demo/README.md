# LED Demo

两个板型，同一份代码，条件编译区分功能。

## 板型

| 板型 | 标识符 | 控制台 | LED 灯带 |
|---|---|---|---|
| CH340 串口版 | `esp32c3_mini_uart` | UART0 (GPIO21 TX) | 无（GPIO20 被 UART RX 占用） |
| USB 版 | `esp32c3_mini_usb` | USB Serial/JTAG (CDC ACM) | WS2812 via UART0 TX=GPIO20 |

## 构建

```bash
cd /home/henry/Workspace/zephyrproject/apps/bluetooth_keyboard
source /home/henry/Workspace/zephyrproject/.venv/bin/activate

# CH340 串口版 — 仅打印日志，无灯带
west build -b esp32c3_mini_uart apps/led_demo -d build_uart -p always
west flash -d build_uart

# USB 版 — WS2812 灯带演示
west build -b esp32c3_mini_usb apps/led_demo -d build_usb -p always
west flash -d build_usb
```

## 行为

- **CH340 版**: 每 5 秒通过串口打印一条消息
- **USB 版**: WS2812 灯带循环演示 — 全红 → 全绿 → 全蓝 → 熄灭 → 彩虹跑马

## 模块依赖

- `led_control` — WS2812B 驱动 + 电源门控（通过 `CONFIG_KEYBOARD_COMMON_LED_CONTROL` 控制）