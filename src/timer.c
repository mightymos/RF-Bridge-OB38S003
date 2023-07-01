#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "..\inc\globals.h"
#include "..\inc\rf_protocols.h"
#include "..\inc\timer.h"

// store attempts to decode radio bits
__xdata struct RADIO_PACKET_T gPacket;

// track count of timer 1 which is decremented, and finally flag reaching zero
uint16_t gTimerOneCount = 0;
bool     gIsTimerOneFinished = true;

// track time since startup in one millisecond increments
static unsigned long gTimeMilliseconds = 0;


// rc-switch protocol 1
const uint16_t gTimings [] = { 350, 1050, 10850 };
//const uint16_t gTimings [] = { 1000, 1000, 1000 };


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

// FIXME: change function name because this is not directly reloading timer
//void reload_timer1(unsigned int reload)
//{
//    gIsTimerOneFinished = false;
//    gTimerOneCount = reload;
//}

void reload_timer1(unsigned char high, unsigned char low)
{
    gIsTimerOneFinished = false;
    
    TH1 = high;
    TL1 = low;
}

void timer0_isr(void) __interrupt (1)
{
    gTimeMilliseconds++;

    // one millisecond to overflow
    TH0 = 0xc1;
    TL0 = 0x7f;
    
    // ten microseconds to overflow
    //TH0 = 0xff;
    //TL0 = 0x5f;
}


// timer 1 interrupt
void timer1_isr(void) __interrupt (3)
{
    // one shot type count essentially used to implement non-blocking delays
    //if (gTimerOneCount > 0)
    //{
    //    gTimerOneCount--;
    //    
    //    if (gTimerOneCount == 0)
    //    {
    //        gIsTimerOneFinished = true;
    //    }
    //}
    
    gIsTimerOneFinished = true;
    
    // tracks time since timer enabled, used to track long periods of time
    //gTimerOneElapsed++;
    
    // ten microseconds to overflow
    //TH1 = 0xff;
    //TL1 = 0x5f;
}

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (5)
{
    uint8_t lowByteSaved;
    uint8_t highByteSaved;
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

    // save in case check at end of interrupts reverts value
    lowByteSaved  = lowByteNew;
    highByteSaved = highByteNew;
    
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
    
    // skip any edge which does not fall within tolerance of expected periods
    if (diff % gTimings[0] < TOLERANCE_MIN)
    {
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
        
        // DEBUG:
        reset_pin_toggle();
        
    } else {
        // revert
        lowByteNew  = lowByteSaved;
        highByteNew = highByteSaved;
    }
        
    //clear compare/capture 1 flag
    CCCON &= ~0x02;
}


    // In this approach we will capture the time between each transition coming from the demodulation circuit. The Input Capture
    // function on a micro-controller is very useful for this because it will generate an interrupt, precise time measurements, and
    // allow decision processing based on the elapsed counter value.
    // 1. Set up timer to interrupt on every edge (may require changing edge trigger in the ISR)
    // FIXME: done externally to interrupt so that's okay right?
    
    // 2. ISR routine should flag the edge occurred and store count value
    
    
    // 3. Start timer, capture first edge and discard this.
    // FIXME: we do not discard the first edge but should
    
    // 4. Capture next edge and check if stored count value equal 2T (T = ½ data rate)
    
    
    // 5. Repeat step 4 until count value = 2T (This is now synchronized with the data clock)
    // 6. Read current logic level of the incoming pin and save as current bit value (1 or 0)
    // 7. Capture next edge
    // a. Compare stored count value with T
    // b. If value = T
    // ● Capture next edge and make sure this value also = T (else error)
    // ● Next bit = current bit
    // ● Return next bit
    // c. Else if value = 2T
    // ● Next bit = opposite of current bit
    // ● Return next bit
    // d. Else
    // ● Return error
    // 8. Store next bit in buffer
    // 9. If desired number of bits are decoded; exit to continue further processing
    // 10. Else set current bit to next bit and loop to step 7
