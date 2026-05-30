/**
 * @file key_input.c
 * @brief 按键输入模块（矩阵扫描 + 去抖）
 *
 * 提供键盘矩阵扫描、去抖和事件回调功能，支持轮询模式。
 * SPI 注意事项：WS2812B 数据线与矩阵行线（GPIO6/10）共用 SPI2，
 * 通过 pinctrl 在 SPI 和 GPIO 模式间切换。
 *
 * @author eternal-echo <zoragail220535@gmail.com>
 * @date   2026-05-30
 * @version 1.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>

#include "key_input.h"

LOG_MODULE_REGISTER(key_input, LOG_LEVEL_INF);

/* =========================================================================
 * Pin definitions
 * ========================================================================= */
const uint8_t row_pins[MATRIX_ROWS] = { 6, 7, 8, 9, 10 };
const uint8_t col_pins[MATRIX_COLS] = { 0, 1, 2, 3 };

/* =========================================================================
 * Shared state
 * ========================================================================= */
const struct device *gpio_port;
int32_t idle_counter_ms;

/* =========================================================================
 * Debounce
 * ========================================================================= */
#define DEBOUNCE_STABLE_COUNT  3
#define SCAN_INTERVAL_MS       10

static uint8_t raw_state[MATRIX_ROWS];
static uint8_t debounced_state[MATRIX_ROWS];
static uint8_t debounce_counter[MATRIX_ROWS][MATRIX_COLS];
static key_event_callback_t user_callback;

/* =========================================================================
 * Initialisation  (SYS_INIT at POST_KERNEL 50 — GPIO driver ready)
 * ========================================================================= */
static int key_input_hw_init(void)
{
	gpio_port = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_port)) {
		LOG_ERR("GPIO device not ready");
		return -ENODEV;
	}

	for (int r = 0; r < MATRIX_ROWS; r++) {
		int ret = gpio_pin_configure(gpio_port, row_pins[r], GPIO_OUTPUT_HIGH);
		if (ret < 0) {
			LOG_ERR("Row %d (GPIO%d) config failed: %d", r, row_pins[r], ret);
			return ret;
		}
	}

	for (int c = 0; c < MATRIX_COLS; c++) {
		int ret = gpio_pin_configure(gpio_port, col_pins[c],
					     GPIO_INPUT | GPIO_PULL_DOWN);
		if (ret < 0) {
			LOG_ERR("Col %d (GPIO%d) config failed: %d", c, col_pins[c], ret);
			return ret;
		}
	}

	LOG_INF("Matrix initialized: %dx%d", MATRIX_ROWS, MATRIX_COLS);
	return 0;
}

SYS_INIT(key_input_hw_init, POST_KERNEL, 50);

int key_input_register_callback(key_event_callback_t cb)
{
	user_callback = cb;
	return 0;
}

/* =========================================================================
 * Scanning
 * ========================================================================= */
static void scan_row(int row)
{
	gpio_pin_set(gpio_port, row_pins[row], 0);
	k_busy_wait(50);

	uint8_t row_state = 0;
	for (int c = 0; c < MATRIX_COLS; c++) {
		int val = gpio_pin_get(gpio_port, col_pins[c]);
		if (val < 0) {
			LOG_ERR("Col read failed on row %d col %d", row, c);
			continue;
		}
		if (val == 0) {
			row_state |= BIT(c);
		}
	}

	gpio_pin_set(gpio_port, row_pins[row], 1);
	raw_state[row] = row_state;
}

static void debounce_matrix(void)
{
	for (int r = 0; r < MATRIX_ROWS; r++) {
		for (int c = 0; c < MATRIX_COLS; c++) {
			bool raw_pressed = (raw_state[r] & BIT(c)) != 0;
			bool debounced_pressed = (debounced_state[r] & BIT(c)) != 0;

			if (raw_pressed == debounced_pressed) {
				debounce_counter[r][c] = 0;
			} else {
				debounce_counter[r][c]++;
				if (debounce_counter[r][c] >= DEBOUNCE_STABLE_COUNT) {
					if (raw_pressed) {
						debounced_state[r] |= BIT(c);
					} else {
						debounced_state[r] &= ~BIT(c);
					}
					debounce_counter[r][c] = 0;
				}
			}
		}
	}
}

/* =========================================================================
 * Process changes → user callback
 * ========================================================================= */
static void process_matrix_changes(uint8_t old_state[MATRIX_ROWS],
				   uint8_t new_state[MATRIX_ROWS])
{
	if (!user_callback) {
		return;
	}

	for (int r = 0; r < MATRIX_ROWS; r++) {
		uint8_t changed = old_state[r] ^ new_state[r];
		if (changed == 0) {
			continue;
		}
		for (int c = 0; c < MATRIX_COLS; c++) {
			if (!(changed & BIT(c))) {
				continue;
			}
			bool pressed = (new_state[r] & BIT(c)) != 0;
			user_callback(r, c, pressed);
			idle_counter_ms = 0;
		}
	}
}

/* =========================================================================
 * Poll-mode: scan thread
 * ========================================================================= */
#ifdef CONFIG_KEYBOARD_COMMON_KEY_INPUT_POLL
static void matrix_scan_thread_fn(void *arg1, void *arg2, void *arg3)
{
	uint8_t prev_state[MATRIX_ROWS] = { 0 };

	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (1) {
		for (int r = 0; r < MATRIX_ROWS; r++) {
			scan_row(r);
		}

		debounce_matrix();
		process_matrix_changes(prev_state, debounced_state);
		memcpy(prev_state, debounced_state, sizeof(prev_state));

		idle_counter_ms += SCAN_INTERVAL_MS;
		k_sleep(K_MSEC(SCAN_INTERVAL_MS));
	}
}

K_THREAD_DEFINE(key_input_tid, 2048, matrix_scan_thread_fn, NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);

void key_input_start(void)
{
	k_thread_start(key_input_tid);
}

static int key_input_poll_start(void)
{
	k_thread_start(key_input_tid);
	return 0;
}

SYS_INIT(key_input_poll_start, APPLICATION, 70);
#endif /* CONFIG_KEYBOARD_COMMON_KEY_INPUT_POLL */