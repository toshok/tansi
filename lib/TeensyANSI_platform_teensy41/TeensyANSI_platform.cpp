#include "SD.h"

#include "TeensyANSI_platform.h"

const int sdcardCSPin = BUILTIN_SDCARD;

// init GPIO configuration
void platform_init()
{
    SD.begin(sdcardCSPin);
    // SdFat.begin(sdcardPin, SD_SCK_MHZ(50));
}

void platform_late_init()
{
}

void platform_post_sd_card_init()
{
}

void platform_disable_led(void)
{
}

void platform_log(const char *s)
{
}

void platform_emergency_log_save()
{
}

// Poll function that is called every few milliseconds.
// Can be left empty or used for platform-specific processing.
void platform_poll()
{
}
