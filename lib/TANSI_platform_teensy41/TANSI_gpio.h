// GPIO definitions

#pragma once

// ANSI control bus
#define ANSI_CB0                       0
#define ANSI_CB1                       1
#define ANSI_CB2                       2
#define ANSI_CB3                       3
#define ANSI_CB4                       4
#define ANSI_CB5                       5
#define ANSI_CB6                       6
#define ANSI_CB7                       7

// unidirectional single ended source = host
#define ANSI_SELECT_OUT_ATTN_IN_STROBE 27
#define ANSI_COMMAND_REQUEST           28
#define ANSI_PARAMETER_REQUEST         29
#define ANSI_BUS_DIRECTION_OUT         30
#define ANSI_READ_GATE                 31
#define ANSI_WRITE_GATE                32

// unidirectional single ended source = device
#define ANSI_BUS_ACKNOWLEDGE           33
#define ANSI_INDEX                     34
#define ANSI_SECTOR_MARK               35
#define ANSI_ATTENTION                 36
#define ANSI_BUSY                      37


// port enable is special enough we'll keep it separate from everything else
#define ANSI_PORT_ENABLE               41

// these pins represent differential NRZ signals that are converted
// to 0-3.3v single ended by external hardware
#define ANSI_READ_DATA                 14
#define ANSI_READ_REF_CLOCK            15
#define ANSI_WRITE_CLOCK               16
#define ANSI_WRITE_DATA                17

#define LED_ON()  digitalWrite(LED_BUILTIN, HIGH)
#define LED_OFF() digitalWrite(LED_BUILTIN, LOW)

#define SET_ACTIVE(pin)   digitalWrite(pin, LOW)
#define SET_INACTIVE(pin) digitalWrite(pin, HIGH)


#define SD_CONFIG BUILTIN_SDCARD