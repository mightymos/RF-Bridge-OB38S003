#include <stdint.h>
#include <stdlib.h>

// OB38S003 uses timer 2 capture mode for similar functionality to PCA on EFM8BB1
#include "OB38S003.h"

#include "hal.h"

// FIXME: is there a way to avoid this?
// we are getting the definition of capture_handler() from the portisch header
// but actually getting the implementation from rcswitch when that mode is compiled
//#if defined(TARGET_RCSWITCH)
//    #include "rcswitch.h"
//#elif defined(TARGET_PORTISCH)
    #include "portisch_rf_handling.h"
//#else
//    #error Please define TARGET_MCU in makefile
//#endif

#include "timer_interrupts.h"


// tens of microseconds increments
static __xdata uint16_t gTimer0Timeout;
// milliseconds increments
static __xdata uint16_t gTimer1Timeout;


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
    
    // FIXME: do we also need to clear capture registers?
    //CCH1 = 0x00;
    //CCL1 = 0x00;
    TH2 = 0x00;
    TL2 = 0x00;
	
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

void set_timer0_reload(const uint8_t reload)
{
    // autoreload register
    TH0 = reload;
    // actual timer register
    TL0 = reload;
}

void set_timer1_reload(const uint8_t reload)
{
    // autoreload register
    TH1 = reload;
    // actual timer register
    TL1 = reload;
}


#if 0

/*
 * Init Timer 0 with microseconds interval, maximum is 65535micros.
 */
void init_first_delay_us(const uint16_t timeout)
{
    // clock source is fosc
    PFCON &= ~0x03;
    PFCON |=  0x01;
    
    //
    set_timer0_reload(TIMER0_RELOAD_10MICROS);
    
    // FIXME: portisch removed 65micros because of startup delay
    //        but I do not see any information on this in ob38s003 datasheet
    gTimer0Timeout  = timeout;

    // start timer
    TR0 = true;
}

#endif

/*
 * Init Timer 1 with milliseconds interval, maximum is ~2.5ms.
 */
void init_first_delay_ms(const uint16_t timeout)
{
    // clock source is fosc / 96 to allow millisecond durations
    PFCON &= ~0x03;
    PFCON |=  0x02;
    
    //
    set_timer0_reload(TIMER0_RELOAD_1MILLIS);

    gTimer0Timeout  = timeout;

    // start timer
    TR0 = true;
}

/*
 * Init Timer 0 with microseconds interval, maximum is 65535micros.
 */
void init_second_delay_us(const uint16_t timeout)
{
    // clock source is fosc
    PFCON &= ~0x0C;
    PFCON |=  0x04;
    
    //
    set_timer1_reload(TIMER1_RELOAD_10MICROS);
    
    // FIXME: portisch removed 65micros because of startup delay
    //        but I do not see any information on this in ob38s003 datasheet
    gTimer1Timeout  = timeout;

    // start timer
    TR1 = true;
}

/*
 * Init Timer 1 with milliseconds interval, maximum is ~2.5ms.
 */
void init_second_delay_ms(const uint16_t timeout)
{
    // clock source is fosc / 96 to allow millisecond durations
    PFCON &= ~0x0C;
    PFCON |=  0x08;
    
    //
    set_timer1_reload(TIMER1_RELOAD_1MILLIS);

    gTimer1Timeout  = timeout;

    // start timer
    TR1 = true;
}


void wait_first_delay_finished(void)
{
    // wait until timer has finished
    while(TR0);
}

void wait_second_delay_finished(void)
{
    // wait until timer has finished
    while(TR1);
}


void stop_first_delay(void)
{
    // stop timer
    TR0 = false;
    
    // clear overflow flag (why, to avoid triggering interrupt next enable?)
    TF0 = false;
}

void stop_second_delay(void)
{
    // stop timer
    TR1 = false;
    
    // clear overflow flag (why, to avoid triggering interrupt next enable?)
    TF1 = false;
}

bool is_first_delay_finished(void)
{
    return !TR0;
}

bool is_second_delay_finished(void)
{
    return !TR1;
}


// timer 0 interrupt
void timer0_isr(void) __interrupt (d_T0_Vector)
{
    // we use autoreload instead of manually reloading count so that interrupt is short
    // ob38s003 microcontroller automatically clears timer flag
    
    // DEBUG:
    //debug_pin01_toggle();
    
    // we avoid handling intervals of more than one because instructions slow done interrupt too much
    
    // we decrement because if we incremented we would have to compare to a timeout external to interrupt
    // and we would then have to access variable atomically (e.g., by disabling interrupts briefly)
    // and as a result we might miss counts
    gTimer0Timeout--;
        
    // check if pulse time is over
    if(gTimer0Timeout == 0)
    {
        // stop timer
        TR0 = false;
    }    
}


// timer 1 interrupt
void timer1_isr(void) __interrupt (d_T1_Vector)
{
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
    
}