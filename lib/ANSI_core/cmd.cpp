#include <cstring>

#include "elapsedMillis.h"
#include "ansi.h"
#include "TANSI_log.h"

extern AnsiDev gAnsiDev;

static void load_attribute(uint8_t attribute_value);
static uint8_t report_attribute();

// extra bookkeeping for our time dependent (not immediate) commands
static void start_time_dependent_command(
    uint32_t durationMillis,
    void (*callback)() = nullptr
);
struct SeekParams {
    uint8_t cylinder_high;
    uint8_t cylinder_low;
};
static SeekParams gSeekParams;
static void finish_seek();
static void finish_rezero();

void ansi_execute_command()
{
    AnsiDev* dev = &gAnsiDev;
    uint8_t param_out = dev->param_out;

    switch (dev->cmd) {
        case ANSI_CMD_REPORT_ILLEGAL_COMMAND:
			dbgmsg("ansicmd REPORT_ILLEGAL_COMMAND");
            // This command shall force the Illegal Command Bit to be set in the
            // General Status Byte (see Section 4.4). The General Status Byte,
            // with the Illegal Command Bit equal to one, is returned to the host
            // by the Parameter Byte of the command sequence.
            dev->general_status |= GS_ILLEGAL_COMMAND;
            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_CLEAR_FAULT:
			dbgmsg("ansicmd CLEAR_FAULT");
            // This command shall cause all fault status bits of the selected
            // device to be reset, provided the fault condition has passed. If
            // the fault condition persits the appropriate status bit shall
            // continue to be equal to one. The General Status Byte, cleared of
            // previous fault status, shall be returned by the Parameter Byte of
            // the command sequence.
            // The Clear Fault Command shall also reset the Attention Condition
            // caused by the fault condition, again only if the fault condition no
            // longer exists.

            dev->general_status &= ~(
                GS_CONTROL_BUS_ERROR |
                GS_ILLEGAL_COMMAND |
                GS_ILLEGAL_PARAMETER
            );

            clear_sb1(SB1_SEEK_ERROR | SB1_RW_FAULT | SB1_POWER_FAULT | SB1_COMMAND_REJECT);

            set_attention_state(false);

            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_CLEAR_ATTENTION:
			dbgmsg("ansicmd CLEAR_ATTENTION");
            // This command shall cause the Attention Condition to be reset in the
            // selected device. The General Status Byte shall be returned by the
            // Parameter Byte of the command sequence.
            //
            // If the error or other condition that caused the Attention Condition
            // persists, the Attention Condition shall not be set again. If,
            // however, the condition is reset and the error reoccurs, the
            // Attention Condition shall be set again.
            dev->sense_byte_2 &= ~(
                SB2_INITIAL_STATE |
                SB2_READY_TRANSITION |
                SB2_DEVICE_ATTR_TABLE_MODIFIED
            );

			dev->general_status &= ~GS_NORMAL_COMPLETE;

            set_attention_state(false);

            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_SEEK:
			dbgmsg("ansicmd SEEK");
            // This command shall cause the selected device to seek to the
            // cylinder identified as the target cylinder by the Load Cylinder
            // Address Commands (see Sections 4.1.3 and 4.1.4). The General
            // Status Byte shall be returned to the host by the Parameter Byte of
            // the command sequence with the Busy Executing bit set (see Section
            // 4.4.1.7).
            // The Seek Command shall set the Attention Condition and the Illegal
            // Parameter Bit in the General Status Byte if the target cylinder
            // address is outside the cylinder address range of the device.
            // Upon completion of any seek (including a zero length seek) the
            // device shall clear the Busy Executing bit in the General Status
            // Byte and set the Attention Condition.

            gSeekParams.cylinder_high = dev->load_cylinder_high;
            gSeekParams.cylinder_low = dev->load_cylinder_low;
            start_time_dependent_command(
                5, // 5ms.  look up this timing...
                finish_seek
            );

            dev->general_status |= GS_BUSY_EXECUTING;
            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_REZERO:
			dbgmsg("ansicmd REZERO");
            // This command shall cause the selected device to position the moving
            // head(s) over cylinder zero. The General Status byte shall be
            // returned to the host by the Parameter Byte of the command sequence
            // with the Busy Executing bit set (see Section 4.4.1.7).
            //
            // Upon the completion of the positioning of the moving head(s) over
            // cylinder zero the device shall clear the Busy Executing bit in the
            // General Status byte and set the Attention Condition.

            start_time_dependent_command(
                5, // 5ms.  look up this timing...
                finish_rezero
            );

            dev->general_status |= GS_BUSY_EXECUTING;
            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_REPORT_SENSE_BYTE_2:
			dbgmsg("ansicmd REPORT_SENSE_BYTE_2");
            // The command shall cause the selected device to return Sense Byte 2
            // by the Parameter Byte of the command sequence. No other action
            // shall be taken in the device.
            dev->param_in = dev->sense_byte_2;
            return;

        case ANSI_CMD_REPORT_SENSE_BYTE_1:
			dbgmsg("ansicmd REPORT_SENSE_BYTE_1");
            // This command shall cause the selected device to return Sense Byte 1
            // by the Parameter Byte of the command sequence. No other action
            // shall be taken in the device.
            dev->param_in = dev->sense_byte_1;
            return;

        case ANSI_CMD_REPORT_GENERAL_STATUS:
			dbgmsg("ansicmd REPORT_GENERAL_STATUS");
            // This command shall cause the selected device to return the general
            // Status Byte by the Parameter Byte of the command sequence. This
            // command shall not perform any other function in the device and acts
            // as a "no-op" in order to allow the host to monitor the device's
            // General Status Byte without changing any device condition.
            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_REPORT_ATTRIBUTE:
			dbgmsg("ansicmd REPORT_ATTRIBUTE");
            // This command shall cause the selected device to return a byte of
            // information that is the Device Attribute whose number was defined
            // in the Load Attribute Number Command (see Section 4.1.6). The
            // contents of the byte is defined by Table 4-3 and Section 4.3.
            dbgmsg("    attribute =", dev->attribute_number);
            dev->param_in = report_attribute();
            return;

        case ANSI_CMD_SET_ATTENTION:
			dbgmsg("ansicmd SET_ATTENTION");
            // This command shall cause the selected device to set the Attention
            // Condition. No other action shall be caused.
            // The General Status Byte shall be transferred to the host by the
            // Parameter Byte of the command sequence.

            start_time_dependent_command(
                5 // 5ms.  look up this timing...
                // no callback yet
            );
            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_SELECTIVE_RESET:
			dbgmsg("ansicmd SELECTIVE_RESET");
            // This command shall cause the selected device to reach Initial State
            // (see Section 3.2.1). This is a time dependent command and as such
            // shall set the Busy Executing bit prior to the assertion of the
            // acknowledge to parameter request and shall be reflected in the
            // returned General Status Byte. Upon completion of.the parameter
            // byte transfer the device shall go to the initial state and all
            // resetable parameter. attentions. errors. etc •• shall be reset.
            // When the initial state is reached bit 0 of Sense Byte 2 will be set
            // and bit 6 of the General Status Byte shall be cleared. (This
            // causes the setting of the Attention Condition).
            //
            // TODO
            dbgmsg("ANSI_CMD_SELECTIVE_RESET unimplemented");

            start_time_dependent_command(
                5 // 5ms.  look up this timing...
                // no callback yet
            );

            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_REFORMAT_TRACK:
			dbgmsg("ansicmd REFORMAT_TRACK");
            // This command shall cause the selected device to reconfigure the
            // arrangement of Sector Pulse generation according to parameters
            // received via the Load Sector Pulses Per Track Commands (see
            // Sections 4.1.9 to 4.1.11). The General Status Byte shall be
            // returned to the host by the Parameter Byte of the command
            // sequence.
            // The Partition Track Command is a Time Dependent Command and as such
            // shall set the Busy Executing bit in the General Status Byte
            // returned by this command (see Section 4.4.1.7) and it is to remain
            // set while this command execution is in process. Also. the device
            // shall exercise appropriate control over the Busy signal at the
            // interface (see Section 3.2.7).
            // Upon the completion of execution of this command the Bytes Per
            // Sector and the Sector Per Track will be updated in the Attribute
            // Table and also bit 6 of Attribute byte OE Hex will be cleared and
            // this shall set the Attention Condition.
            // The Partition Track Command shall set the Attention Condition and
            // the Illegal Parameter Bit in the General Status Byte if the Sector
            // Pulses Per Track create a set that is outside the range of the
            // device.
            // activating Read Gate or Write Gate while this command is executing
            // is a violation of protocol.
            //
            // TODO
            dbgmsg("ANSI_CMD_REFORMAT_TRACK unimplemented");
            start_time_dependent_command(
                5 // 5ms.  look up this timing...
                // no callback yet
            );
            dev->general_status |= GS_BUSY_EXECUTING;
            dev->param_in = dev->general_status;
            return;

        case ANSI_CMD_REPORT_CYL_ADDR_HIGH:
			dbgmsg("ansicmd REPORT_CYL_ADDR_HIGH");
            // This command shall cause the selected device to return a byte of
            // information that is the most significant byte of a 16 bit .number
            // that, indicates the cylinder address of the current position of the
            // moving heads. This number shall not reflect the most recent
            // cylinder address set by the Set Cylinder Address Commands (see
            // Sections 4.1.3 and 4.1.4) ,unless there has been an intervening Seek
            // Command completed (see Section 4.2.4).
            // If executed during a seek operation, the information returned shall
            // be ascertained by the vendor specification.
            // The information shall be transferred by the Parameter Byte of the
            // command sequence.
            dbgmsg("    CYL_ADDR_HIGH", dev->current_cylinder_high);
            dev->param_in = dev->current_cylinder_high;
            return;

        case ANSI_CMD_REPORT_CYL_ADDR_LOW:
			dbgmsg("ansicmd REPORT_CYL_ADDR_LOW");
            // This command shall cause the selected device to return a byte of
            // information that is the least significant byte of a 16 bit number
            // that indicates the cylinder address of the current position of the
            // moving heads. This number shall not reflect the most recent
            // cylinder address set by the Set Cylinder Address Commands (see
            // Sections 4.1.3 and 4.1.4) unless there has been an intervening Seek
            // Command completed (see Section 4.2.4).
            // If executed during a seek operation, the information returned shall
            // be ascertained by the vendor specification.
            // The information shall be transferred by the Parameter Byte of the
            // command sequence.
            //
            dbgmsg("    CYL_ADDR_LOW", dev->current_cylinder_low);
            dev->param_in = dev->current_cylinder_low;
            return;

        case ANSI_CMD_REPORT_TEST_BYTE:
			dbgmsg("ansicmd REPORT_TEST_BYTE");
            // This command shall cause the selected device to return a copy of
            // the Test Byte transferred to the device via the Load 'Test Byte
            // Command. (See Section 4.1.12.)
            // The Test Byte shall be transferred by the Parameter Byte of the
            // command sequence.
            //
            dbgmsg("ANSI_CMD_REPORT_TEST_BYTE", dev->test_byte);
            dev->param_in = dev->test_byte;
            return;

        case ANSI_CMD_ATTENTION_CONTROL:
			dbgmsg("ansicmd ATTENTION_CONTROL");
            // This command shall condition the selected device to enable or
            // disable its attention circuitry based on the value of the Parameter
            // Byte as shown below.
            // 7 6 5 4 3 2 1 0
            // | o o o o o o o
            // |
            // o - Enable Attention
            // 1 - Disable Attention
            //
            // This command allows the host to selectively ignore attention
            // requests from certain devices on the interface. This might be done
            // in response to a device that generates spurious attention requests
            // due to a malfuntion.
            // The Enable Attention Command shall cause the selected device to
            // gate its internal Attention Condition onto the party line ("wired
            // OR") Attention Signal. The Disable Attention Command shall cause
            // the selected device to disable the gating of the internal Attention
            // Condition onto the party line Attention Signal. This command shall
            // have no impact on the function of the radial status returned with
            // the Attention In Strobe Signal (see Signal 3.2.3.2).
            // Devices shall be initilized with the Attention circuitry enabled.
            //
            dev->attention_enabled = (param_out & 0x80) ? false : true;
            return;

        case ANSI_CMD_WRITE_CONTROL:
			dbgmsg("ansicmd WRITE_CONTROL");
            // This command shall condition the selected device to enable or
            // disable its write circuitry based on the value of the parameter
            // Byte as shown below:
            // 7 6 5 4 3 2 1 0
            // | o o o o o o o
            // |
            // 1 - Write Enable
            // o - Write Disable
            //
            // This command is used in conjunction with the Write Gate Signal and
            // therefore merely enables the write circuitry while the Write Gate
            // Signal activates the circuitry at the proper time. An active Write
            // Gate Signal while the device's write circuitry is disabled shall
            // result in no data being recorded.
            // Devices shall be initialized with the write circuitry disabled.
            // A Write Control Command execute during a write operation is a
            // violation of protocol.
            //
            dev->write_enabled = (param_out & 0x80) != 0;
            return;

        case ANSI_CMD_LOAD_CYL_ADDR_HIGH:
			dbgmsg("ansicmd LOAD_CYL_ADDR_HIGH");
            // This command shall condition the selected device to accept the
            // Parameter Byte as the most significant Byte of a cylinder address.
            // This command is used in conjunction with the Seek Command (see
            // Section 4.2.4) and therefore is a means of supplying the most
            // significant byte of a target cylinder address.
            // This command shall not cause any head motion. Loading a cylinder
            // address outside the range of a device shall not cause an error
            // unless a subsequent Seek Command is issued to that illegal
            // cylinder.
            // Devices shall be initialized with the target cylinder address equal
            // to zero.
            //
            dbgmsg("  cyl_addr_high", param_out);
			dev->load_cylinder_high = param_out;
            return;

        case ANSI_CMD_LOAD_CYL_ADDR_LOW:
			dbgmsg("ansicmd LOAD_CYL_ADDR_LOW");
            // This command shall condition the selected device to accept the
            // Parameter Byte as the least signficant byte of a cylinder address.
            // This command is used in conjunction with the Seek Command (see
            // Section 4.2.4) and therefore is a means of supplying the least
            // significant byte of a target cylinder address.
            // This command shall not cause any head motion. Loading a cylinder
            // address outside the range of a device shall not cause an error
            // unless a subsequent seek command is issued to that illegal
            // cylinder.
            // Devices shall be initialized with the target cylinder address equal
            // to zero.
            //
            dbgmsg("ANSI_CMD_LOAD_CYL_ADDR_LOW", param_out);
			dev->load_cylinder_low = param_out;
            return;

        case ANSI_CMD_SELECT_HEAD:
			dbgmsg("ansicmd SELECT_HEAD");
            // This command shall condition the selected device to. accept the
            // Parameter Byte as the binary address of the head selected for read
            // or write operations. This command shall enable the moving heads
            // and shall disable the fixed heads.
            // A Select Moving Head Command issued during a read or write
            // operation is a violation of protocol.
            // The device shall set the Attention Condition and the Illegal
            // Parameter Bit in the General Status Byte upon receipt of a head
            // address outside the head address range of the device.
            // Devices shall be initialized with moving head zero selected.
            dev->selected_head = param_out;
            return;

        case ANSI_CMD_LOAD_ATTRIBUTE_NUMBER:
			dbgmsg("ansicmd LOAD_ATTRIBUTE_NUMBER", param_out);
            // This command shall condition the selected device to accept the
            // Parameter Byte as the number of a Device Attribute as defined in
            // Table 4-3. This command prepares the device for a subsequent Load
            // Device Attribute Command or Report Device Attribute Command (see
            // Sections 4.1.7 and 4.2.9). This command may be issued at any time.
			dev->attribute_number = param_out;
            return;

        case ANSI_CMD_LOAD_ATTRIBUTE:
			dbgmsg("ansicmd LOAD_ATTRIBUTE", dev->attribute_number);
            // This command shall condition the selected device to accept the
            // Parameter Byte as the new value of a Device Attribute. The number
            // of the Device Attribute must have been previously defined by the
            // Load Attribute Number Command (see Section 4.1.6).
            dbgmsg("    value=", param_out);
            load_attribute(param_out);
			dbgmsg("    done");
            return;

        case ANSI_CMD_SPIN_CONTROL:
			dbgmsg("ansicmd SPIN_CONTROL");
            // This command shall condition the seleted device to enter a spin up
            // or spin down cycle based on the value of the Parameter Byte as
            // shown below.
            // 7 6 5 4 3 2 1 0
            // | o o o o o o o
            // |
            // 1 - Spin Up
            // o - Spin Down
            // A spin up cycle shall consist of starting the rotation of the
            // spindle. A spin down cycle shall consist of stopping the rotation
            // of the spindle.
            // Upon completion of a spin control cycle the device shall set the
            // Attention Condition. Issuing a spin up command to a device whose
            // spindle is already at full speed or issuing a spin down command to
            // a device whose spindle has already stopped shall also set the
            // Attention Condition.
            // The Spin Control Command is a Time Dependent Command and as such
            // shall set the Busy Executing bit in the General Status Byte (see
            // Section 4.4.1.7) while command execution is in process. Also, the
            // device shall exercise appropriate control over the Busy signal at
            // the interface (see Section 3.2.7).
            // A spin down cycle shall cause the repositioning of the moving
            // head(s) over the landing zone and stop the rotation of the
            // spindle. If the device detects that it cannot successfully seek to
            // the landing zone it shall set the Attention Condition and set bit 0
            // of Sense Byte 1.
            // See vendor specification for initial state of the Spin Control.

            start_time_dependent_command(
                10 // 10ms.  look up this timing...
                // no callback for the time being
            );
            return;

        case ANSI_CMD_LOAD_SECT_PER_TRACK_HIGH:
            dbgmsg("ansicmd LOAD_SECT_PER_TRACK_HIGH unimplemented");
            return;

        case ANSI_CMD_LOAD_SECT_PER_TRACK_MEDIUM:
		    dbgmsg("ansicmd LOAD_SECT_PER_TRACK_MEDIUM unimplemented");
            return;

        case ANSI_CMD_LOAD_SECT_PER_TRACK_LOW:
		    dbgmsg("ansicmd LOAD_SECT_PER_TRACK_LOW unimplemented");
            return;

        case ANSI_CMD_LOAD_BYTES_PER_SECT_HIGH:
		    dbgmsg("ansicmd LOAD_BYTES_PER_SECT_HIGH unimplemented");
            return;

        case ANSI_CMD_LOAD_BYTES_PER_SECT_MEDIUM:
		    dbgmsg("ansicmd LOAD_BYTES_PER_SECT_MEDIUM unimplemented");
            return;

        case ANSI_CMD_LOAD_BYTES_PER_SECT_LOW:
		    dbgmsg("ansicmd LOAD_BYTES_PER_SECT_LOW unimplemented");
            return;

        case ANSI_CMD_LOAD_READ_PERMIT_HIGH:
		    dbgmsg("ansicmd LOAD_READ_PERMIT_HIGH unimplemented");
            return;

        case ANSI_CMD_LOAD_READ_PERMIT_LOW:
		    dbgmsg("ansicmd LOAD_READ_PERMIT_LOW unimplemented");
            return;

        case ANSI_CMD_LOAD_WRITE_PERMIT_HIGH:
		    dbgmsg("ansicmd LOAD_WRITE_PERMIT_HIGH unimplemented");
            return;

        case ANSI_CMD_LOAD_WRITE_PERMIT_LOW:
		    dbgmsg("ansicmd LOAD_WRITE_PERMIT_LOW unimplemented");
            return;

        case ANSI_CMD_LOAD_TEST_BYTE:
		    dbgmsg("ansicmd LOAD_TEST_BYTE", param_out);
			dev->test_byte = param_out;
			return;

        default:
            dbgmsg("unknown ANSI command", dev->cmd);
            return;
    }
}

enum DeviceTypeId {
    NonRemovableDisk = 0x01,
    RemovableDisk = 0x02,
};

static void initialize_attributes()
{
    AnsiDev* dev = &gAnsiDev;
    /* ensure our attributes have been initialized */
    if (!dev->attributes_initialized) {
        dev->attributes_initialized = true;
        memset(dev->attributes, 0, sizeof(dev->attributes));

        dev->attributes[0x00] = 0x00;          // User ID - user defined
	    dev->attributes[0x01] = dev->disk_type->model_id >> 8;   // Model ID High - vendor defined
        dev->attributes[0x02] = dev->disk_type->model_id & 0xff; // Model ID Low - vendor defined
        dev->attributes[0x03] = 0x00;          // Revision ID - vendor defined

	    dev->attributes[0x0D] = DeviceTypeId::NonRemovableDisk; // Device Type ID - device dependent
	    dev->attributes[0x0E] = 0x00; // Table Modification - action dependent
	    dev->attributes[0x0F] = 0x00; // Table ID - vendor defined

        uint32_t bytes_per_track = dev->disk_type->sectors * HARD_DISK_SECTOR_SIZE;
	    dev->attributes[0x10] = (bytes_per_track >> 16) & 0xff;       // MSB of # of bytes per track
	    dev->attributes[0x11] = (bytes_per_track >> 8) & 0xff;        // MedSB of # of bytes per track
	    dev->attributes[0x12] = bytes_per_track & 0xff;               // LSB of # of bytes per track
	    dev->attributes[0x13] = (HARD_DISK_SECTOR_SIZE >> 16) & 0xff; // MSB of # of bytes per sector
	    dev->attributes[0x14] = (HARD_DISK_SECTOR_SIZE >> 8) & 0xff;  // MedSB of # of bytes per sector
	    dev->attributes[0x15] = HARD_DISK_SECTOR_SIZE & 0xff;         // LSB of # of bytes per sector
	    dev->attributes[0x16] = 0x00;                                 // MSB of # of sector pulses per track
	    dev->attributes[0x17] = dev->disk_type->sectors >> 8;                       // MedSB of # of sector pulses per track
	    dev->attributes[0x18] = dev->disk_type->sectors & 0xff;                     // LSB of # of sector pulses per track
	    dev->attributes[0x19] = 0x00;                                 // Sectoring method

	    dev->attributes[0x20] = dev->disk_type->cylinders >> 8;              // MSB of # of cylinders
	    dev->attributes[0x21] = dev->disk_type->cylinders & 0xff;            // LSB of # of cylinders
	    dev->attributes[0x22] = dev->disk_type->heads;                       // Number of heads

	    dev->attributes[0x30] = 0x00;                          // Encoding method #1
	    dev->attributes[0x31] = 0x00;                          // Preamble #1 number of bytes
	    dev->attributes[0x32] = 0x00;                          // Preamble #1 pattern
	    dev->attributes[0x33] = 0x00;                          // Sync #1 pattern
	    dev->attributes[0x34] = 0x00;                          // Postamble #1 number of bytes
	    dev->attributes[0x35] = 0x00;                          // Postamble #1 pattern
	    dev->attributes[0x36] = 0x00;                          // Gap #1 number of bytes
	    dev->attributes[0x37] = 0x00;                          // Gap #1 pattern

	    dev->attributes[0x40] = 0x00;                          // Encoding Method #2
	    dev->attributes[0x41] = 0x00;                          // Preamble #2 number of bytes
	    dev->attributes[0x42] = 0x00;                          // Preamble #2 pattern
	    dev->attributes[0x43] = 0x00;                          // Sync #2 pattern
	    dev->attributes[0x44] = 0x00;                          // Postamble #2 number of bytes
	    dev->attributes[0x45] = 0x00;                          // Postamble #2 pattern
	    dev->attributes[0x46] = 0x00;                          // Gap #2 number of bytes
	    dev->attributes[0x47] = 0x00;                          // Gap #2 pattern
    }
}

static void load_attribute(uint8_t attribute_value)
{
    initialize_attributes();
    gAnsiDev.attributes[gAnsiDev.attribute_number] = attribute_value;
}

static uint8_t report_attribute()
{
    initialize_attributes();
    return gAnsiDev.attributes[gAnsiDev.attribute_number];
}

static void finish_seek()
{
    AnsiDev* dev = &gAnsiDev;
    dev->current_cylinder_high = gSeekParams.cylinder_high;
    dev->current_cylinder_low = gSeekParams.cylinder_low;
}

static void finish_rezero()
{
    AnsiDev* dev = &gAnsiDev;
    dev->current_cylinder_high = 0;
    dev->current_cylinder_low = 0;
}

// globals to store pointers to the callback and params for the time dependent command
elapsedMillis gTimeDependentElapsedMillis;
uint32_t gTimeDependentDurationMillis;
void (*gTimeDependentCallback)();

static void start_time_dependent_command(
    uint32_t durationMillis,
    void (*callback)()
) {
    gTimeDependentCallback = callback;
    gTimeDependentDurationMillis = durationMillis;
    gTimeDependentElapsedMillis = 0;
}

// returns true if the time dependent command is still executing (based on the elapsed time.)
// if the command is done executing, the callback is invoked and the function returns false.
bool ansi_poll_time_dependent() {
    if (gTimeDependentElapsedMillis >= gTimeDependentDurationMillis) {
        if (gTimeDependentCallback) {
            gTimeDependentCallback();
        }
        return false;
    }
    return true;
}
