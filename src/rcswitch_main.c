/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2024 Jonathan Armstrong. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#if !defined(TARGET_MCU_EFM8BB1) && !defined(TARGET_MCU_OB38S003) && !defined(TARGET_MCU_EFM8BB1LCB) && !defined(TARGET_MCU_EFM8BB52)
    #error Please define TARGET_MCU in makefile
#endif

// printf() requires a decent amount of code space and ram which we would like to avoid
// and printf is not particularly useful once packet format is used
// including will conflict with puts() in uart_software.c
//#include <stdio.h>

// borrowed from area-8051 uni-stc HAL...
#include "delay.h"

// basically just function wrappers for setting pins etc
// not really a complete hardware abstraction layer
#include "hal.h"


// the classic library for radio packet decoding
#include "rcswitch.h"

// similar to portisch commands
#include "state_machine.h"

// generic tick logic independent of controller
//#include "ticks.h"

// hardware specific
#include "timer_interrupts.h"

// ring buffer supported
#include "uart.h"


// since the uart pins are used for communication with ESP8265
// it is helpful to have serial output on another pin (e.g., reset pin)
//#include "uart_software.h"

// a rough way of setting a blink period
// set a threshold
// about every six seconds @ 24500000 MHz
//#define HEARTBEAT_THRESHOLD 0x80000


// sdccman sec. 3.8.1 indicates isr prototype must appear or be included in the file containing main
// it is probably more proper to achieve this through include files but also easy to omit
// and then things just will not work with no clear reason why, even though compilation is succcessful
#if defined(TARGET_MCU_OB38S003)
    // for software uart
    // FIXME: if reset pin is set to reset function, instead of gpio, does this interfere with anything (e.g., software serial?)
    //extern void tm0(void)        __interrupt (d_T0_Vector);
    // supports timeout
    extern void timer0_isr(void) __interrupt (d_T0_Vector);
    // pca like capture mode for radio decoding
    extern void timer2_isr(void) __interrupt (d_T2_Vector);
    // hardware uart
    extern void uart_isr(void)   __interrupt (d_UART0_Vector);

#elif defined(TARGET_MCU_EFM8BB1) || defined(TARGET_MCU_EFM8BB1LCB)

    // software uart
    //extern void tm0(void)        __interrupt (TIMER0_VECTOR);
    // supports timeout
    extern void timer2_isr(void) __interrupt (TIMER2_VECTOR);
    // hardware uart (uses timer 1)
    extern void uart_isr(void)   __interrupt (UART0_VECTOR);
    // radio decoding
    extern void pca0_isr(void)   __interrupt (PCA0_VECTOR);

    // unique ID is stored in xram (MSB at address 0xFF)
    //#define ID0_ADDR_RAM 0xFF
    //#define ID1_ADDR_RAM 0xFE
    //#define ID2_ADDR_RAM 0xFD
    //#define ID3_ADDR_RAM 0xFC

    // this will fail if we assign external ram to values which are initialized
    // and we really do not need the feature anyway
    //void startup_uid(void)
    //{
    //  puthex2(*((__xdata unsigned char*) ID0_ADDR_RAM));
    //  puthex2(*((__xdata unsigned char*) ID1_ADDR_RAM));
    //  puthex2(*((__xdata unsigned char*) ID2_ADDR_RAM));
    //  puthex2(*((__xdata unsigned char*) ID3_ADDR_RAM));
    //}
#elif defined(TARGET_MCU_EFM8BB52)
    // software uart
    //extern void tm0(void)        __interrupt (TIMER0_VECTOR);
    // supports timeout
    extern void timer2_isr(void) __interrupt (TIMER2_IRQn);
    // hardware uart (uses timer 1)
    extern void uart_isr(void)   __interrupt (UART0_IRQn);
    // radio decoding
    extern void pca0_isr(void)   __interrupt (PCA0_IRQn);
#else
    #error Please define TARGET_MCU in makefile
#endif

//-----------------------------------------------------------------------------
// FIXME: this is sometimes needed to initialize external ram, setup watch dog timer, etc.
//-----------------------------------------------------------------------------
void __sdcc_external_startup(void)
{

}


#if 0
    void startup_debug(const __idata unsigned char* stackStart)
    {
        // just demonstrate serial uart is working basically
        printf_fast("Startup...\r\n");
        
        printf_fast("Start of stack: %p\r\n", stackStart);
        
        //printf_fast("num. of protocols: %u\r\n", numProto);

        // DEBUG: demonstrates that we cannot write above SP (stack pointer)
        //*gStackStart       = 0x5a;
        //*(gStackStart + 1) = 0x5a;
        //printf_fast("gStackStart[%p]: 0x%02x\r\n", gStackStart,   *gStackStart);
        //printf_fast("gStackStart[%p]: 0x%02x\r\n", gStackStart+1, *(gStackStart + 1));
    }
#endif

// buzzer
void startup_beep(void)
{
    // FIXME: startup beep helpful or annoying?
    buzzer_on();
    delay1ms(20);
    buzzer_off();   
}


// led
void startup_blink(void)
{
    // double blink
    led_on();
    delay1ms(1000);
    led_off();
    
    led_on();
    delay1ms(1000);
    led_off();
}



//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
    // just track how many loops have transpired as a rough way of tracking time
    //uint32_t ticks = HEARTBEAT_THRESHOLD;
    
    
    // upper eight bits hold error or no data flags
    unsigned int rxdataWithFlags = UART_NO_DATA;
    
    // allows communication between uart state machine and radio state machine
    RF_COMMAND_T rfCommand;
    

    // hardware initialization
    set_clock_mode();
    
#if defined(TARGET_MCU_OB38S003)
    init_port_pins();
#elif defined(TARGET_MCU_EFM8BB1) || defined(TARGET_MCU_EFM8BB1LCB) || defined(TARGET_MCU_EFM8BB52)
    // the crossbar on this microcontroller makes initialization more complicated
    init_port_pins_for_serial();
#else
    #error Please define TARGET_MCU in makefile
#endif
    
    // set default pin levels
    led_off();
    buzzer_off();
    tdata_off();
    
// defined (or commented out) in project-defs.h
#if defined(DEBUG_PINS_ENABLED)

    // DEBUG:
    // on some boards, "debug pin" is actually buzzer so we do not want to use it for debugging unless buzzer has been removed
    debug_pin01_off();
    
#endif
    
    //
    startup_blink();
    delay1ms(500);
    
    // setup hardware serial
    // timer 1 is clock source for uart0 on efm8bb1
    // should call these after initializing port pins
    init_uart0();
    uart_rx_enabled();
    
    // hardware serial interrupt
    init_uart0_transmit_interrupt_flag();
    enable_serial_interrupt();
    
#if 0
    // software serial
    // default state is reset/pin1 high if using software uart as transmit pin
    soft_tx_pin_on();

    // allows use of a gpio to output text characters for debugging hardware uart is assigned for communicating with esp8285
    init_software_uart();
#endif

    
#if defined(TARGET_MCU_OB38S003)
    // timer 0 provides one millisecond tick or supports software uart
    // at various times during development timer 0 has been used to support software uart
    //init_timer0(SOFT_BAUD);
    
    // supports microseconds and milliseconds delays respectively
    init_timer0_8bit_autoreload();
    //init_timer1_8bit_autoreload();
    
    // timer 2 supports compare and capture module
    // for determining pulse lengths of received radio signals
    init_timer2_as_capture();
    
    //
    enable_timer0_interrupt();
    //enable_timer1_interrupt();
    //enable_timer2_interrupt();
#elif defined(TARGET_MCU_EFM8BB1) || defined(TARGET_MCU_EFM8BB1LCB) || defined(TARGET_MCU_EFM8BB52)
    // pca used timer0 in stock portisch (why?), however rcswitch can use dedicated pca counters
    // at various times during development timer 0 has been used to support software uart
    //init_timer0(SOFT_BAUD);
    // uart must use timer1 on this controller
    init_timer1_8bit_autoreload(TIMER1_UART0);
    timer1_run();
    
    // timer 2 is used on demand to produce delays (i.e., time enabled at start, wait, then timer stopped at overflow)
    // however, we initialize reload value when using delay so nothing to initialize at this step
    //init_timer2(TIMER2_RELOAD_10MICROS);
    
    // timer 3 is unused for now
    
    //enable_timer0_interrupt();
    //enable_timer1_interrupt();
    enable_timer2_interrupt();
    
    // there are two interrupts required enabled to perform edge capture
    enable_pca0_interrupt();
    
    // pca0 clock source was timer 0 on portisch
    // however we use system clock divided by 12 as source here
    pca0_init();
    pca0_run();
#endif
    

    // radio receiver edge detection
    enable_capture_interrupt();
    
    // enable interrupts
    enable_global_interrupts();
 
#if defined(TARGET_MCU_OB38S003)
    // FIXME: function empty on efm8bb1, because unknown if receiver has enable pin
    radio_receiver_on();
#endif
    
    //startup_beep();
    //startup_debug(stackStart);
    startup_blink();
    //startup_reset_status();
    
    // just to give some startup time
    delay1ms(500);

        
    // watchdog will force a reset, unless we periodically write to it, demonstrating loop is not stuck somewhere
    enable_watchdog();

#if 0
    // demonstrate software uart is working
    putstring("boot\r\n");
#endif


//#if defined(TARGET_MCU_EFM8BB1) || defined(TARGET_MCU_EFM8BB1LCB)
//  putstring("uid:");
//  startup_uid();
//  putstring("\r\n");
//#endif

    while (true)
    {

        // if this is not periodically called, watchdog will force microcontroller reset
        refresh_watchdog();
    

        // try to get one byte from uart rx buffer
        // otherwise, the flags will indicate no data
        rxdataWithFlags = uart_getc();

     
        // check if serial transmit buffer is empty
        if(!is_uart_tx_buffer_empty())
        {
            if (is_uart_tx_finished())
            {
                // if not empty, set transmit interrupt flag, which triggers actual transmission
                uart_init_tx_polling();
            }
        }
        

        // process serial receive data
        rfCommand = uart_state_machine(rxdataWithFlags);
        
        
        // it seems better to separate the state machine for the radio and uart
        rf_state_machine(rfCommand);
            

        if (available())
        {
            // FIXME: there must be a better way to lock
            // disable interrupt is needed to avoid corrupting the currently received packet
            disable_capture_interrupt();
            
            // formatted for tasmota for output to web interface
            radio_rfin();
            
            // causes the led to strobe for visual feedback as packet is being received
            led_toggle();
            
#if 0
            // DEBUG: using software uart
            // this is slow because of low baud rate, so prefer to exclude in normal operation
            radio_decode_debug();
#endif

            // clears received data
            reset_available();
            
            // FIXME: if we use software uart to send debug output, this will be slow to be re-enabled
            enable_capture_interrupt();
        }
        
        
#if 0
        // it would be nice to increment ticks instead and then track current minus previous
        // so that ticks may be used with multiple thresholds for various rough timing needs
        // or the alternative would be to repurpose or dual purpose a timer, however software uart has been very helpful for debugging
        // finally, it is probably not necessary to use multiple ticks
        ticks--;
        
        // compare to threshold
        if (ticks == 0)
        {
            // DEBUG
            //debug_pin01_toggle();
            
            led_toggle();
            
            // reset count
            ticks = HEARTBEAT_THRESHOLD;
        }
        
#endif
        
     

    }
    
}
