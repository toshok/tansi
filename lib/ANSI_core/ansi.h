
#pragma once

#include <cstdint>

#include "cmd.h"
#include "disk_types.h"

enum AnsiDevState {
    ANSI_DEV_STATE_DISCONNECTED = 0,
    ANSI_DEV_STATE_CONNECTED,
    ANSI_DEV_STATE_SELECTED,
    ANSI_DEV_STATE_READ_COMMAND,
    ANSI_DEV_STATE_AWAITING_PARAM_OUT,
    ANSI_DEV_STATE_EXECUTE_COMMAND,
    ANSI_DEV_STATE_AWAITING_PARAM_IN,
    ANSI_DEV_STATE_AWAITING_TIME_DEPENDENT_COMMAND,
    ANSI_DEV_STATE_READING,
    ANSI_DEV_STATE_WRITING
};

struct AnsiOutPins {
    // the control bus operates both as in and out, but
    // the normal state is out.
    uint8_t cb0: 1;
    uint8_t cb1: 1;
    uint8_t cb2: 1;
    uint8_t cb3: 1;
    uint8_t cb4: 1;
    uint8_t cb5: 1;
    uint8_t cb6: 1;
    uint8_t cb7: 1;
    uint8_t select_out_attn_in_strobe: 1;
    uint8_t command_request: 1;
    uint8_t parameter_request: 1;
    uint8_t bus_direction_out: 1;
    uint8_t port_enable: 1;
    uint8_t read_gate: 1;
    uint8_t write_gate: 1;

    // write clock/data handled elsewhere
};

// ANSI low voltage = logic high, high voltage = logic low
#define ACTIVE(pin) (!pins.pin)
#define INACTIVE(pin) (pins.pin)

struct AnsiDev {
    // values can be 0-7
    uint8_t id;

    AnsiDiskType *disk_type;

    AnsiDevState state;
    AnsiOutPins previous_pins;

    uint8_t cmd;
    uint8_t param_out;
    uint8_t param_in;

    uint8_t general_status;
    uint8_t sense_byte_1;
    uint8_t sense_byte_2;

    bool attention_enabled;
    bool write_enabled;

    uint8_t selected_head;
	uint8_t current_cylinder_high;
    uint8_t current_cylinder_low;

    uint8_t load_cylinder_high;
    uint8_t load_cylinder_low;

    uint8_t test_byte;

    uint8_t attribute_number;
    bool attributes_initialized;
  	uint8_t attributes[0x48];
};

void ansi_poll();
void ansi_read_out_pins(AnsiOutPins *pins);

// called when initializing, and when transitioning from connected to disconnected states
void ansi_initial_state();

// general status bits
#define GS_NOT_READY         0x01
#define GS_CONTROL_BUS_ERROR 0x02
#define GS_ILLEGAL_COMMAND   0x04
#define GS_ILLEGAL_PARAMETER 0x08
#define GS_SENSE_BYTE_1      0x10
#define GS_SENSE_BYTE_2      0x20
#define GS_BUSY_EXECUTING    0x40
#define GS_NORMAL_COMPLETE   0x80

// sense byte 1 bits
#define SB1_SEEK_ERROR          0x01
#define SB1_RW_FAULT            0x02
#define SB1_POWER_FAULT         0x04
#define SB1_RW_PERMIT_VIOLATION 0x08
#define SB1_SPEED_ERROR         0x10
#define SB1_COMMAND_REJECT      0x20
#define SB1_OTHER_ERRORS        0x40
#define SB1_VENDOR_ERRORS       0x80

// sense byte 2 bits
#define SB2_INITIAL_STATE                          0x01
#define SB2_READY_TRANSITION                       0x02
#define SB2_DEV_RESERVED_TO_THIS_POINT             0x04
#define SB2_FORCED_RELEASE                         0x08
#define SB2_DEV_RESERVED_TO_ALT_PORT               0x10
#define SB2_DEVICE_ATTR_TABLE_MODIFIED             0x20
#define SB2_POSITIONED_WITHIN_WRITE_PROTECTED_AREA 0x40
#define SB2_VENDOR_ATTNS						   0x80

void set_general_status(uint8_t value);
void clear_general_status(uint8_t value);
void set_sb1(uint8_t value);
void clear_sb1(uint8_t value);
void set_sb2(uint8_t value);
void clear_sb2(uint8_t value);
void write_control_bus(uint8_t value);

void set_attention_state(bool state);