#include <stdint.h>
#include <stdlib.h>

// OB38S003 uses timer 2 capture mode for similar functionality to PCA on EFM8BB1
#include "OB38S003.h"

#include "hal.h"

// FIXME: is there a way to avoid this?
//#if defined(TARGET_RCSWITCH)
//    #include "rcswitch.h"
//#elif defined(TARGET_PORTISCH)
    #include "portisch.h"
//#else
//    #error Please define TARGET_BOARD in makefile
//#endif

#include "timer_interrupts.h"

// track time since startup in one millisecond increments
//static __xdata uint16_t gTimeMilliseconds = 0;
//static __xdata uint16_t gTimeTenMicroseconds = 0;

static __xdata uint16_t gTimer1Timeout;
static __xdata uint16_t gTimer1Interval;

//uint16_t get_time_milliseconds(void)
//{
//  return gTimeMilliseconds;
//}

//uint16_t get_time_ten_microseconds(void)
//{
//  return gTimeTenMicroseconds;
//}

// FIXME: this is actually timer2 on ob38s003 but use pca naming convention for now
void clear_interrupt_flags_pca(void)
{
    // clear all interrupt flags of PCA0
	//PCA0CN0 &= ~(CF__BMASK | CCF0__BMASK | CCF1__BMASK | CCF2__BMASK);
    CCCON &= ~0x0F;
}

void clear_pca_counter(void)
{
	//uint8_t flags = PCA0MD;
    
    // timer 2 stop
    T2CON &= ~0x03;
    
    // comment
    CCH1 = 0x00;
    CCL1 = 0x00;
	
    // start timer2, input frequency from prescaler
    T2CON |= 0x01;
}

// appears that a tick implementation that increments a counter
// is difficult to obtain accurate timings for on this processor
// we think because accessing the count atomically requires disabling interrupts
// and therefore some interrupt counts are missed

// therefore Portisch favored this approach to timer delay
// which decrements a timer all within the interrupt
// and essentially indicates completion by disabling timer
// and checking in user land that it has been disabled as a sort of finished flag
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
 * Init Timer 1 with microseconds interval, maximum is 65535micros.
 */
void init_delay_timer_us(const uint16_t interval, const uint16_t timeout)
{
    // FIXME: why were we explicitly setting to 10 microseconds previously?
    set_timer1_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000000 / (uint32_t)interval))));
    // 10 microseconds
    //TH1 = T1_AUTORELOAD_10MICROS;
    //TL1 = T1_AUTORELOAD_10MICROS;

    // 1 microsecond
    //TH1 = 0xF0;
    //TL1 = 0xF0;
    
    // remove 65micros because of startup delay
    gTimer1Timeout  = timeout;
    gTimer1Interval = interval;

    // start timer
    TR1 = true;
}


/*
 * Init Timer 1 with milliseconds interval, maximum is ~2.5ms.
 */
void init_delay_timer_ms(const uint16_t interval, const uint16_t timeout)
{    
    set_timer1_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000 / (uint32_t)interval))));

    gTimer1Timeout  = timeout;
    gTimer1Interval = interval;

    // start timer
    TR1 = true;
}


void wait_delay_timer_finished(void)
{
    // wait until timer has finished
    while(TR1);
}


void stop_delay_timer(void)
{
    // stop timer
    TR1 = false;
    
    // clear overflow flag (why, to avoid triggering interrupt next enable?)
    TF1 = false;
}

bool is_delay_timer_finished(void)
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
    // we use autoreload instead of manually reloading count so that interrupt is short
    // ob38s003 microcontroller automatically clears timer flag
    
    // DEBUG:
    //debug_pin01_toggle();
    
    
    // we avoid handling intervals of more than one because instructions slow done interrupt too much
    
    // we decrement because if we incremented we would have to compare to a timeout external to interrupt
    // and we would then have to access variable atomically (e.g., by disabling interrupts briefly)
    // and as a result we might miss counts
    gTimer1Timeout--;
        
    // check if pulse time is over
    if(gTimer1Timeout == 0)
    {
        // stop timer
        TR1 = false;
    }       
}

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (d_T2_Vector)
{
    uint16_t currentCapture = get_capture_mode();
    
    capture_handler(currentCapture);
    
    // done in the interrupt already on efm8bb1
    // but must be explicitly cleared on ob38s003
    // so just always clear it
    //clear pca0 interrupt flag
    clear_capture_flag();
    
    // DEBUG:
    if (rdata_level())
    {
      debug_pin01_on();
    } else {
      debug_pin01_off();
    }
}