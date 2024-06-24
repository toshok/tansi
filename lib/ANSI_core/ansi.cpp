
#include "ansi.h"

#include "TANSI_log.h"
#include "TANSI_platform.h"

// strings for state names
static const char* state_names[] = {"DISCONNECTED",
                                    "CONNECTED",
                                    "SELECTED",
                                    "READ_COMMAND",
                                    "AWAITING_PARAM_OUT",
                                    "EXECUTE_COMMAND",
                                    "AWAITING_PARAM_IN",
                                    "AWAITING_TIME_DEPENDENT_COMMAND",
                                    "READING",
                                    "WRITING"};

AnsiDev gAnsiDev;

static void ansi_sample_out_pins(AnsiOutPins& pins) {
#define READ_PIN(pinName) pins.pin_##pinName = platform_read_pin(ANSI_##pinName)

    READ_PIN(CB0);
    READ_PIN(CB1);
    READ_PIN(CB2);
    READ_PIN(CB3);
    READ_PIN(CB4);
    READ_PIN(CB5);
    READ_PIN(CB6);
    READ_PIN(CB7);

    READ_PIN(SELECT_OUT_ATTN_IN_STROBE);
    READ_PIN(COMMAND_REQUEST);
    READ_PIN(PARAMETER_REQUEST);
    READ_PIN(BUS_DIRECTION_OUT);
    READ_PIN(PORT_ENABLE);
    READ_PIN(READ_GATE);
    READ_PIN(WRITE_GATE);
}

static uint8_t control_bus_byte(AnsiOutPins& pins) {
    uint8_t v = 0;
    if (PIN(pins, CB0))
        v |= 0x01;
    if (PIN(pins, CB1))
        v |= 0x02;
    if (PIN(pins, CB2))
        v |= 0x04;
    if (PIN(pins, CB3))
        v |= 0x08;
    if (PIN(pins, CB4))
        v |= 0x10;
    if (PIN(pins, CB5))
        v |= 0x20;
    if (PIN(pins, CB6))
        v |= 0x40;
    if (PIN(pins, CB7))
        v |= 0x80;
    return v;
}

void ansi_poll() {
    static bool first_poll = true;
    AnsiDevState cur_state;
    AnsiDevState next_state;
    AnsiOutPins pins;

    if (first_poll) {
        first_poll = false;
        logmsg("ANSI initial state ", state_names[gAnsiDev.state]);
    }

    ansi_sample_out_pins(pins);
#if 0
    logmsg("ANSI pins: cb0=", pins.cb0, " cb1=", pins.cb1, " cb2=", pins.cb2, " cb3=", pins.cb3, " cb4=", pins.cb4, " cb5=", pins.cb5, " cb6=", pins.cb6, " cb7=", pins.cb7, " seai=", pins.select_out_attn_in_strobe, " pe=", pins.port_enable);
#endif

    cur_state = gAnsiDev.state;
    // if nothing else changes it, the next state is the same as the current
    // state.
    next_state = cur_state;

    // regardless of the state transitions, set the control bus direction based
    // on the state of the bus_direction_out pin.
    platform_set_control_bus_direction(
        ACTIVE(pins, BUS_DIRECTION_OUT) ? CONTROL_BUS_OUT : CONTROL_BUS_IN);

    switch (cur_state) {
    case ANSI_DEV_STATE_DISCONNECTED: {
        // The only pin we watch for changing here is
        // port enable.
        if (INACTIVE(pins, PORT_ENABLE)) {
            break;
        }

        next_state = ANSI_DEV_STATE_CONNECTED;
        break;
    }
    case ANSI_DEV_STATE_CONNECTED: {
        // if port enable is inactive, we are disconnected
        if (INACTIVE(pins, PORT_ENABLE)) {
            next_state = ANSI_DEV_STATE_DISCONNECTED;
            ansi_initial_state();
            break;
        }

        if (INACTIVE(pins, SELECT_OUT_ATTN_IN_STROBE)) {
            break;
        }

        if (INACTIVE(pins, BUS_DIRECTION_OUT)) {
            break;
        }

        uint8_t id = control_bus_byte(pins);
        if (id & (1 << gAnsiDev.id)) {
            // we are selected
            next_state = ANSI_DEV_STATE_SELECTED;
            SET_ACTIVE(BUS_ACKNOWLEDGE);
        }
        break;
    }

    case ANSI_DEV_STATE_SELECTED: {
        if (ACTIVE(pins, BUS_DIRECTION_OUT) && ACTIVE(pins, COMMAND_REQUEST)) {
            // start of command handshake
            next_state = ANSI_DEV_STATE_READ_COMMAND;
            break;
        }

        if (ACTIVE(pins, READ_GATE)) {
            // start reading!
            next_state = ANSI_DEV_STATE_READING;
            break;
        }

        if (ACTIVE(pins, WRITE_GATE)) {
            // start writing!
            next_state = ANSI_DEV_STATE_WRITING;
            break;
        }

        if (INACTIVE(pins, SELECT_OUT_ATTN_IN_STROBE)) {
            // nothing to do here, we're still selected
            break;
        }

        // select_out/attn_in strobe is active, so depending on the state
        // of bus_direction, selection is changing or we should gate our
        // attention onto our radial line.
        if (ACTIVE(pins, BUS_DIRECTION_OUT)) {
            uint8_t cb = control_bus_byte(pins);
            if (cb & (1 << gAnsiDev.id)) {
                // we are still selected
                next_state = ANSI_DEV_STATE_SELECTED;
                SET_ACTIVE(BUS_ACKNOWLEDGE);
            } else {
                // we are no longer selected
                next_state = ANSI_DEV_STATE_CONNECTED;
            }
            break;
        }

        // bus_direction is in, so we should gate our attention onto the control
        // bus.
        platform_write_control_bus_byte(1 << gAnsiDev.id);
        break;
    }

    case ANSI_DEV_STATE_READ_COMMAND: {
        // the command comes from the control bus and is a byte
        gAnsiDev.cmd = control_bus_byte(pins);

        SET_ACTIVE(BUS_ACKNOWLEDGE);

        if (command_is_param_out(gAnsiDev.cmd)) {
            next_state = ANSI_DEV_STATE_AWAITING_PARAM_OUT;
            break;
        }

        // otherwise, we can execute the command.  The parameter request
        // will be single_ended_activeed as usual, but we'll be the ones writing
        // the parameter.
        next_state = ANSI_DEV_STATE_EXECUTE_COMMAND;
        break;
    }

    case ANSI_DEV_STATE_AWAITING_PARAM_OUT: {
        if (!ACTIVE(pins, BUS_DIRECTION_OUT) ||
            !ACTIVE(pins, PARAMETER_REQUEST)) {
            break;
        }

        gAnsiDev.param_out = control_bus_byte(pins);
        SET_ACTIVE(BUS_ACKNOWLEDGE);
        next_state = ANSI_DEV_STATE_EXECUTE_COMMAND;
        break;
    }

    case ANSI_DEV_STATE_EXECUTE_COMMAND: {
        bool time_dependent = command_is_time_dependent(gAnsiDev.cmd);
        if (time_dependent) {
            // activate the busy signal
            SET_ACTIVE(BUSY);
        }
        ansi_execute_command();
        if (command_is_param_out(gAnsiDev.cmd)) {
            next_state = time_dependent
                             ? ANSI_DEV_STATE_AWAITING_TIME_DEPENDENT_COMMAND
                             : ANSI_DEV_STATE_SELECTED;
        } else {
            next_state = ANSI_DEV_STATE_AWAITING_PARAM_IN;
        }
        break;
    }

    case ANSI_DEV_STATE_AWAITING_TIME_DEPENDENT_COMMAND: {
        if (ansi_poll_time_dependent()) {
            // we aren't done yet.
            break;
        }

        // we're done with the time dependent command
        // deactivate the busy signal
        SET_INACTIVE(BUSY);
        SET_ACTIVE(ATTENTION);
        // XXX(toshok) clear the busy GS bit?
        next_state = ANSI_DEV_STATE_SELECTED;
        break;
    }

    case ANSI_DEV_STATE_AWAITING_PARAM_IN: {
        if (ACTIVE(pins, BUS_DIRECTION_OUT) ||
            INACTIVE(pins, PARAMETER_REQUEST)) {
            break;
        }

        bool time_dependent = command_is_time_dependent(gAnsiDev.cmd);
        platform_write_control_bus_byte(gAnsiDev.param_in);
        // what's the handshake part of this?  presumably the host needs to ack?
        next_state = time_dependent
                         ? ANSI_DEV_STATE_AWAITING_TIME_DEPENDENT_COMMAND
                         : ANSI_DEV_STATE_SELECTED;
        break;
    }
    case ANSI_DEV_STATE_READING: {
        // XXX(toshok) implement
        break;
    }
    case ANSI_DEV_STATE_WRITING: {
        // XXX(toshok) implement
        break;
    }
    default: {
        logmsg("ANSI unknown state ", cur_state);
        break;
    }
    }

    if (cur_state != next_state) {
        logmsg("ANSI state ", state_names[cur_state], " -> ",
               state_names[next_state]);
        gAnsiDev.state = next_state;
    }

    gAnsiDev.previous_pins = pins;
}

void ansi_initial_state() { gAnsiDev.attributes_initialized = false; }

void set_general_status(uint8_t value) {}

void clear_general_status(uint8_t value) {}

void set_sb1(uint8_t value) {
    if ((gAnsiDev.sense_byte_1 & value) != value) {
        gAnsiDev.sense_byte_1 |= value;
        // all sb1 bits set attention on 0->1 transition
        set_attention_state(true);
        set_general_status(GS_SENSE_BYTE_1);
    }
}

void clear_sb1(uint8_t value) {
    gAnsiDev.sense_byte_1 &= ~value;
    if (gAnsiDev.sense_byte_1 == 0) {
        clear_general_status(GS_SENSE_BYTE_1);
    } else {
        set_general_status(GS_SENSE_BYTE_1);
    }
}

void set_sb2(uint8_t value) {
    if ((gAnsiDev.sense_byte_2 & value) != value) {
        gAnsiDev.sense_byte_2 |= value;
        // only certain sb2 bits set attention on 0->1 transition
        if (value |
            (SB2_INITIAL_STATE | SB2_READY_TRANSITION | SB2_FORCED_RELEASE |
             SB2_DEVICE_ATTR_TABLE_MODIFIED | SB2_VENDOR_ATTNS)) {
            set_attention_state(true);
            set_general_status(GS_SENSE_BYTE_2);
        }
    }
}

void clear_sb2(uint8_t value) {
    gAnsiDev.sense_byte_2 &= ~value;
    if (gAnsiDev.sense_byte_2 == 0) {
        clear_general_status(GS_SENSE_BYTE_2);
    } else {
        set_general_status(GS_SENSE_BYTE_2);
    }
}

void set_attention_state(bool state) {
    gAnsiDev.attention_enabled = state;
    SET_BOOL(ATTENTION, state);
}