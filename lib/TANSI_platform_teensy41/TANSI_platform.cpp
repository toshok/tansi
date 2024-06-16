#include "SD.h"

#include "TANSI_log.h"
#include "TANSI_platform.h"

const char *g_platform_name = PLATFORM_NAME;
const int sdcardCSPin = BUILTIN_SDCARD;
const int ledPin = LED_BUILTIN;

// init GPIO configuration
void platform_init()
{
    SD.begin(BUILTIN_SDCARD);

    pinMode(ANSI_CB0, INPUT);
    pinMode(ANSI_CB1, INPUT);
    pinMode(ANSI_CB2, INPUT);
    pinMode(ANSI_CB3, INPUT);
    pinMode(ANSI_CB4, INPUT);
    pinMode(ANSI_CB5, INPUT);
    pinMode(ANSI_CB6, INPUT);
    pinMode(ANSI_CB7, INPUT);

    pinMode(ANSI_SELECT_OUT_ATTN_IN_STROBE, INPUT);
    pinMode(ANSI_COMMAND_REQUEST, INPUT);
    pinMode(ANSI_PARAMETER_REQUEST, INPUT);
    pinMode(ANSI_BUS_DIRECTION_OUT, INPUT);
    pinMode(ANSI_READ_GATE, INPUT);
    pinMode(ANSI_WRITE_GATE, INPUT);

    pinMode(ANSI_PORT_ENABLE, INPUT);

    pinMode(ANSI_BUS_ACKNOWLEDGE, OUTPUT_OPENDRAIN);
    pinMode(ANSI_INDEX, OUTPUT_OPENDRAIN);
    pinMode(ANSI_SECTOR_MARK, OUTPUT_OPENDRAIN);
    pinMode(ANSI_ATTENTION, OUTPUT_OPENDRAIN);
    pinMode(ANSI_BUSY, OUTPUT_OPENDRAIN);

    pinMode(ANSI_READ_DATA, OUTPUT_OPENDRAIN);
    pinMode(ANSI_READ_REF_CLOCK, OUTPUT_OPENDRAIN);
    pinMode(ANSI_WRITE_CLOCK, INPUT);
    pinMode(ANSI_WRITE_DATA, INPUT);
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
