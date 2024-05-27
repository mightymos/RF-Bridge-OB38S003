#ifndef _SOFTWARE_UART_H
#define _SOFTWARE_UART_H

#include <stdbool.h>

#if TARGET_BOARD_EFM8BB1
#include "sonoffr20_pins.h"

// pin definitions
#define RXB DEBUG_PIN0
#define TXB DEBUG_PIN1
//#define BTN P3_2

//define baudrate const
// example formula was for STC15 8051 microcontroller with 24MHz / 3 = 8MHz max. clock at 3.3V ??
//BAUD = 65536 - FOSC/3/BAUDRATE/M (1T:M=1; 12T:M=12)
//NOTE: (FOSC/3/BAUDRATE) must be greater than 98, (RECOMMEND GREATER THAN 110)

// example for efm8bb1
// #define BAUD (65536 - (MCU_FREQ/2400/12))
// hex(65536 - (24500000/2400/12))
#define BAUD 0xFCAD

#endif

#if TARGET_BOARD_OB38S003
#include "sonoffr22_pins.h"

// pin definitions
#define RXB BUZZER
#define TXB RESET_PIN
//#define BTN P3_2

// example
// hex(65536 - (16000000/2400/1))
#define BAUD 0xE5F5

#endif



extern unsigned char TBUF, RBUF;
extern __bit TING,RING;
extern __bit TEND,REND;
extern unsigned char t, r;

// when indexing take care to apply wrap around equivalent to buffer size (e.g., buf[r & 0x0F] for size 16)
//extern unsigned char buf[8];


//-----------------------------------------
//initial UART module variable
void init_software_uart(void);

void putc(const char c);
void putstring(const char *s);

void puthex(unsigned char v);
void puthex2(const unsigned char x);

unsigned char uart_rx(bool* result);
void uart_loop_test(void);


#endif // _SOFTWARE_UART_H