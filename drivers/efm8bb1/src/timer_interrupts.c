#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "EFM8BB1.h"


#include "hal.h"
#include "rcswitch.h"
#include "timer.h"

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


void timer0_isr(void) __interrupt (TIMER0_VECTOR)
{
    gTimeMilliseconds++;

    // one millisecond to overflow
    load_timer0(0xFCAD);
}


// timer 1 interrupt
void timer1_isr(void) __interrupt (TIMER1_VECTOR)
{
    // tracks time since timer enabled, used to track long periods of time
    gTimeTenMicroseconds++;
    
    // ten microseconds to overflow
    load_timer1(0xFFEA);
}

void pca0_channel0EventCb(void)
{
    //FIXME: we need to specify the actual time step this represents
	uint16_t current_capture_value = PCA0CP0 * 10;
	uint8_t flags = PCA0MD;
	
    const uint8_t gapMagicNumber  = 200;
    const uint8_t repeatThreshold   = 2;
    
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

    // go from 8-bit to 16-bit variables
    previous = current;
    current = current_capture_value;
	
	// clear counter
	PCA0MD &= 0xBF;
	PCA0H = 0x00;
	PCA0L = 0x00;
	PCA0MD = flags;
    
    
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
    // with timer clocked at (16 MHz / 4), four counts are needed to get one microsecond
    //duration = duration / 4;
    // 16 MHz / 12
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
    //clear_ccp1_flag();
}



void pca0_isr(void) __interrupt (PCA0_VECTOR)
{
  // save and clear flags
  uint8_t flags = PCA0CN0 & (CF__BMASK | CCF0__BMASK | CCF1__BMASK | CCF2__BMASK);

  PCA0CN0 &= ~flags;

  //if( (PCA0PWM & COVF__BMASK) && (PCA0PWM & ECOV__BMASK))
  //{
  //  PCA0_intermediateOverflowCb();
  //}

  PCA0PWM &= ~COVF__BMASK;

  //if((flags & CF__BMASK) && (PCA0MD & ECF__BMASK))
  //{
  //  PCA0_overflowCb();
  //}

  if((flags & CCF0__BMASK) && (PCA0CPM0 & ECCF__BMASK))
  {
  	// apparently our radio input
    pca0_channel0EventCb();
  }

  //if((flags & CCF1__BMASK) && (PCA0CPM1 & ECCF__BMASK))
  //{
  //  PCA0_channel1EventCb();
  //}

  //if((flags & CCF2__BMASK) && (PCA0CPM2 & ECCF__BMASK))
  //{
  //  PCA0_channel2EventCb();
  //}
}