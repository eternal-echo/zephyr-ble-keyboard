/**
 * @file main.c
 * @brief LED Demo — 点亮所有 WS2812B 为红色
 *
 * 极简演示：上电即点亮 17 颗 WS2812B 为红色。
 * 无矩阵扫描、无 BLE、无亮度调节。
 *
 * 依赖模块：led_control（CONFIG_KEYBOARD_COMMON_LED_CONTROL）
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_demo, LOG_LEVEL_INF);

#include <led_control.h>

int main(void)
{
	LOG_INF("=== LED Demo ===");

	/* 所有灯珠设为红色 */
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		led_pixels[i].r = 0xFF;
		led_pixels[i].g = 0x00;
		led_pixels[i].b = 0x00;
	}

	int ret = led_update_rgb();
	if (ret == 0) {
		LOG_INF("All %d LEDs set to RED OK", LED_NUM_PIXELS);
	} else {
		LOG_ERR("led_update_rgb() failed: %d", ret);
	}

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}