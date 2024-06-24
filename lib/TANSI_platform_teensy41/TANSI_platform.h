
#pragma once

#include "TANSI_gpio.h"
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char* g_platform_name;

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
void platform_log(const char* s);

// Poll function that is called every few milliseconds.
// Can be left empty or used for platform-specific processing.
void platform_poll();

// Reinitialize SD card connection and save log from interrupt context.
// This can be used in crash handlers.
void platform_emergency_log_save();

#define platform_read_pin(pin) digitalReadFast(pin)

// "in" and "out" here are from the perspective of the host (to match the rest
// of the ansi spec/terminology.)
enum ControlBusDirection { CONTROL_BUS_IN, CONTROL_BUS_OUT };
void platform_set_control_bus_direction(ControlBusDirection direction);
void platform_write_control_bus_byte(uint8_t v);

#ifdef __cplusplus
}
#endif