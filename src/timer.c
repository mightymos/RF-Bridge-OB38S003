#include <stdint.h>
#include <stdlib.h>

#include "..\inc\globals.h"
#include "..\inc\rf_protocols.h"
#include "..\inc\timer.h"

uint16_t gTimer2Timeout;
uint16_t gTimer2Interval;
//uint16_t gTimer3Timeout;
//uint16_t gTimer3Interval;

// rc-switch protocol 1
const uint16_t timings [] = { 350, 1050, 10850 };

__xdata struct RADIO_PACKET_T gPacket;

volatile bool gSingleStep = true;

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (5)
{
    static uint8_t lowByteOld  = 0;
    static uint8_t highByteOld = 0;
    static uint8_t lowByteNew  = 0;
    static uint8_t highByteNew = 0;
    
    static uint8_t count = 0;
    
    static volatile uint16_t previous = 0;
    static volatile uint16_t current = 0;
    unsigned long diff;
    
    // DEBUG: output pulses to compare to radio DO pin on oscilloscope (should match)
    // check if rising or falling edge
    //if (is_rdata_low())
    //{
    //    // FIXME: why is this inverted?
    //    uart_tx_pin_on();
    //} else {
    //    uart_tx_pin_off();
    //}

    // we are always looking for pulse duration (i.e., difference), so need to store previous and new values
    lowByteOld  = lowByteNew;
    highByteOld = highByteNew;
    
    // this stores timer 2 value without stopping timer 2
    lowByteNew  = CCL1;
    highByteNew = CCH1;
    
    // go from 8-bit to 16-bit variables
    previous = current;
    current = (highByteNew << 8) | lowByteNew;
    
    
    // FIXME: must use absolute value in case of timer overflow condition
    //   is this true (for this situation)?
    diff = abs(current - previous);
    
    // FIXME: no magic numbers
    // with timer clocked at (16 MHz / 4), four counts are needed to get one microsecond
    //diff = diff / 4;
    // 16 MHz / 12
    // e.g., (1/(16000000/24)) * dec(0xFFFF) = 98.30 milliseconds maximum can be counted
    diff = (diff * 3) / 2;
    
    // store sync durations and bit durations in array
    //gDuration[count] = diff;
    
    // DEBUG:
    // if (gSingleStep)
    // {
        // if (!is_rdata_low() && (gPacket.count == 0))
        // {
            // reset_pin_on();
            
            // gPacket.syncFirstDuration = current;
            // gPacket.count = 1;
            
        // } else if (is_rdata_low() && (gPacket.count == 1)) {
            // reset_pin_off();
            
            // gPacket.syncSecondDuration = current;
            // gPacket.count = 2;
            // gPacket.diff = diff;
            
            // gSingleStep = false;
        // } else {
            // reset_pin_off();
            // gPacket.syncFirstDuration  = 0;
            // gPacket.syncSecondDuration = 0;
            // gPacket.count = 0;
        // }
        
    // }
    
    // DEBUG:
    //gPacket.count = count;
    
    if (count == 0)
    {
        if ( abs(diff - timings[0]) < TOLERANCE_MIN )
        {
            //DEBUG:
            //led_toggle();
            //DEBUG:
            reset_pin_on();
            gPacket.syncFirstDuration = diff;
            gPacket.syncFirst = true;
            count = 1;
        } else {
            //DEBUG:
            reset_pin_off();
        }
    } else if (count == 1) {
        if ( abs(diff - timings[2]) < TOLERANCE_MAX )
        {
            //DEBUG:
            led_toggle();
            reset_pin_off();
            gPacket.syncSecondDuration = diff;
            gPacket.syncSecond = true;
            count = 2;
        } else {
            //DEBUG:
            reset_pin_off();
            count = 0;
        }
    } else if (count >= 2 && count < MAX_COUNT) {
        gPacket.duration[count - 2] = diff;
        count++;
        
        
        if ( abs(diff - timings[0]) < TOLERANCE_MIN && abs(diff - timings[1]) < TOLERANCE_MIN )
        {
            //DEBUG:
            reset_pin_on();
        } else {
            if ( abs(diff - timings[1]) < TOLERANCE_MIN && abs(diff - timings[0]) < TOLERANCE_MIN )
            {
                //DEBUG:
                reset_pin_off();
            } else {
                //DEBUG:
                reset_pin_off();
                
                count = 0;
            }
        }
        
        if (count >= MAX_COUNT)
        {
            //DEBUG:
            //printf("count: %d\r\n", count);
            gPacket.captureDone = true;
            count = 0;
        }
    }
    
    // store durations in array
    //gDuration[count++] = diff;
    
    
    //clear compare/capture 1 flag
    CCCON &= ~0x02;
}