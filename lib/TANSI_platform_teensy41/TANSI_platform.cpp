#include "SD.h"

#include "TANSI_log.h"
#include "TANSI_platform.h"

const char *g_platform_name = PLATFORM_NAME;
const int sdcardCSPin = BUILTIN_SDCARD;

// init GPIO configuration
void platform_init()
{
    SD.begin(BUILTIN_SDCARD);
}

void platform_late_init()
{
    logmsg("Platform: ", g_platform_name);
    logmsg("FW Version: ", g_log_firmwareversion);    
}

void platform_post_sd_card_init()
{
}

void platform_disable_led(void)
{
}

void platform_log(const char *s)
{
    Serial.print(s);
}

void platform_emergency_log_save()
{
}

// Poll function that is called every few milliseconds.
// Can be left empty or used for platform-specific processing.
void platform_poll()
{
}
