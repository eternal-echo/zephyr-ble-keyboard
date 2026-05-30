if(CONFIG_KEYBOARD_COMMON_KEY_INPUT)
    zephyr_include_directories(key_input)
    add_subdirectory(key_input)
endif()

if(CONFIG_KEYBOARD_COMMON_LED_CONTROL)
    zephyr_include_directories(led_control)
    add_subdirectory(led_control)
endif()