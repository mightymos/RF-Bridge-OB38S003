/*
 * uart.h
 *
 *  Ported on: 02.17.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "globals.h"

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------
#define RF_CODE_START		0xAA
#define RF_CODE_STOP		0x55

#define UART_RX_BUFFER_SIZE 32
#define UART_TX_BUFFER_SIZE	32


// high byte error return code of uart_getc()
// Framing Error by UART
#define UART_FRAME_ERROR      0x1000
// Overrun condition by UART
#define UART_OVERRUN_ERROR    0x0800
// Parity Error by UART
#define UART_PARITY_ERROR     0x0400
// receive ringbuffer overflow
#define UART_BUFFER_OVERFLOW  0x0200
// no receive data available
#define UART_NO_DATA          0x0100

//-----------------------------------------------------------------------------
// global enums
//-----------------------------------------------------------------------------
typedef enum
{
	IDLE,
	SYNC_INIT,
	SYNC_FINISH,
	RECEIVE_LEN,
	RECEIVING,
	TRANSMIT,
	COMMAND,
    RF_FINISHED
} UART_STATE_T;

typedef enum
{
	NONE                       = 0x00,
	RF_CODE_ACK                = 0xA0,
	RF_CODE_LEARN              = 0xA1,
	RF_CODE_LEARN_KO           = 0xA2,
	RF_CODE_LEARN_OK           = 0xA3,
	RF_CODE_RFIN               = 0xA4,
	RF_CODE_RFOUT              = 0xA5,
	RF_CODE_SNIFFING_ON        = 0xA6,
	RF_CODE_SNIFFING_OFF       = 0xA7,
	RF_CODE_RFOUT_NEW          = 0xA8,
	RF_CODE_LEARN_NEW          = 0xA9,
	RF_CODE_LEARN_KO_NEW       = 0xAA,
	RF_CODE_LEARN_OK_NEW       = 0xAB,
	RF_CODE_RFOUT_BUCKET       = 0xB0,
	RF_CODE_SNIFFING_ON_BUCKET = 0xB1,
	RF_DO_BEEP                 = 0xC0,
    SINGLE_STEP_DEBUG          = 0xFE,
	RF_ALTERNATIVE_FIRMWARE    = 0xFF
} UART_COMMAND_T;


//-----------------------------------------------------------------------------
// public variables
//-----------------------------------------------------------------------------

// public prototypes
extern void uart_init_tx_polling(void);
extern bool is_uart_tx_buffer_empty(void);
extern unsigned int uart_getc(void);
extern void uart_putc(uint8_t txdata);
extern void uart_write(uint8_t value);
extern void uart_put_command(uint8_t command);


extern volatile bool gTXFinished;

#endif // INC_UART_H_
