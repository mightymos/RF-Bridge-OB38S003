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
//#include <stdint.h>
//#include <stdlib.h>

// printf() requires a decent amount of code space and ram which we would like to avoid
// and printf is not particularly useful once packet format is used
#include <stdio.h>

// borrowed from area-8051 uni-stc HAL...
#include "delay.h"

// basically just function wrappers for setting pins etc
// NOT a fancy hardware abstraction layer
#include "hal.h"

// additional special function registers not present in standard 8051/8052
#include "OB38S003.h"

// the classic library for radio packet decoding
#include "rcswitch.h"

//
#include "state_machine.h"

// hardware specific
#include "timer.h"
#include "uart.h"

// since the uart pins are used for communication with ESP8265
// it is helpful to have serial output on another pin (e.g., reset pin)
#include "uart_software.h"


// sdccman sec. 3.8.1 indicates isr prototype must appear or be included in the file containing main
// millisecond tick count
//extern void timer0_isr(void) __interrupt (1);
// software uart
// FIXME: if reset pin is set to reset function, instead of gpio, does this interfere with anything (e.g., software serial?)
//extern void tm0(void)        __interrupt (d_T0_Vector);
extern void timer1_isr(void) __interrupt (d_T1_Vector);
extern void uart_isr(void)   __interrupt (d_UART0_Vector);
extern void timer2_isr(void) __interrupt (d_T2_Vector);


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

void startup_beep(void)
{
    // FIXME: startup beep helpful or annoying?
    buzzer_on();
    delay1ms(20);
    buzzer_off();   
}

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

// this can be pretty slow to blink out an eight bit reset register
void startup_reset_status(void)
{
    uint8_t index;
    
    for (index = 1; index <= RSTS; index++)
    {
        led_on();
        //reset_pin_on();
        delay1ms(1000);
        led_off();
        //reset_pin_off();
        delay1ms(1000);
    }
}


//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{

    // holdover from when we considered using rtos
    //const __idata unsigned char* stackStart = (__idata unsigned char*) get_stack_pointer() + 1;

    // have only tested decoding with two protocols so far
    const uint8_t repeats = 8;
    // FIXME: comment on what this does
    // lowest ID is 1
    const uint8_t protocolId = 1;
    
    // track elapsed time for doing something periodically (e.g., every 10 seconds)
    unsigned long previousTimeSendRadio = 0;
    unsigned long previousTimeHeartbeat = 0;
    unsigned long elapsedTimeSendRadio;
    unsigned long elapsedTimeHeartbeat;
    unsigned long heartbeat = 0;
    
    
    // upper eight bits hold error or no data flags
    unsigned int rxdata = UART_NO_DATA;
    

    // hardware initialization
    set_clock_1t_mode();
    init_port_pins();
    
    // set default pin levels
    led_off();
    buzzer_off();
    tdata_off();
    
    
    // setup hardware serial
    init_uart();
    uart_rx_enabled();
    
    // hardware serial interrupt
    init_serial_interrupt();
    
    // FIXME: consider reading pin state to select mirror mode or decoding mode
    // default state is reset high if using software uart
    //reset_pin_on();
    
    // software serial
    //init_software_uart();
    //enable_timer0();
        
    // provides one millisecond tick
    // (warning: cannot be used at the same time as software uart for now)
    //init_timer0();
        
    // provides ten microsecond tick
    init_timer1();
    
    // tick style functionality
    enable_timer1_interrupt();
    
    // timer supports compare and capture module for determining pulse lengths of received radio signals
    init_timer2_capture();

    // radio receiver edge detection
    init_capture_interrupt();
	
    // enable interrupts
    enable_global_interrupts();
 
    
    // enable radio receiver
    radio_receiver_on();
    
    //startup_beep();
    //startup_debug(stackStart);
    startup_blink();
    //startup_reset_status();
    
    // just to give some startup time
    delay1ms(500);

        
    // watchdog will force a reset, unless we periodically write to it, demonstrating loop is not stuck somewhere
    enable_watchdog();

    while (true)
    {

        // if this is not periodically called, watchdog will force microcontroller reset
        refresh_watchdog();
    
    

        // try to get one byte from uart rx buffer
        rxdata = uart_getc();

     
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
        if (rxdata != UART_NO_DATA)
        {
            uart_state_machine(rxdata);
        }
            

        if (available())
        {
            // FIXME: there must be a better way to lock
            // this is needed to avoid corrupting the currently received packet
            disable_capture_interrupt();
            
            // formatted for tasmota
            radio_decode_report();
            
            // DEBUG: formatted like rc-switch example
            //radio_decode_debug();
            
            // DEBUG:
            //radio_timings();
            
            led_toggle();

            reset_available();
            
            enable_capture_interrupt();
            
            // DEBUG: using software uart
            // FIXME: a little dangerous as-is because basically sits in a while() loop ?
            // protocol index
            //putc('p');
            //putc('x');
            //puthex2(get_received_protocol());
            //putc(' ');

            // bits received
            //putc('b');
            //putc('x');
            //puthex2(get_received_bitlength());
            //putc('\r');
            //putc('\n');
        }
        
        
#if 1
        // do a task like blink led about every ten seconds to show loop is alive
        elapsedTimeHeartbeat = get_elapsed_timer1(previousTimeHeartbeat);

        if (elapsedTimeHeartbeat >= 1000000)
        {
            // test software uart
            //puthex2(heartbeat);
            //putc('\r');
            //putc('\n');
            
            led_toggle();
            
            previousTimeHeartbeat = get_current_timer1();
            
            heartbeat++;
        }
        
#endif
        
     
     
#if 0
        // FIXME: future use for transmitting
        // FIXME: should we check to see if we are in the middle of receiving?
        
        // periodically send out a radio transmission
        elapsedTimeSendRadio = get_elapsed_timer1(previousTimeSendRadio);

        if (elapsedTimeSendRadio >= 30000)
        {
            // FIXME: not sure if we NEED to disable radio receiver but we probably should (to avoid loopback)
            //radio_receiver_off();
            
            led_toggle();
            
            // FIXME: do stuff
            
            //radio_receiver_on();
            
            previousTimeSendRadio = get_current_timer1();
        }
        
#endif 
      
        
        

    }
    
}
