#pragma once

#include <cstdint>

typedef enum {
    ANSI_CMD_REPORT_ILLEGAL_COMMAND = 0x00,
    ANSI_CMD_CLEAR_FAULT = 0x01,
    ANSI_CMD_CLEAR_ATTENTION = 0x02,
    ANSI_CMD_SEEK = 0x03,
    ANSI_CMD_REZERO = 0x04,
    ANSI_CMD_REPORT_SENSE_BYTE_2 = 0x0D,
    ANSI_CMD_REPORT_SENSE_BYTE_1 = 0x0E,
    ANSI_CMD_REPORT_GENERAL_STATUS = 0x0F,
    // --
    ANSI_CMD_REPORT_ATTRIBUTE = 0x10,
    ANSI_CMD_SET_ATTENTION = 0x11,
    ANSI_CMD_SELECTIVE_RESET = 0x14,
    ANSI_CMD_SEEK_TO_LANDING_ZONE = 0x15,
    ANSI_CMD_REFORMAT_TRACK = 0x16,
    // --
    ANSI_CMD_REPORT_CYL_ADDR_HIGH = 0x29,
    ANSI_CMD_REPORT_CYL_ADDR_LOW = 0x2A,
    ANSI_CMD_REPORT_READ_PERMIT_HIGH = 0x2B,
    ANSI_CMD_REPORT_READ_PERMIT_LOW = 0x2C,
    ANSI_CMD_REPORT_WRITE_PERMIT_HIGH = 0x2D,
    ANSI_CMD_REPORT_WRITE_PERMIT_LOW = 0x2E,
    ANSI_CMD_REPORT_TEST_BYTE = 0x2F,
    // --
    ANSI_CMD_ATTENTION_CONTROL = 0x40,
    ANSI_CMD_WRITE_CONTROL = 0x41,
    ANSI_CMD_LOAD_CYL_ADDR_HIGH = 0x42,
    ANSI_CMD_LOAD_CYL_ADDR_LOW = 0x43,
    ANSI_CMD_SELECT_HEAD = 0x44,
    // --
    ANSI_CMD_LOAD_ATTRIBUTE_NUMBER = 0x50,
    ANSI_CMD_LOAD_ATTRIBUTE = 0x51,
    ANSI_CMD_READ_CONTROL = 0x53,   // XXX only in apollo eng handbook
    ANSI_CMD_OFFSET_CONTROL = 0x54, // XXX only in apollo eng handbook
    ANSI_CMD_SPIN_CONTROL = 0x55,
    ANSI_CMD_LOAD_SECT_PER_TRACK_HIGH = 0x56,   // MSB
    ANSI_CMD_LOAD_SECT_PER_TRACK_MEDIUM = 0x57, // MedSB
    ANSI_CMD_LOAD_SECT_PER_TRACK_LOW = 0x58,    // LSB
    ANSI_CMD_LOAD_BYTES_PER_SECT_HIGH = 0x59,   // MSB
    ANSI_CMD_LOAD_BYTES_PER_SECT_MEDIUM = 0x5A, // MedSB
    ANSI_CMD_LOAD_BYTES_PER_SECT_LOW = 0x5B,    // LSB
                                                // --
    ANSI_CMD_LOAD_READ_PERMIT_HIGH = 0x6B,  // XXX only in apollo eng handbook
    ANSI_CMD_LOAD_READ_PERMIT_LOW = 0x6C,   // XXX only in apollo eng handbook
    ANSI_CMD_LOAD_WRITE_PERMIT_HIGH = 0x6D, // XXX only in apollo eng handbook
    ANSI_CMD_LOAD_WRITE_PERMIT_LOW = 0x6E,  // XXX only in apollo eng handbook
    ANSI_CMD_LOAD_TEST_BYTE = 0x6F,
} AnsiCmd;

void ansi_execute_command();
bool ansi_poll_time_dependent();

static inline bool command_is_param_out(uint8_t cmd) {
    return (cmd & 0x40) != 0;
}
static inline bool command_is_param_in(uint8_t cmd) {
    return (cmd & 0x40) == 0;
}

static inline bool command_is_time_dependent(uint8_t cmd) {
    // there doesn't seem to be a nice bit check possible for this, so we just
    // check for the specific command values from the spec.
    return (cmd == ANSI_CMD_SPIN_CONTROL || cmd == ANSI_CMD_SEEK ||
            cmd == ANSI_CMD_REZERO || cmd == ANSI_CMD_SET_ATTENTION ||
            cmd == ANSI_CMD_REFORMAT_TRACK);
}
