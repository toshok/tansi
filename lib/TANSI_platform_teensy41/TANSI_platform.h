
#pragma once

#include <Arduino.h>
#include "TANSI_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char *g_platform_name;

#define PLATFORM_NAME "TANSI v0.1 (Teensy 4.1)"

// Initialize SPI and GPIO configuration
void platform_init();

// Initialization for main application, not used for bootloader
void platform_late_init();

// Initialization after the SD Card has been found
void platform_post_sd_card_init();

// Disable the status LED
void platform_disable_led(void);

// Debug logging functions
void platform_log(const char *s);

// Poll function that is called every few milliseconds.
// Can be left empty or used for platform-specific processing.
void platform_poll();

// Reinitialize SD card connection and save log from interrupt context.
// This can be used in crash handlers.
void platform_emergency_log_save();

enum ControlBusDirection {
    CONTROL_BUS_INPUT,
    CONTROL_BUS_OUTPUT
};
void platform_set_control_bus_direction(ControlBusDirection direction);

#ifdef __cplusplus
}
#endif