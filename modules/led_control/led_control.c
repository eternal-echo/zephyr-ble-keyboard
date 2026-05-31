#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL

/**
 * @file led_control.c
 * @brief LED 灯带控制模块（WS2812B）
 *
 * 基于 Zephyr LED strip 框架驱动 WS2812B 灯带。
 * POW_PIN（LED 电源开关）由 regulator-fixed 设备树节点控制（可选）。
 *
 * @author eternal-echo <zoragail220535@gmail.com>
 * @date   2026-05-30
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <string.h>

#include "led_control.h"

LOG_MODULE_REGISTER(led_control, LOG_LEVEL_INF);

/* =========================================================================
 * Devicetree accessors
 * ========================================================================= */
#define STRIP_NODE        DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS  DT_PROP(DT_ALIAS(led_strip), chain_length)

#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL_POWER_GATE
#include <zephyr/drivers/regulator.h>
#define LED_PWR_NODE      DT_NODELABEL(led_pwr)
#define LED_PWR_GPIO_PIN  DT_GPIO_PIN(LED_PWR_NODE, enable_gpios)
#endif

/* =========================================================================
 * Shared state
 * ========================================================================= */
static const struct device *strip_dev;
struct led_rgb led_pixels[STRIP_NUM_PIXELS];

#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL_POWER_GATE
static const struct device *reg_dev;
#endif

/* Global brightness: 255 = full, 0 = off */
static uint8_t global_brightness = 255;

/* =========================================================================
 * Initialization
 * ========================================================================= */
static int led_hw_init(void)
{
	strip_dev = DEVICE_DT_GET(STRIP_NODE);
	if (!device_is_ready(strip_dev)) {
		LOG_ERR("LED strip device not ready");
		return -ENODEV;
	}

#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL_POWER_GATE
	reg_dev = DEVICE_DT_GET(LED_PWR_NODE);
	if (!device_is_ready(reg_dev)) {
		LOG_ERR("POW_PIN regulator not ready");
		return -ENODEV;
	}
	LOG_INF("LED strip initialized: %d pixels, POW_PIN=GPIO%d",
		 STRIP_NUM_PIXELS, LED_PWR_GPIO_PIN);
#else
	LOG_INF("LED strip initialized: %d pixels (no power gate)",
		 STRIP_NUM_PIXELS);
#endif
	return 0;
}

SYS_INIT(led_hw_init, APPLICATION, 0);

/* =========================================================================
 * Power gate via regulator (no-op if CONFIG_KEYBOARD_COMMON_LED_CONTROL_POWER_GATE=n)
 * ========================================================================= */
void led_power_enable(bool on)
{
#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL_POWER_GATE
	if (on) {
		regulator_enable(reg_dev);
	} else {
		regulator_disable(reg_dev);
	}
#else
	ARG_UNUSED(on);
#endif
}

/* =========================================================================
 * Update LEDs (with brightness scaling)
 * ========================================================================= */
int led_update_rgb(void)
{
	led_power_enable(true);
	k_busy_wait(100); /* Allow MOSFET + LED stabilization */

	struct led_rgb scaled[STRIP_NUM_PIXELS];

	for (int i = 0; i < STRIP_NUM_PIXELS; i++) {
		scaled[i].r = (led_pixels[i].r * global_brightness) / 255;
		scaled[i].g = (led_pixels[i].g * global_brightness) / 255;
		scaled[i].b = (led_pixels[i].b * global_brightness) / 255;
	}

	int ret = led_strip_update_rgb(strip_dev, scaled, STRIP_NUM_PIXELS);
	if (ret < 0) {
		LOG_ERR("led_strip_update_rgb failed: %d", ret);
		return ret;
	}

	return 0;
}

/* =========================================================================
 * Brightness control
 * ========================================================================= */
void led_set_brightness(uint8_t level)
{
	global_brightness = level;
}

uint8_t led_get_brightness(void)
{
	return global_brightness;
}

/* =========================================================================
 * Shutdown (for deep sleep)
 * ========================================================================= */
void led_shutdown(void)
{
	/* Clear pixel buffer so LEDs go black before power cut */
	memset(led_pixels, 0, sizeof(led_pixels));
	led_strip_update_rgb(strip_dev, led_pixels, STRIP_NUM_PIXELS);
	k_busy_wait(1000);

#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL_POWER_GATE
	/* Cut power via regulator */
	led_power_enable(false);
#endif
}

/* =========================================================================
 * PM helper — expose enable pin number for deep sleep GPIO hold
 * ========================================================================= */
int led_get_enable_pin(void)
{
#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL_POWER_GATE
	return LED_PWR_GPIO_PIN;
#else
	return -1;
#endif
}

#endif /* CONFIG_KEYBOARD_COMMON_LED_CONTROL */