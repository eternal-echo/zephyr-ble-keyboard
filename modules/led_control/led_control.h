/**
 * @file led_control.h
 * @brief LED 灯带控制模块接口
 *
 * 硬件（通过设备树 overlay 定义）：
 *   - WS2812B 数据线 → SPI2 MOSI GPIO20（worldsemi,ws2812-spi）
 *   - POW_PIN      → regulator-fixed 节点 led-pwr-ctrl（有源低 MOSFET 门控）
 *
 * 初始化流程（通过 SYS_INIT 自动完成）：
 *   POST_KERNEL 51 — LED 灯带 + regulator 设备绑定
 *
 * 用法：
 *   led_update_rgb() — 自动上电并更新所有像素
 *   led_shutdown()   — 清空像素并断电（进入深度睡眠前调用）
 *
 * @author eternal-echo <zoragail220535@gmail.com>
 * @date   2026-05-30
 * @version 1.0
 */

#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/devicetree.h>

/* Number of WS2812B LEDs — derived from devicetree chain-length property */
#define LED_NUM_PIXELS  DT_PROP(DT_ALIAS(led_strip), chain_length)

#if !DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#error LED_NUM_PIXELS undefined: led-strip alias missing chain-length property
#endif

/* Enable / disable LED power via POW_PIN */
void led_power_enable(bool on);

/* Update all LEDs with RGB data (powers on automatically if off) */
int led_update_rgb(void);

/* Turn off all LEDs and cut power (called before deep sleep) */
void led_shutdown(void);

/* Return POW_PIN GPIO number from DT (for deep sleep GPIO hold) */
int led_get_enable_pin(void);

/*
 * Pixel buffer accessors — fill then call led_update_rgb().
 * Example:
 *   led_pixels[0].r = 0xFF; led_pixels[0].g = 0; led_pixels[0].b = 0;
 *   led_update_rgb();
 */
extern struct led_rgb led_pixels[LED_NUM_PIXELS];

/*
 * Global brightness control (0-255).
 * 255 = full intensity (default), 0 = off.
 * The multiplier is applied inside led_update_rgb() — led_pixels[]
 * always stores the unscaled target values.
 */
void led_set_brightness(uint8_t level);
uint8_t led_get_brightness(void);

#endif /* LED_CONTROL_H */