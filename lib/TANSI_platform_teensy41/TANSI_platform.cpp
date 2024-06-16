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

    // we start out with reading from the control bus
    platform_set_control_bus_direction(CONTROL_BUS_INPUT);

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

void platform_set_control_bus_direction(ControlBusDirection direction)
{
    const bool input = direction == CONTROL_BUS_INPUT;
    pinMode(ANSI_CB0, input ? INPUT : OUTPUT);
    pinMode(ANSI_CB1, input ? INPUT : OUTPUT);
    pinMode(ANSI_CB2, input ? INPUT : OUTPUT);
    pinMode(ANSI_CB3, input ? INPUT : OUTPUT);
    pinMode(ANSI_CB4, input ? INPUT : OUTPUT);
    pinMode(ANSI_CB5, input ? INPUT : OUTPUT);
    pinMode(ANSI_CB6, input ? INPUT : OUTPUT);
    pinMode(ANSI_CB7, input ? INPUT : OUTPUT);
}
