//=========================================================
//=========================================================

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

// borrowed from area-8051 uni-stc HAL...
#include "delay.h"
#include "hal.h"

// FIXME: probably avoid this dependency later
#include "ob38s003_sfr.h"
//#include "rcswitch.h"
//#include "timer.h"
//#include "uart.h"
//#include "uart_software.h"


// DEBUG: bench sensor
//21:47:05.533 RSL: RESULT = {"Time":"2023-08-05T21:47:05","RfReceived":{"Sync":10280,"Low":340,"High":1010,"Data":"80650A","RfKey":"None"}}
//19:16:47.313 RSL: RESULT = {"Time":"2023-08-05T19:16:47","RfReceived":{"Sync":10280,"Low":340,"High":1010,"Data":"80650E","RfKey":"None"}}


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
    // look for level transitions on pins rather than absolute level, so that we can count signal edges
    bool        rdataLevelOld;
    static bool rdataLevelNew = false;
    
    bool        tdataLevelOld;
    static bool tdataLevelNew = false;
    

    // hardware initialization
    set_clock_1t_mode();
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
    
    // keep on permanently just to show something is working
    led_on();
    
    // just to give some startup time
    delay1ms(500);

        
    // watchdog will force a reset, unless we periodically write to it, demonstrating loop is not stuck somewhere
    enable_watchdog();

    while (true)
    {

        // if this is not periodically called, watchdog will force microcontroller reset
        refresh_watchdog();
    
    
        // mirror radio voltage levels to esp8265
        // mirror radio pin levels to uart pins (used as gpio)
        // so that esp8265 can effectively decode the same signals
        rdataLevelOld = rdataLevelNew;
        rdataLevelNew = rdata_level();
        
        // look for a level change (i.e., an edge)
        // because I am thinking expliciting setting pin output constantly, even with no change, is somehow a bad idea
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

        // FIXME: variable name may need to be changed
        //        (is actually uart rx pin level)
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
    }
    
}
