#ifndef _SOFTWARE_UART_H
#define _SOFTWARE_UART_H

#include <stdbool.h>

//define baudrate const
// example formula was for STC15 8051 microcontroller with 24MHz / 3 = 8MHz max. clock at 3.3V ??
//BAUD = 65536 - FOSC/3/BAUDRATE/M (1T:M=1; 12T:M=12)
//NOTE: (FOSC/3/BAUDRATE) must be greater than 98, (RECOMMEND GREATER THAN 110)


#if defined(TARGET_MCU_OB38S003)

	// e.g, hex(65536 - (16000000/2400/1))
	#define SOFT_BAUD 0xE5F5

#elif defined(TARGET_MCU_EFM8BB1) || defined (TARGET_MCU_EFM8BB1LCB)

	// e.g., hex(65536 - (24500000/2400/12))
	#define SOFT_BAUD 0xFCAD

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

unsigned char uart_rx(bool* result);


#endif // _SOFTWARE_UART_H