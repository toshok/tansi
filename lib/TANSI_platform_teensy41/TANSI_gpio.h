// GPIO definitions

#pragma once

// ANSI control bus
#define ANSI_CB0  1
#define ANSI_CB1  2
#define ANSI_CB2  3
#define ANSI_CB3  4
#define ANSI_CB4  5
#define ANSI_CB5  5
#define ANSI_CB6  6
#define ANSI_CB7  7

// signal pins with source = host
#define ANSI_SELECT_OUT_ATTN_IN_STROBE 9
#define ANSI_COMMAND_REQUEST           10
#define ANSI_PARAMETER_REQUEST         11
#define ANSI_BUS_DIRECTION_OUT         12
#define ANSI_PORT_ENABLE               13
#define ANSI_READ_GATE                 14
#define ANSI_WRITE_GATE                15

// signal pins with source = device
#define ANSI_BUS_ACKNOWLEDGE           16
#define ANSI_INDEX                     17
#define ANSI_SECTOR_MARK               18
#define ANSI_ATTENTION                 19
#define ANSI_BUSY                      20

// these pins represent differential NRZ signals that are converted
// to 0-1 by external hardware
#define ANS_READ_DATA                  21
#define ANS_READ_REF_CLOCK             22
#define ANS_WRITE_CLOCK                23
#define ANS_WRITE_DATA                 24

#define LED_ON()
#define LED_OFF()

#define SD_CONFIG BUILTIN_SDCARD