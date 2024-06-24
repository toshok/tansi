#include "SD.h"

#include "TANSI_log.h"
#include "TANSI_platform.h"

const char* g_platform_name = PLATFORM_NAME;
// const int sdcardCSPin = BUILTIN_SDCARD;
// const int ledPin = LED_BUILTIN;

// init GPIO configuration
void platform_init() {
    SD.begin(BUILTIN_SDCARD);

    // we start out with reading from the control bus
    //
    // TODO(toshok) maybe this call should be made at the ansi layer so we don't
    // need to duplicate it on each platform?
    platform_set_control_bus_direction(CONTROL_BUS_OUT);

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

void platform_late_init() {
    logmsg("Platform: ", g_platform_name);
    logmsg("FW Version: ", g_log_firmwareversion);
}

void platform_post_sd_card_init() {}

void platform_disable_led(void) {}

void platform_log(const char* s) { Serial.print(s); }

void platform_emergency_log_save() {}

// Poll function that is called every few milliseconds.
// Can be left empty or used for platform-specific processing.
void platform_poll() {}

void platform_set_control_bus_direction(ControlBusDirection direction) {
    const bool output_from_drive = direction == CONTROL_BUS_IN;
    pinMode(ANSI_CB0, output_from_drive ? OUTPUT : INPUT);
    pinMode(ANSI_CB1, output_from_drive ? OUTPUT : INPUT);
    pinMode(ANSI_CB2, output_from_drive ? OUTPUT : INPUT);
    pinMode(ANSI_CB3, output_from_drive ? OUTPUT : INPUT);
    pinMode(ANSI_CB4, output_from_drive ? OUTPUT : INPUT);
    pinMode(ANSI_CB5, output_from_drive ? OUTPUT : INPUT);
    pinMode(ANSI_CB6, output_from_drive ? OUTPUT : INPUT);
    pinMode(ANSI_CB7, output_from_drive ? OUTPUT : INPUT);
}

void platform_write_control_bus_byte(uint8_t v) {
    digitalWriteFast(ANSI_CB0, v & 0x01 ? LOW : HIGH);
    digitalWriteFast(ANSI_CB1, v & 0x02 ? LOW : HIGH);
    digitalWriteFast(ANSI_CB2, v & 0x04 ? LOW : HIGH);
    digitalWriteFast(ANSI_CB3, v & 0x08 ? LOW : HIGH);
    digitalWriteFast(ANSI_CB4, v & 0x10 ? LOW : HIGH);
    digitalWriteFast(ANSI_CB5, v & 0x20 ? LOW : HIGH);
    digitalWriteFast(ANSI_CB6, v & 0x40 ? LOW : HIGH);
    digitalWriteFast(ANSI_CB7, v & 0x80 ? LOW : HIGH);
}