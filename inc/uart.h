/*
 * uart.h
 *
 *  Ported on: 02.17.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include <stdbool.h>
#include <stdint.h>

// FIXME: explain choice of size
// portisch allocated TX=64 and RX=32
#define UART_RX_BUFFER_SIZE 64
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
// public variables
//-----------------------------------------------------------------------------

// public prototypes
extern void uart_init_tx_polling(void);
extern bool is_uart_tx_finished(void);
extern bool is_uart_tx_buffer_empty(void);
extern unsigned int uart_getc(void);
extern void uart_putc(const uint8_t txdata);
extern void uart_write(const uint8_t value);
extern void uart_put_command(const uint8_t command);


#endif // INC_UART_H_
