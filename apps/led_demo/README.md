# LED Demo — 全红点亮

上电即点亮所有 17 颗 WS2812B 为红色。  
极简演示，验证 LED 硬件和 `led_control` 模块工作正常。

## 构建

```bash
cd /home/henry/Workspace/zephyrproject/apps/bluetooth_keyboard
source .venv/bin/activate

west build -b esp32c3_devkitm apps/led_demo --pristine -d build/led_demo

west flash
```

## 模块依赖

- `led_control` — WS2812B 驱动 + 电源门控（通过 `CONFIG_KEYBOARD_COMMON_LED_CONTROL` 控制）