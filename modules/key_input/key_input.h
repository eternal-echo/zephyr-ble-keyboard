/**
 * @file key_input.h
 * @brief 按键输入模块接口
 *
 * 5×4 矩阵扫描（行 GPIO6-10，列 GPIO0-3），3 次采样去抖，10ms 扫描间隔。
 * 初始化流程（通过 SYS_INIT 自动完成）：
 *   POST_KERNEL 50 — GPIO 引脚配置
 *   APPLICATION 70 — 扫描线程启动（轮询模式）
 *
 * 用法：
 *   key_input_register_callback(my_handler);
 *   // 扫描线程已在运行，按键变化时触发回调
 *
 * @author eternal-echo <zoragail220535@gmail.com>
 * @date   2026-05-30
 * @version 1.0
 */

#ifndef KEY_INPUT_H
#define KEY_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>

#define MATRIX_ROWS  5
#define MATRIX_COLS  4

/* Pin tables (shared with PM layer) */
extern const uint8_t row_pins[MATRIX_ROWS];
extern const uint8_t col_pins[MATRIX_COLS];

/* GPIO port used by matrix */
extern const struct device *gpio_port;

/* Idle counter: incremented by scan thread, read by PM decision */
extern int32_t idle_counter_ms;

/* Callback type: invoked on every debounced state change */
typedef void (*key_event_callback_t)(uint8_t row, uint8_t col, bool pressed);

/*
 * Register the event callback.
 * Hardware init runs automatically at POST_KERNEL via SYS_INIT.
 */
int key_input_register_callback(key_event_callback_t cb);

#ifdef CONFIG_KEYBOARD_COMMON_KEY_INPUT_POLL
/* Start the scan thread (K_THREAD_DEFINE'd internally). */
void key_input_start(void);

/* Thread handle for k_thread_start() in main */
extern const k_tid_t key_input_tid;
#endif

#endif /* KEY_INPUT_H */