/*
 *
 *  Created on: 02.16.2023
 *      Author: Jonathan Armstrong
 */

#include "hal.h"

#if defined(TARGET_MCU_OB38S003)
    // for access to SBUF definition
    #include "OB38S003.h"
#elif defined(TARGET_MCU_EFM8BB1) || defined(TARGET_MCU_EFM8BB1LCB)
    // these are just a difference in naming convention
    #define SBUF SBUF0
    #define SCON SCON0
#elif defined(TARGET_MCU_EFM8BB52)
    // this microcontroller does have two uarts
    #define SBUF SBUF0
    #define SCON SCON0
    #define TI SCON0_TI
#endif

#include "portisch_command_format.h"
#include "uart.h"

#include <stdint.h>


//
// the original implementation from RF-Bridge-EFM8BB1 looks similar to this implementation
// https://github.com/hrshygoodness/EFM32-Library/blob/master/v2/an/an0820_efm32_smart_card/usart_driver.c
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
__xdata volatile uint8_t UART_RX_Buffer[UART_RX_BUFFER_SIZE];
__xdata volatile uint8_t UART_TX_Buffer[UART_TX_BUFFER_SIZE];

__xdata static volatile uint8_t UART_RX_Buffer_Position = 0;
__xdata static volatile uint8_t UART_TX_Buffer_Position = 0;
__xdata static volatile uint8_t UART_Buffer_Read_Position = 0;
__xdata static volatile uint8_t UART_Buffer_Write_Position = 0;
__xdata static volatile uint8_t UART_Buffer_Write_Len = 0;

//__xdata static volatile uint8_t lastRxError;

// prefer to avoid storing in external ram to take advantage of bit addressable internal ram
static volatile bool gTXFinished = true;

//-----------------------------------------------------------------------------
// UART ISR Callbacks
//-----------------------------------------------------------------------------
// removed to save code space


void uart_write(const uint8_t value)
{
    SBUF = value;
}

// FIXME: this could probably be replaced with call to
// void init_serial_interrupt(void) which sets the same flag
void uart_init_tx_polling(void)
{
  TI = 1;
}

//=========================================================
//=========================================================
#if defined(TARGET_MCU_OB38S003)
void uart_isr(void) __interrupt (d_UART0_Vector)
#elif defined(TARGET_MCU_EFM8BB1) || defined(TARGET_MCU_EFM8BB1LCB)
void uart_isr(void) __interrupt (UART0_VECTOR)
#elif defined(TARGET_MCU_EFM8BB52)
void uart_isr(void) __interrupt (UART0_IRQn)
#endif
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
        UART_RX_Buffer[UART_RX_Buffer_Position] = SBUF;
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
            uart_write(UART_TX_Buffer[UART_Buffer_Write_Position]);
            UART_Buffer_Write_Position++;
            UART_Buffer_Write_Len--;
            
            gTXFinished = false;
        } else {
            gTXFinished = true;
        }


        if (UART_Buffer_Write_Position == UART_TX_BUFFER_SIZE)
        {
            UART_Buffer_Write_Position = 0;
        }
    }
}


// we avoid the wait until finished loop of original portisch
// but can achieve the same behavior with calling functions
bool is_uart_tx_finished(void)
{
    return gTXFinished;
}

bool is_uart_tx_buffer_empty(void)
{    
    if ( UART_Buffer_Write_Len > 0 )
    {
        return false;
    }
    
    return true;
}


//************************************************************************
//Function: uart_getc()
//Purpose:  return byte from ringbuffer
//Returns:  lower byte:  received byte from ringbuffer
//          higher byte: last receive error
//************************************************************************
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
    //rxdata |= (lastRxError << 8);
    //lastRxError = 0;
    
    return rxdata;
}

//************************************************************************
//Function: uart_putc()
//Purpose:  write byte to ringbuffer for transmitting via UART
//Input:    byte to be transmitted
//Returns:  none
//************************************************************************
void uart_putc(const uint8_t txdata)
{
    if (UART_TX_Buffer_Position == UART_TX_BUFFER_SIZE)
    {
        UART_TX_Buffer_Position = 0;
    }

    UART_TX_Buffer[UART_TX_Buffer_Position] = txdata;
    UART_TX_Buffer_Position++;
    UART_Buffer_Write_Len++;
}

void uart_put_command(const uint8_t command)
{
    // in other words 0xAA, sonoff convention maybe?
    uart_putc(RF_CODE_START);
    // defined here: https://github.com/Portisch/RF-Bridge-EFM8BB1/wiki/Commands
    uart_putc(command);
    // in other words 0x55
    uart_putc(RF_CODE_STOP);
}