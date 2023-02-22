/*
 *
 *  Created on: 02.16.2023
 *      Author: Jonathan Armstrong
 */

//#include <SI_EFM8BB1_Register_Enums.h>
#include <string.h>
#include "..\inc\globals.h"
//#include "uart_0.h"
#include "..\inc\uart.h"
#include "..\inc\rf_handling.h"
#include "..\inc\rf_protocols.h"

//
// the original implementation from RF-Bridge-EFM8BB1 looks similar to this implementation
// https://github.com/hrshygoodness/EFM32-Library/blob/master/v2/an/an0820_efm32_smart_card/usart_driver.c
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
__xdata volatile uint8_t UART_RX_Buffer[UART_RX_BUFFER_SIZE];
__xdata volatile uint8_t UART_TX_Buffer[UART_TX_BUFFER_SIZE];

static volatile uint8_t UART_RX_Buffer_Position = 0;
static volatile uint8_t UART_TX_Buffer_Position = 0;
static volatile uint8_t UART_Buffer_Read_Position = 0;
static volatile uint8_t UART_Buffer_Write_Position = 0;
static volatile uint8_t UART_Buffer_Write_Len = 0;
static volatile uint8_t lastRxError;


//-----------------------------------------------------------------------------
// UART ISR Callbacks
//-----------------------------------------------------------------------------
void uart_receiveCompleteCb()
{
    
}

void uart_transmitCompleteCb()
{
    
}

int getchar(void)
{
    return SBUF;
}

void uart_write(uint8_t value)
{
	SBUF = value;
}

void uart_init_tx_polling()
{
  TI = 1;
}

//=========================================================
//=========================================================
void uart_isr(void) __interrupt (4)
{
    // FIXME: not sure if it is a good idea to clear both flags together
    // FIXME: need to define bit mask if we want to clear sfr this way
	// buffer and clear flags immediately so we don't miss an interrupt while processing
	const uint8_t flags = SCON & (0x01 | 0x02);
	SCON &= ~flags;
    // FIXME: this approach may be more portable because register address is not the same as a bit mask
    //const bool flagRI = RI;
    //const bool flagTI = TI;
    
    //RI = 0;
    //TI = 0;
        
	// receiving byte
	if (flags & 0x01)
	{        
        // store received data in buffer
    	UART_RX_Buffer[UART_RX_Buffer_Position] = getchar();
        UART_RX_Buffer_Position++;

        // set to beginning of buffer if end is reached
        if (UART_RX_Buffer_Position == UART_RX_BUFFER_SIZE)
        {
	    	UART_RX_Buffer_Position = 0;
        }
	}

	// transmit byte
	if (flags & 0x02)
	{
		if (UART_Buffer_Write_Len > 0)
		{
			UART0_write(UART_TX_Buffer[UART_Buffer_Write_Position]);
			UART_Buffer_Write_Position++;
			UART_Buffer_Write_Len--;
		}


		if (UART_Buffer_Write_Position == UART_TX_BUFFER_SIZE)
        {
			UART_Buffer_Write_Position = 0;
        }
	}
}

// FIXME: it seems like a bad idea to have a possible infinite loop
//   inside a function; better to return a flag?
//void uart_wait_until_TX_finished(void)
//{
//	while(!gTXFinished);
//}

//bool is_uart_tx_finished(void)
//{
//    return gTXFinished;
//}

bool is_uart_tx_buffer_empty(void)
{
    bool isBufferEmpty = true;
    
    if ( UART_Buffer_Write_Len > 0 )
    {
        isBufferEmpty = false;
    }
    
    return isBufferEmpty;
}

/*************************************************************************
Function: uart_getc()
Purpose:  return byte from ringbuffer
Returns:  lower byte:  received byte from ringbuffer
          higher byte: last receive error
**************************************************************************/
unsigned int uart_getc(void)
{
	unsigned int rxdata;

    if ( UART_Buffer_Read_Position == UART_RX_Buffer_Position )
    {
        // no data available
        return UART_NO_DATA;
    }

    // get data from receive buffer
    rxdata = UART_RX_Buffer[UART_Buffer_Read_Position];
    UART_Buffer_Read_Position++;

    if (UART_Buffer_Read_Position == UART_RX_BUFFER_SIZE)
    {
    	UART_Buffer_Read_Position = 0;
    }

    // FIXME: can not see where lastRxError is ever set?
    rxdata |= (lastRxError << 8);
    lastRxError = 0;
    
    return rxdata;
}

/*************************************************************************
Function: uart_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none
**************************************************************************/
void uart_putc(uint8_t txdata)
{
	if (UART_TX_Buffer_Position == UART_TX_BUFFER_SIZE)
    {
		UART_TX_Buffer_Position = 0;
    }

	UART_TX_Buffer[UART_TX_Buffer_Position] = txdata;
	UART_TX_Buffer_Position++;
	UART_Buffer_Write_Len++;
}

void uart_put_command(uint8_t command)
{
    // in other words 0xAA, sonoff convention maybe?
	uart_putc(RF_CODE_START);
    // defined here I think: https://github.com/Portisch/RF-Bridge-EFM8BB1/wiki/Commands
	uart_putc(command);
    // in other words 0x55
	uart_putc(RF_CODE_STOP);
    
    // triggers interrupt
	//uart_init_tx_polling();
}

void uart_put_rf_data_standard(const uint8_t command)
{
	uint8_t index = 0;
	uint8_t b = 0;

	uart_putc(RF_CODE_START);
	uart_putc(command);

	// sync low time
	uart_putc((gSyncLow >> 8) & 0xFF);
	uart_putc(gSyncLow & 0xFF);
	// bit 0 high time
	uart_putc((gBitLow >> 8) & 0xFF);
	uart_putc(gBitLow & 0xFF);
	// bit 1 high time
	uart_putc((gBitHigh >> 8) & 0xFF);
	uart_putc(gBitHigh & 0xFF);

	// copy data to UART buffer
	index = 0;
    
    // 24 bits total at eight bits each is three bytes
	while(index < 3)
	{
		uart_putc(gRFData[index]);
		index++;
	}
    
	uart_putc(RF_CODE_STOP);

	uart_init_tx_polling();
}

void uart_put_rf_data_advanced(const uint8_t command, const uint8_t protocol_index)
{
    // FIXME: explain what this variable is doing
	uint8_t index = 0;
    // FIXME: need a better variable name
	uint8_t b = 0;
	uint8_t bits = 0;

	uart_putc(RF_CODE_START);
	uart_putc(command);

    //
	bits = protocols[protocol_index].bit_count;

	while(index < bits)
	{
		index += 8;
		b++;
	}

	uart_putc(b+1);

	// send index of this protocol
	uart_putc(protocol_index);

	// copy data to UART buffer
	index = 0;
	while(index < b)
	{
		uart_putc(gRFData[index]);
		index++;
	}
    
	uart_putc(RF_CODE_STOP);

	uart_init_tx_polling();
}



#if INCLUDE_BUCKET_SNIFFING == 1
void uart_put_rf_buckets(const uint8_t command)
{
	uint8_t index = 0;

	uart_putc(RF_CODE_START);
	uart_putc(command);
	// put bucket count + sync bucket
	uart_putc(bucket_count + 1);

	// start and wait for transmit
	uart_init_tx_polling();
	uart_wait_until_TX_finished();

	// send up to 7 buckets
	while (index < bucket_count)
	{
		uart_putc((buckets[index] >> 8) & 0x7F);
		uart_putc(buckets[index] & 0xFF);
		index++;
	}

	// send sync bucket
	uart_putc((bucket_sync >> 8) & 0x7F);
	uart_putc(bucket_sync & 0xFF);

	// start and wait for transmit
	uart_init_tx_polling();
	uart_wait_until_TX_finished();

	index = 0;
	while(i < gActualByte)
	{
		uart_putc(gRFData[index]);
		index++;

        // FIXME: investigate if modulo divide might be using up lots of code space and/or ram
		// be safe to have no buffer overflow
		if ((index % UART_TX_BUFFER_SIZE) == 0)
		{
			// start and wait for transmit
			uart_init_tx_polling();
			uart_wait_until_TX_finished();
		}
	}

	uart_putc(RF_CODE_STOP);

	uart_init_tx_polling();
}
#endif
