
#pragma once

#include <Arduino.h>
#include "TeensyANSI_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char *g_platform_name;

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

// Reinitialize SD card connection and save log from interrupt context.
// This can be used in crash handlers.
void platform_emergency_log_save();

#ifdef __cplusplus
}
#endif