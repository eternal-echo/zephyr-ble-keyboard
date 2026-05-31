#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_demo, LOG_LEVEL_INF);

#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL
#include <led_control.h>

/* Sleep durations in milliseconds */
#define PATTERN_MS  5000
#define CYCLE_MS    200

static void set_all_pixels(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		led_pixels[i].r = r;
		led_pixels[i].g = g;
		led_pixels[i].b = b;
	}
}

static void led_demo_run(void)
{
	LOG_INF("LED strip demo started (%d pixels)", LED_NUM_PIXELS);

	led_set_brightness(80); /* ~31% brightness to avoid drawing too much current */

	while (1) {
		/* All red */
		LOG_INF("All red");
		set_all_pixels(0xFF, 0x00, 0x00);
		led_update_rgb();
		k_sleep(K_MSEC(PATTERN_MS));

		/* All green */
		LOG_INF("All green");
		set_all_pixels(0x00, 0xFF, 0x00);
		led_update_rgb();
		k_sleep(K_MSEC(PATTERN_MS));

		/* All blue */
		LOG_INF("All blue");
		set_all_pixels(0x00, 0x00, 0xFF);
		led_update_rgb();
		k_sleep(K_MSEC(PATTERN_MS));

		/* All off */
		LOG_INF("All off");
		set_all_pixels(0x00, 0x00, 0x00);
		led_update_rgb();
		k_sleep(K_MSEC(PATTERN_MS));

		/* Rainbow cycle: each pixel sweeps through hues */
		LOG_INF("Rainbow cycle");
		for (int step = 0; step < LED_NUM_PIXELS; step++) {
			for (int i = 0; i < LED_NUM_PIXELS; i++) {
				int hue = (i * 256 / LED_NUM_PIXELS + step * 16) % 256;
				uint8_t r, g, b;
				uint8_t sector = hue / 43;
				uint8_t frac = (hue % 43) * 6;

				switch (sector) {
				case 0: r = 0xFF; g = frac;      b = 0;     break;
				case 1: r = 0xFF - frac; g = 0xFF; b = 0;     break;
				case 2: r = 0;    g = 0xFF; b = frac;      break;
				case 3: r = 0;    g = 0xFF - frac; b = 0xFF; break;
				case 4: r = frac;      g = 0;     b = 0xFF; break;
				default:r = 0xFF; g = 0;     b = 0xFF - frac; break;
				}
				led_pixels[i].r = r;
				led_pixels[i].g = g;
				led_pixels[i].b = b;
			}
			led_update_rgb();
			k_sleep(K_MSEC(CYCLE_MS));
		}
	}
}

#else /* !CONFIG_KEYBOARD_COMMON_LED_CONTROL */

static void console_demo_run(void)
{
	LOG_INF("LED Demo — no LED strip on this board variant");
	LOG_INF("Board uses CH340 UART; GPIO20 is occupied by UART RX");

	while (1) {
		LOG_INF("Hello from ESP32-C3-MINI (CH340 UART)");
		k_sleep(K_SECONDS(5));
	}
}

#endif /* CONFIG_KEYBOARD_COMMON_LED_CONTROL */

int main(void)
{
	LOG_INF("LED Demo starting...");

#ifdef CONFIG_KEYBOARD_COMMON_LED_CONTROL
	led_demo_run();
#else
	console_demo_run();
#endif

	return 0;
}