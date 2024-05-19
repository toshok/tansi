
#include "ansi.h"

#include "TANSI_platform.h"
#include "TANSI_log.h"

// strings for state names
static const char* state_names[] = {
    "DISCONNECTED",
    "CONNECTED",
    "SELECTED",
    "READ_COMMAND",
    "AWAITING_PARAM_OUT",
    "EXECUTE_COMMAND",
    "AWAITING_PARAM_IN",
    "AWAITING_TIME_DEPENDENT_COMMAND",
    "READING",
    "WRITING"
};

AnsiDev gAnsiDev;

static uint8_t read_control_bus(AnsiOutPins* pins) {
    uint8_t v = 0;
    if (pins->cb0) v |= 0x01;
    if (pins->cb1) v |= 0x02;
    if (pins->cb2) v |= 0x04;
    if (pins->cb3) v |= 0x08;
    if (pins->cb4) v |= 0x10;
    if (pins->cb5) v |= 0x20;
    if (pins->cb6) v |= 0x40;
    if (pins->cb7) v |= 0x80;
    return v;
}

void
ansi_poll() {
    static bool first_poll = true;
    AnsiDevState next_state;
    AnsiOutPins pins;

    if (first_poll) {
        first_poll = false;
        logmsg("ANSI initial state ", state_names[gAnsiDev.state]);
    }

    ansi_read_out_pins(&pins);

    next_state = gAnsiDev.state;
    switch (gAnsiDev.state) {
    case ANSI_DEV_STATE_DISCONNECTED:
        // The only pin we watch for changing here is
        // port enable.
        if (ACTIVE(port_enable)) {
            next_state = ANSI_DEV_STATE_CONNECTED;
        }
        break;

    case ANSI_DEV_STATE_CONNECTED:
        // if port enable is inactive, we are disconnected
        if (INACTIVE(port_enable)) {
            gAnsiDev.state = ANSI_DEV_STATE_DISCONNECTED;
            ansi_initial_state();
            break;
        }

        if (ACTIVE(select_out_attn_in_strobe)) {
            if (ACTIVE(bus_direction_out)) {
                uint8_t cb = read_control_bus(&pins);
                if (cb & (1<<gAnsiDev.id)) {
                    // we are selected
                    next_state = ANSI_DEV_STATE_SELECTED;
                    SET_ACTIVE(ANSI_BUS_ACKNOWLEDGE);
                }
            }
        }
        break;

    case ANSI_DEV_STATE_SELECTED:
        // start of command handshake
        if (ACTIVE(bus_direction_out) && ACTIVE(command_request)) {
            next_state = ANSI_DEV_STATE_READ_COMMAND;
            break;
        }

        if (ACTIVE(select_out_attn_in_strobe)) {
            if (ACTIVE(bus_direction_out)) {
                uint8_t cb = read_control_bus(&pins);
                if (cb & (1<<gAnsiDev.id)) {
                    // we are still selected
                    next_state = ANSI_DEV_STATE_SELECTED;
                    SET_ACTIVE(ANSI_BUS_ACKNOWLEDGE);
                } else {
                    // we are no longer selected
                    next_state = ANSI_DEV_STATE_CONNECTED;
                }
                break;
            } else {
                // TODO(toshok) gate our attention state onto our radial line.
            }
        }
        break;

    case ANSI_DEV_STATE_READ_COMMAND:
        // the command comes from the control bus and is a byte
        gAnsiDev.cmd = read_control_bus(&pins);

        SET_ACTIVE(ANSI_BUS_ACKNOWLEDGE);

        if (command_is_param_out(gAnsiDev.cmd)) {
            next_state = ANSI_DEV_STATE_AWAITING_PARAM_OUT;
            break;
        } else {
            // otherwise, we can execute the command.  The parameter request
            // will be single_ended_activeed as usual, but we'll be the ones writing the
            // parameter.
            next_state = ANSI_DEV_STATE_EXECUTE_COMMAND;
            break;
        }

    case ANSI_DEV_STATE_AWAITING_PARAM_OUT:
        if (ACTIVE(bus_direction_out) && ACTIVE(parameter_request)) {
            gAnsiDev.param_out = read_control_bus(&pins);
            SET_ACTIVE(ANSI_BUS_ACKNOWLEDGE);
            next_state = ANSI_DEV_STATE_EXECUTE_COMMAND;
        }
        break;

    case ANSI_DEV_STATE_EXECUTE_COMMAND: {
        bool time_dependent = command_is_time_dependent(gAnsiDev.cmd);
        if (time_dependent) {
            // activate the busy signal
            SET_ACTIVE(ANSI_BUSY);
        }
        ansi_execute_command();
        if (command_is_param_out(gAnsiDev.cmd)) {
            next_state = time_dependent ? ANSI_DEV_STATE_AWAITING_TIME_DEPENDENT_COMMAND : ANSI_DEV_STATE_SELECTED;
        } else {
            next_state = ANSI_DEV_STATE_AWAITING_PARAM_IN;
        }
        break;
    }

    case ANSI_DEV_STATE_AWAITING_TIME_DEPENDENT_COMMAND: {
        if (!ansi_poll_time_dependent()) {
            // we're done with the time dependent command
            // deactivate the busy signal
            SET_INACTIVE(ANSI_BUSY);
            SET_ACTIVE(ANSI_ATTENTION);
            // XXX(toshok) clear the busy GS bit?
            next_state = ANSI_DEV_STATE_SELECTED;
        }
        break;
    }

    case ANSI_DEV_STATE_AWAITING_PARAM_IN: {
        if (INACTIVE(bus_direction_out) && ACTIVE(parameter_request)) {
            bool time_dependent = command_is_time_dependent(gAnsiDev.cmd);
            write_control_bus(gAnsiDev.param_in);
            // what's the handshake part of this?  presumably the host needs to ack?
            next_state = time_dependent ? ANSI_DEV_STATE_AWAITING_TIME_DEPENDENT_COMMAND : ANSI_DEV_STATE_SELECTED;
        }
        break;
    }
    }

    if (gAnsiDev.state != next_state) {
        logmsg("ANSI state ", state_names[gAnsiDev.state], " -> ", state_names[next_state]);
    }

    gAnsiDev.state = next_state;
    gAnsiDev.previous_pins = pins;
}

void
ansi_read_out_pins(AnsiOutPins *pins) {
    pins->cb0 = digitalReadFast(ANSI_CB0);
    pins->cb1 = digitalReadFast(ANSI_CB1);
    pins->cb2 = digitalReadFast(ANSI_CB2);
    pins->cb3 = digitalReadFast(ANSI_CB3);
    pins->cb4 = digitalReadFast(ANSI_CB4);
    pins->cb5 = digitalReadFast(ANSI_CB5);
    pins->cb6 = digitalReadFast(ANSI_CB6);
    pins->cb7 = digitalReadFast(ANSI_CB7);

    pins->select_out_attn_in_strobe = digitalReadFast(ANSI_SELECT_OUT_ATTN_IN_STROBE);
    pins->command_request = digitalReadFast(ANSI_COMMAND_REQUEST);
    pins->parameter_request = digitalReadFast(ANSI_PARAMETER_REQUEST);
    pins->bus_direction_out = digitalReadFast(ANSI_BUS_DIRECTION_OUT);
    pins->port_enable = digitalReadFast(ANSI_PORT_ENABLE);
    pins->read_gate = digitalReadFast(ANSI_READ_GATE);
    pins->write_gate = digitalReadFast(ANSI_WRITE_GATE);
}

void ansi_initial_state() {
    gAnsiDev.attributes_initialized = false;
}

void set_general_status(uint8_t value) {
}

void clear_general_status(uint8_t value) {
}

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
        if (value | (
            SB2_INITIAL_STATE |
            SB2_READY_TRANSITION |
            SB2_FORCED_RELEASE |
            SB2_DEVICE_ATTR_TABLE_MODIFIED |
            SB2_VENDOR_ATTNS
        )) {
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
}

void write_control_bus(uint8_t v) {
    // flip the control bus to write

    // write the value

    // flip the control bus back to read
}