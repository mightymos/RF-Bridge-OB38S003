#include <stdint.h>
#include <stdlib.h>

// OB38S003 uses timer 2 capture mode for similar functionality to PCA on EFM8BB1
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
    uint16_t currentCapture = get_timer2();
    
    capture_handler(currentCapture);
}