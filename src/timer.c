#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "..\inc\globals.h"
#include "..\inc\rf_protocols.h"
#include "..\inc\timer.h"

uint16_t gTimer2Timeout;
uint16_t gTimer2Interval;
//uint16_t gTimer3Timeout;
//uint16_t gTimer3Interval;



__xdata struct RADIO_PACKET_T gPacket;

volatile bool gSingleStep = true;

// rc-switch protocol 1
const uint16_t gTimings [] = { 350, 1050, 10850 };
//const uint16_t gTimings [] = { 1000, 1000, 1000 };

// TODO:
typedef enum {
    START_HIGH_STATE = 0,
    START_LOW_STATE  = 1,
    DATA_STATE = 2,
} STATE_TYPE;

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (5)
{
    uint8_t lowByteOld;
    uint8_t highByteOld;
    static uint8_t lowByteNew  = 0;
    static uint8_t highByteNew = 0;
    
    static uint8_t count = 0;
    
    uint16_t previous;
    static uint16_t current = 0;
    unsigned long diff;
    
    static STATE_TYPE currentState = START_HIGH_STATE;
    
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
    
    //printf("P: %u\r\n", previous);
    //printf("C: %u\r\n", current);
    
    // check for overflow condition
    if (current < previous)
    {
        // DEBUG:
        //printf("Over:\r\n");
        
        // FIXME: no magic numbers
        // if overflow, we must compute difference by taking into account wrap around at maximum variable size
        diff = USHRT_MAX  - previous + current;
    } else {
        diff = current - previous;
    }
    
    // FIXME: no magic numbers
    // with timer clocked at (16 MHz / 4), four counts are needed to get one microsecond
    //diff = diff / 4;
    // 16 MHz / 12
    // e.g., (1/(16000000/24)) * dec(0xFFFF) = 98.30 milliseconds maximum can be counted
    diff = (diff * 3) / 2;
    
 
    // state machine begins by looking for sync pulse and then stores duration of data bits
    switch (currentState)
    {
        // only begin saving durations if expected start pulse timings are observed
        //   as an initial attempt to ignore noise pulses
        case START_HIGH_STATE:
            if ( abs(diff - gTimings[0]) < TOLERANCE_MIN )
            {

                
                gPacket.syncFirstDuration = diff;
                gPacket.syncFirstFlag = true;
                currentState = START_LOW_STATE;
            } else {
                //DEBUG:
                //reset_pin_off();
            }
            break;
            
        case START_LOW_STATE:
            
            if ( abs(diff - gTimings[2]) < TOLERANCE_MAX )
            {
                gPacket.syncSecondDuration = diff;
                gPacket.syncSecondFlag = true;
                count = 0;
                currentState = DATA_STATE;
            } else {
                currentState = START_HIGH_STATE;
            }
            break;
            
        case DATA_STATE:
        
            gPacket.duration[count] = diff;
            count++;
            
            // if we see a really long duration, bail out and reset to looking for sync pulse
            //if (diff >= 4500)
            //{
            //    currentState = START_HIGH_STATE;
            //    count = 0;
            //}

            
            if (count >= MAX_PERIOD_COUNT)
            {
                gPacket.captureDone = true;
                
                currentState = START_HIGH_STATE;
                
                // FIXME:
                // we may never get another transition if level remains the same
                // e.g. last bit is low level and default signal level is also low
                // so save last level here
                //gPacket.lastLevel = !is_rdata_low();
            }
            
            break;
            
        default:
            break;
    }
    
    
    //clear compare/capture 1 flag
    CCCON &= ~0x02;
}