#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

// OB38S003 uses timer 2 capture mode for similar functionality to pca on EFM8BB1
#include "OB38S003.h"


#include "hal.h"
#include "rcswitch.h"
#include "timer_interrupts.h"

// track time since startup in one millisecond increments
static unsigned long gTimeMilliseconds = 0;
static unsigned long gTimeTenMicroseconds = 0;

unsigned long get_time_milliseconds(void)
{
	return gTimeMilliseconds;
}

unsigned long get_time_ten_microseconds(void)
{
	return gTimeTenMicroseconds;
}

#if 0

void timer0_isr(void) __interrupt (d_T0_Vector)
{
    gTimeMilliseconds++;

    // one millisecond to overflow
    load_timer0(TIMER0_RELOAD_1MILLIS);
}

#endif

// timer 1 interrupt
void timer1_isr(void) __interrupt (d_T1_Vector)
{
    // tracks time since timer enabled, used to track long periods of time
    gTimeTenMicroseconds++;
    
    // ten microseconds to overflow
    load_timer1(TIMER1_RELOAD_10MICROS);
}

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (d_T2_Vector)
{
    const uint8_t gapMagicNumber  = 200;
    const uint8_t repeatThreshold   = 2;
    
    // track previous and new timer values so we can compute dfiference
    uint8_t lowByteOld;
    uint8_t highByteOld;
    static uint8_t lowByteNew  = 0;
    static uint8_t highByteNew = 0;
    
    // for converting 8-bit timer values to 16-bits to allow subtraction
    uint16_t        previous;
    static uint16_t current = 0;
    
    // this eventually represents the level duration in microseconds (difference between edge transitions)
    unsigned long duration;
    

    // rc-switch variables
    static unsigned int repeatCount = 0;
    static unsigned int changeCount = 0;

    // FIXME: move to rcswitch.h
    const unsigned int separationLimit = gRCSwitch.nSeparationLimit;

    
    // we are always looking for pulse duration (i.e., difference), so need to store previous and new values
    lowByteOld  = lowByteNew;
    highByteOld = highByteNew;
    
    // this stores timer 2 value without stopping timer 2
    // FIXME: I think read order is important here, double check
    lowByteNew  = get_timer2_low();
    highByteNew = get_timer2_high();
    
    // go from 8-bit to 16-bit variables
    previous = current;
    current = (highByteNew << 8) | lowByteNew;
    
    
    // check for overflow condition
    if (current < previous)
    {
        // FIXME: no magic numbers
        // if overflow, we must compute difference by taking into account wrap around at maximum variable size
        duration = USHRT_MAX  - previous + current;
    } else {
        duration = current - previous;
    }
    
    // FIXME: no magic numbers
    // e.g. prescale at (1/4) at 16 MHz, four counts are needed to get one microsecond
    // e.g. prescale at (1/24) at 16 MHz
    // e.g., (1/(16000000/24)) * dec(0xFFFF) = 98.30 milliseconds maximum can be counted
    duration = (duration * 3) / 2;
    
    // from oscillscope readings it appears that first sync pulse of first radio packet is frequently not output properly by receiver
    // this could be because radio receiver needs to "warm up" (despite already being enabled?)
    // and it is known that radio packet transmissions are often repeated (between about four and twenty times) perhaps in part for this reason
    if (duration > separationLimit)
    {
        // A long stretch without signal level change occurred. This could
        // be the gap between two transmission.
        if (abs(duration - timings[0]) < gapMagicNumber)
        {
          // This long signal is close in length to the long signal which
          // started the previously recorded timings; this suggests that
          // it may indeed by a a gap between two transmissions (we assume
          // here that a sender will send the signal multiple times,
          // with roughly the same gap between them).
          repeatCount++;
          
          if (repeatCount == repeatThreshold)
          {
            for(unsigned int i = 1; i <= numProto; i++)
            {
              if (receive_protocol(i, changeCount))
              {
                // receive succeeded for protocol i
                break;
              }
            }
            
            repeatCount = 0;
          }
        }
        
        changeCount = 0;
    }

    // detect overflow
    if (changeCount >= RCSWITCH_MAX_CHANGES)
    {
        changeCount = 0;
        repeatCount = 0;
    }

    timings[changeCount++] = duration;
    
        
    //clear compare/capture 1 flag
    clear_capture_flag();
}