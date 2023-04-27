#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "..\inc\globals.h"
#include "..\inc\rf_protocols.h"
#include "..\inc\timer.h"

//static uint16_t gTimer2Timeout;
//static uint16_t gTimer2Interval;
//uint16_t gTimer3Timeout;
//uint16_t gTimer3Interval;



__xdata struct RADIO_PACKET_T gPacket;

//volatile bool gSingleStep = true;

static unsigned long gTimeMilliseconds = 0;


// rc-switch protocol 1
const uint16_t gTimings [] = { 350, 1050, 10850 };
//const uint16_t gTimings [] = { 1000, 1000, 1000 };

//void timer0_isr(void) __interrupt (1)
//{
//    gTimeMilliseconds++;

//    // one millisecond to overflow
//    //TH0 = 0xc1;
//    //TL0 = 0x7f;
//    
//    // ten microseconds to overflow
//    TH0 = 0xff;
//    TL0 = 0x5f;
//}

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (5)
{
    uint8_t lowByteOld;
    uint8_t highByteOld;
    static uint8_t lowByteNew  = 0;
    static uint8_t highByteNew = 0;
    
   
    uint16_t        previous;
    static uint16_t current = 0;
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
    
    // FIXME: add comment
    gPacket.diff[gPacket.writePosition] = diff;
    
    // FIXME: add comment
    gPacket.writePosition++;
    
    // handle wrap around
    if (gPacket.writePosition == BUFFER_SIZE)
    {
        gPacket.writePosition = 0;
    }
    
    gPacket.length++;
        
    //clear compare/capture 1 flag
    CCCON &= ~0x02;
}


unsigned long get_current_time(void)
{
    unsigned long currentTime;
    
    // FIXME: disable timer0 interrupt for atomic reading of variable
    //        consider check to see if interrupt was enabled in the first place
    ET0 = 0;
    
    // FIXME: compute the proper conversion from counts to milliseconds
    currentTime = gTimeMilliseconds;
    
    // re-enable timer0 interrupt
    ET0 = 1;
    
    return currentTime;
}


unsigned long get_elapsed_time(unsigned long previousTime)
{
    unsigned long currentTime;
    unsigned long elapsed;
    
    currentTime = get_current_time();
    
    //printf("currentTime: %lu\r\n", currentTime);
    
    // handle typical versus wraparound condition
    if (previousTime <= currentTime)
    {
        elapsed = currentTime - previousTime;
    } else {
        elapsed = ULONG_MAX - previousTime + currentTime;
    }
    
    return elapsed;
}