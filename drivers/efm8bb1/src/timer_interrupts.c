#include <stdint.h>
#include <stdlib.h>

//
#include "EFM8BB1.h"

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


// tens of microseconds increments
static __xdata uint16_t gTimer2Timeout;
// milliseconds increments
static __xdata uint16_t gTimer3Timeout;

//unsigned long get_time_milliseconds(void)
//{
//  return gTimeMilliseconds;
//  return 0;
//}

//unsigned long get_time_ten_microseconds(void)
//{
//  return gTimeTenMicroseconds;
//  return 0;
//}

void clear_interrupt_flags_pca(void)
{
    // clear all interrupt flags of PCA0
	PCA0CN0 &= ~(CF__BMASK | CCF0__BMASK | CCF1__BMASK | CCF2__BMASK);
}

void clear_pca_counter(void)
{
    // FIXME: I think this was a bug in portisch
	//uint8_t flags = PCA0MD;
    uint8_t flags = PCA0CN0;
    
	// FIXME: replace or move function to be abstracted from hardware
    // FIXME: I think this was a bug in portisch
    // and with 0xBF effectively clears bit6 of PCA0MD which is a reserved bit6
    // however, if we clear bit6 of PCA0CN0 that clears CR bit which stops PCA timer
    // and it makes more sense to do that prior to clearing counter values
	// clear counter
	//PCA0MD &= 0xBF;
    PCA0CN0 &= 0xBF;
    
	PCA0H = 0x00;
	PCA0L = 0x00;
	
    // FIXME: I think this was a bug in portisch
    // FIXME: why not just explicitly set CR bit if that was the intent?
    //PCA0MD = flags;
    PCA0CN0 = flags;
}

// Portisch favored this approach to timer delay
void set_timer2_reload(const uint16_t reload)
{
    // original portisch does not
    // seems we should initialize timer registers and setup reload values next
    //TMR2H = ((reload >> 8) & 0xFF);
    //TMR2L = (reload & 0xFF);
    
    // these are the reload values (placed into actual timer registers on overflow)
    TMR2RLH = ((reload >> 8) & 0xFF);
    TMR2RLL = (reload & 0xFF);
}

void set_timer3_reload(const uint16_t reload)
{
    // original portisch does not
    // seems we should initialize timer registers and setup reload values next
    //TMR3H = ((reload >> 8) & 0xFF);
    //TMR3L = (reload & 0xFF);
    
    // these are the reload values (placed into actual timer registers on overflow)
    TMR3RLH = ((reload >> 8) & 0xFF);
    TMR3RLL = (reload & 0xFF);
}


#if 0

/*
 * we use this generic naming as compared with Portisch because different timers are used depending on microcontroller
 * Init Timer 2 with microseconds interval, maximum is 65535micros.
 */
void init_first_delay_us(const uint16_t timeout)
{
    //
    //set_timer2_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000000 / (uint32_t)interval))));
    set_timer2_reload(TIMER2_RELOAD_10MICROS);
    
    // remove 65micros because of startup delay
    gTimer2Timeout = timeout;
    
    // we removed the interval variable because we only ever need a 10 microsecond count
    // and the interrupt logic to handle decrementing by an interval made the transmission timing inaccurate

    // start timer
    TR2 = true;
}

#endif

void init_first_delay_ms(const uint16_t timeout)
{
    //
    //set_timer2_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000000 / (uint32_t)interval))));
    set_timer2_reload(TIMER2_RELOAD_1MILLIS);
    
    // remove 65micros because of startup delay
    gTimer2Timeout = timeout;
    
    // we removed the interval variable because we only ever need a 10 microsecond count
    // and the interrupt logic to handle decrementing by an interval made the transmission timing inaccurate

    // start timer
    TR2 = true;
}

void init_second_delay_us(const uint16_t timeout)
{
    //
    //set_timer2_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000000 / (uint32_t)interval))));
    set_timer3_reload(TIMER3_RELOAD_10MICROS);
    
    // remove 65micros because of startup delay
    gTimer3Timeout = timeout;
    
    // we removed the interval variable because we only ever need a 10 microsecond count
    // and the interrupt logic to handle decrementing by an interval made the transmission timing inaccurate

    // start timer
    TMR3CN0 |= TR3__RUN;
}

/*
 * Init Timer 3 with milliseconds interval, maximum is ~2.5ms.
 */
void init_second_delay_ms(const uint16_t timeout)
{    
    set_timer3_reload(TIMER3_RELOAD_1MILLIS);

    gTimer3Timeout = timeout;

    // start timer
    TMR3CN0 |= TR3__RUN;
}


void wait_first_delay_finished(void)
{
    // wait until timer has finished
    while(TR2);
}

void wait_second_delay_finished(void)
{
    // wait until timer has finished
    // FIXME: compare to TR3__RUN as portisch did?
    while(TMR3CN0 & TR3__BMASK);
}


void stop_first_delay(void)
{
    // stop timer
    TR2 = false;
    
    // clear overflow flag (why, to avoid triggering interrupt next enable?)
    TF2 = false;
}

void stop_second_delay(void)
{
    // stop timer
    TMR3CN0 &= ~TR3__RUN;
    
    // clear overflow flag (why, to avoid triggering interrupt next enable?)
    TMR3CN0 &= ~TF3H__SET;
}

bool is_first_delay_finished(void)
{
    return !TR2;
}

bool is_second_delay_finished(void)
{
    return ((TMR3CN0 & TR3__BMASK) != TR3__RUN);
}

// timer 2 interrupt
void timer2_isr(void) __interrupt (TIMER2_VECTOR)
{
    // FIXME: clear at start or end of interrupt?
    TF2 = 0;
    
    gTimer2Timeout--;
        
    // check if pulse time is over
    if(gTimer2Timeout == 0)
    {
        // stop timer
        TR2 = false;
    }   
}

// timer 3 interrupt
void timer3_isr(void) __interrupt (TIMER3_VECTOR)
{
    // FIXME: clear at start or end of interrupt?
    TMR3CN0 &= ~TF3H__SET;
    
    gTimer3Timeout--;
        
    // check if pulse time is over
    if(gTimer3Timeout == 0)
    {
        // stop timer
        TMR3CN0 &= ~TR3__RUN;
    }   
}

// captures counter value on edge transitions
void pca0_isr(void) __interrupt (PCA0_VECTOR)
{
    // this was originally PCA0CP0 * 10 on original portisch which used timer 0 overflow instead of the dedicated PCA counter
    uint16_t currentCapture = PCA0CP0;
    
    // save and clear flags
    uint8_t flags = PCA0CN0 & (CF__BMASK | CCF0__BMASK | CCF1__BMASK | CCF2__BMASK);
    //bool flag = CCF0;

    // clear
    PCA0CN0 &= ~flags;
    //CCF0 = 0;

    // FIXME: we might eventually want to use CF flag to detect counter wrap around
    if((flags & CCF0__BMASK) && (PCA0CPM0 & ECCF__BMASK))
    //if (flag)
    {
        // apparently our radio input
        //pca0_channel0EventCb();
        capture_handler(currentCapture);
    }
    
    //clear overflow flag
    //clear_capture_flag();

}