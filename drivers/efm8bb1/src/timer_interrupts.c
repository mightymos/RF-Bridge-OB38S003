#include <stdint.h>
#include <stdlib.h>

#include "EFM8BB1.h"


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


// timer 2 interrupt
void timer2_isr(void) __interrupt (TIMER2_VECTOR)
{
    // tracks time since timer enabled, used to track long periods of time
    gTimeTenMicroseconds++;
    
	// clear interrupt flag
	TF2 = 0;
}


void pca0_channel0EventCb(void)
{
    //FIXME: we need to record the actual time step this represents so it is clear to human readers
	uint16_t currentCapture = PCA0CP0;
    
    capture_handler(currentCapture);
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

	// FIXME: we might eventually want to use CF flag to detect counter wrap around
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