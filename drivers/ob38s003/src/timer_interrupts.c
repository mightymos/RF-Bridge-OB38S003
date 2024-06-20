#include <stdint.h>
#include <stdlib.h>

// OB38S003 uses timer 2 capture mode for similar functionality to PCA on EFM8BB1
#include "OB38S003.h"

#include "delay.h"
#include "hal.h"
#include "rcswitch.h"
#include "timer_interrupts.h"

// track time since startup in one millisecond increments
static __xdata uint16_t gTimeMilliseconds = 0;
static __xdata uint16_t gTimeTenMicroseconds = 0;

static __xdata uint16_t gTimer1Timeout;
static __xdata uint16_t gTimer1Interval;

uint16_t get_time_milliseconds(void)
{
	return gTimeMilliseconds;
}

uint16_t get_time_ten_microseconds(void)
{
	return gTimeTenMicroseconds;
}

// Portisch favored this approach to timer delay
void set_timer1_reload(const uint16_t reload)
{
	/***********************************************************************
	 - Timer 2 Reload High Byte
	 ***********************************************************************/
	TH1 = ((reload >> 8) & 0xFF);
	/***********************************************************************
	 - Timer 2 Reload Low Byte = 0x86
	 ***********************************************************************/
	TL1 = (reload & 0xFF);
}


/*
 * Init Timer 2 with microseconds interval, maximum is 65535micros.
 */
void init_timer1_us(const uint16_t interval, const uint16_t timeout)
{
    //
	//set_timer1_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000000 / (uint32_t)interval))));
	TH1 = 0x60;
	TL1 = 0x60;

	// remove 65micros because of startup delay
	gTimer1Timeout  = timeout;
	gTimer1Interval = interval;

	// start timer
	TR1 = true;
}


/*
 * Init Timer 2 with milliseconds interval, maximum is ~2.5ms.
 */
void init_timer1_ms(const uint16_t interval, const uint16_t timeout)
{    
	set_timer1_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000 / (uint32_t)interval))));

	gTimer1Timeout  = timeout;
	gTimer1Interval = interval;

	// start timer
	TR1 = true;
}


void wait_timer1_finished(void)
{
	// wait until timer has finished
	while(TR1);
}


void stop_timer1(void)
{
	// stop timer
	TR1 = false;
	
	// clear overflow flag (why, to avoid triggering interrupt next enable?)
	TF1 = false;
}

bool is_timer1_finished(void)
{
	return !TR1;
}

#if 0

void timer0_isr(void) __interrupt (d_T0_Vector)
{
    // one millisecond to overflow
	TH0 = TH0_RELOAD_1MILLIS;
	TL0 = TL0_RELOAD_1MILLIS;
	
    gTimeMilliseconds++;
}

#endif

// timer 1 interrupt
void timer1_isr(void) __interrupt (d_T1_Vector)
{
	// we avoid reloading with a function because the function call slows down interrupt
	//TH1 = TH1_RELOAD_10MICROS;
	//TL1 = TL1_RELOAD_10MICROS;
	
    // tracks time since timer enabled, used to track long periods of time
    //gTimeTenMicroseconds++;
	
	// Clear Timer 2 high overflow flag
	// (our microcontroller automatically clears this flag)
	//TF1 = false;
	//debug_pin01_toggle();

	// check if pulse time is over
	if(gTimer1Timeout == 0)
	{
		// stop timer
		TR1 = false;
	} else {
	
	//if (gTimer1Timeout < gTimer1Interval)
	//{
	//	gTimer1Timeout = 0;
	//} else {
	//	gTimer1Timeout -= gTimer1Interval;
		gTimer1Timeout--;
	//}
	}
    
}

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (d_T2_Vector)
{
    uint16_t currentCapture = get_timer2();
    
    capture_handler(currentCapture);
}