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

// track time since startup in one millisecond increments
//static unsigned long gTimeMilliseconds = 0;
//static unsigned long gTimeTenMicroseconds = 0;

static __xdata uint16_t gTimer2Timeout;
static __xdata uint16_t gTimer2Interval;

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
    //PCA0MD = flags;
    PCA0CN0 = flags;
}

// Portisch favored this approach to timer delay
void set_timer2_reload(const uint16_t reload)
{
    /***********************************************************************
     - Timer 2 Reload High Byte
     ***********************************************************************/
    TMR2RLH = ((reload >> 8) & 0xFF);
    /***********************************************************************
     - Timer 2 Reload Low Byte = 0x86
     ***********************************************************************/
    TMR2RLL = (reload & 0xFF);
}


/*
 * we use this generic naming as compared with Portisch because different timers are used depending on microcontroller
 * Init Timer 2 with microseconds interval, maximum is 65535micros.
 */
void init_delay_timer_us(const uint16_t interval, const uint16_t timeout)
{
    //
    set_timer2_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000000 / (uint32_t)interval))));
    //TH1 = 0x60;
    //TL1 = 0x60;

    // remove 65micros because of startup delay
    gTimer2Timeout  = timeout;
    gTimer2Interval = interval;

    // start timer
    TR2 = true;
}


/*
 * Init Timer 2 with milliseconds interval, maximum is ~2.5ms.
 */
void init_delay_timer_ms(const uint16_t interval, const uint16_t timeout)
{    
    set_timer2_reload((uint16_t)(0x10000 - ((uint32_t) MCU_FREQ / (1000 / (uint32_t)interval))));

    gTimer2Timeout  = timeout;
    gTimer2Interval = interval;

    // start timer
    TR2 = true;
}


void wait_delay_timer_finished(void)
{
    // wait until timer has finished
    while(TR2);
}


void stop_delay_timer(void)
{
    // stop timer
    TR2 = false;
    
    // clear overflow flag (why, to avoid triggering interrupt next enable?)
    TF2 = false;
}

bool is_delay_timer_finished(void)
{
    return !TR2;
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


void pca0_isr(void) __interrupt (PCA0_VECTOR)
{
    //FIXME: we need to record the actual time step this represents so it is clear to human readers
    //FIXME: should be PCA0CP0 * 10 for Portisch?
    //       probably not, because we are using dedicated PCA counter instead of timer 0 as portisch did originally
    uint16_t currentCapture = PCA0CP0;
    
    // save and clear flags
    uint8_t flags = PCA0CN0 & (CF__BMASK | CCF0__BMASK | CCF1__BMASK | CCF2__BMASK);

    PCA0CN0 &= ~flags;


    //if( (PCA0PWM & COVF__BMASK) && (PCA0PWM & ECOV__BMASK))
    //{
    //  PCA0_intermediateOverflowCb();
    //}

    // FIXME: add comment
    PCA0PWM &= ~COVF__BMASK;

    //if((flags & CF__BMASK) && (PCA0MD & ECF__BMASK))
    //{
    //  PCA0_overflowCb();
    //}

    // FIXME: we might eventually want to use CF flag to detect counter wrap around
    if((flags & CCF0__BMASK) && (PCA0CPM0 & ECCF__BMASK))
    {
        // apparently our radio input
        //pca0_channel0EventCb();
        capture_handler(currentCapture);
        
        // DEBUG:
        //if (rdata_level())
        //{
        //  debug_pin01_on();
        //} else {
        //  debug_pin01_off();
        //}
    }
    
    // done in the interrupt already on efm8bb1
    // but must be explicitly cleared on ob38s003
    // so just always clear it
    //clear pca0 interrupt flag
    clear_capture_flag();

    //if((flags & CCF1__BMASK) && (PCA0CPM1 & ECCF__BMASK))
    //{
    //  PCA0_channel1EventCb();
    //}

    //if((flags & CCF2__BMASK) && (PCA0CPM2 & ECCF__BMASK))
    //{
    //  PCA0_channel2EventCb();
    //}
}