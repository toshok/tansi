// GPIO definitions

#pragma once

// ANSI control bus is on port A
#define ANSI_CB_PORT PORTA
#define ANSI_CB_DDR DDRA
#define ANSI_CB_PIN PINA
#define ANSI_CB0  (1<<0)
#define ANSI_CB1  (1<<1)
#define ANSI_CB2  (1<<2)
#define ANSI_CB3  (1<<3)
#define ANSI_CB4  (1<<4)
#define ANSI_CB5  (1<<5)
#define ANSI_CB6  (1<<6)
#define ANSI_CB7  (1<<7)

// signal pins with source = host on port B
#define ANSI_OUT_PORT PORTB
#define ANSI_OUT_DDR DDRB
#define ANSI_OUT_PIN PINB
#define ANSI_SELECT_OUT_ATTN_IN_STROBE (1<<0)
#define ANSI_COMMAND_REQUEST           (1<<1)
#define ANSI_PARAMETER_REQUEST         (1<<2)
#define ANSI_BUS_DIRECTION             (1<<3)
#define ANSI_READ_GATE                 (1<<4)
#define ANSI_WRITE_GATE                (1<<5)

// signal pins with source = device on port C
#define ANSI_IN_PORT PORTC
#define ANSI_IN_DDR DDRC
#define ANSI_IN_PIN PINC
#define ANSI_BUS_ACKNOWLEDGE           (1<<0)
#define ANSI_INDEX                     (1<<1)
#define ANSI_SECTOR_MARK               (1<<2)
#define ANSI_ATTENTION                 (1<<3)
#define ANSI_BUSY                      (1<<4)

// these pins represent differential NRZ signals that are converted
// to 0-1 by external hardware
#define ANSI_DATA_PORT PORTD
#define ANSI_DATA_DDR DDRD
#define ANSI_DATA_PIN PIND
#define ANS_READ_DATA                  (1<<0)
#define ANS_READ_REF_CLOCK             (1<<1)
#define ANS_WRITE_CLOCK                (1<<2)
#define ANS_WRITE_DATA                 (1<<3)
