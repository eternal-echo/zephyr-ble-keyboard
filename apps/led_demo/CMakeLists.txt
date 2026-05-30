cmake_minimum_required(VERSION 3.20.0)

# Find the keyboard_common module (relative to project root)
set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../modules")

# Allow Zephyr to find our custom boards
set(BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(led_demo)

target_sources(app PRIVATE
    src/main.c
)