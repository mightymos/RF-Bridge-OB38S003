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

#if !defined(TARGET_BOARD_EFM8BB1) && !defined(TARGET_BOARD_OB38S003) && !defined(TARGET_BOARD_EFM8BB1LCB)
	#error Please define TARGET_BOARD in makefile
#endif

// nop style delays
#include "delay.h"

// basically just function wrappers for setting pins (i.e., not a full hardware abstraction layer)
#include "hal.h"

//#if TARGET_BOARD_OB38S003
// sdcc does not have sfrs defined for this microcontroller, so must include them
//#include "OB38S003.h"
//#endif

#include <stdint.h>

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



//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
    // track elapsed time for doing something periodically (e.g., every 10 seconds)
    //unsigned long previousTimeSendRadio = 0;
    //unsigned long previousTimeHeartbeat = 0;
    //unsigned long elapsedTimeSendRadio;
    //unsigned long elapsedTimeHeartbeat;
    //unsigned long heartbeat = 0;
    
    // look for level transitions on pins rather than absolute level, so that we can count signal edges
    bool        rdataLevelOld;
    static bool rdataLevelNew = false;
    
    bool        tdataLevelOld;
    static bool tdataLevelNew = false;
    

    // hardware initialization
	set_clock_mode();

    init_port_pins();
    
    // set default pin levels
    led_off();
    buzzer_off();
    tdata_off();

    
    // enable radio receiver
    radio_receiver_on();
    
    //startup_beep();
    //startup_debug(stackStart);
    startup_blink();
    //startup_reset_status();
    
    // just to give some startup time
    delay1ms(500);
	
	// shows power is on
	led_on();
        
    // watchdog will force a reset, unless we periodically write to it, demonstrating loop is not stuck somewhere
    enable_watchdog();

    while (true)
    {

        // if this is not periodically called, watchdog will force microcontroller reset
        refresh_watchdog();
    
        // mirror radio pin levels to uart pins (used as gpio)
        // so that ESP8265 can effectively decode the same signals
        rdataLevelOld = rdataLevelNew;
        rdataLevelNew = rdata_level();
        
        // look for a level change (i.e., an edge)
        // explicitly setting pin output constantly, even with no change, is possibly a bad idea for some reason
        if (rdataLevelOld != rdataLevelNew)
        {
            // radio receiver
            if (rdataLevelNew)
            {
                uart_tx_pin_on();
            } else {
                uart_tx_pin_off();
            }
        }

        // FIXME: variable name may need to be changed (is actually uart rx pin level)
        tdataLevelOld = tdataLevelNew;
        tdataLevelNew = uart_rx_pin_level();
        
        if (tdataLevelOld != tdataLevelNew)
        {
            
            // radio transmitter
            if (tdataLevelNew)
            {
                tdata_on();
            } else {
                tdata_off();
            }
        }

		// FIXME: it would be nice if we could somehow still blink LED
		// when a suspected packet is being received
        //if (available())
        //{
        //}
    }
    
}
