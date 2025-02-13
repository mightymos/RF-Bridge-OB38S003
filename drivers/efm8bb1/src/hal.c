/*
 * hal.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */
#include <stdbool.h>
#include <stdint.h>

#include <EFM8BB1.h>


void set_clock_mode(void)
{
    //*****************************************
    // - Clock derived from the Internal High-Frequency Oscillator
    // - SYSCLK is equal to selected clock source divided by 1
    //*****************************************
    CLKSEL = CLKSL__HFOSC | CLKDIV__SYSCLK_DIV_1;
    
    //**********************************************************************
    // - System clock divided by 12
    // - Counter/Timer 0 uses the system clock
    // - Timer 2 high byte uses the clock defined by T2XCLK in TMR2CN0
    // - Timer 2 low byte uses the system clock
    // - Timer 3 high byte uses the clock defined by T3XCLK in TMR3CN0
    // - Timer 3 low byte uses the system clock
    // - Timer 1 uses the clock defined by the prescale field, SCA
    //***********************************************************************/
    //CKCON0 = SCA__SYSCLK_DIV_12 | T0M__SYSCLK | T2MH__EXTERNAL_CLOCK | T2ML__SYSCLK | T3MH__EXTERNAL_CLOCK | T3ML__SYSCLK | T1M__PRESCALE;
    // controls the timer 0/1 prescale, deefaults is system clocked divided by 12
    //CKCON0 |= SCA__SYSCLK_DIV_12;
    
    // as explained in timer_interrupts.hal
    // 1/((24500000)/(256-0x0B)) = 0.00001 seconds
    //CKCON0 |= T0M__SYSCLK;
    
    // FIXME: this is only applicable for split 8-bit timer mode, need to double check
    // FIXME: used for portisch
    //CKCON0 |= T2ML__SYSCLK;
    //CKCON0 |= T3ML__SYSCLK;
    //TMR2CN0 default is 16-bit autoreload and system clock divided by 12
    
    // default is timer 1 uses the clock defined by the prescale field sca
    //CKCON0 |= T1M__PRESCALE;
}


// pg. 44 - once the watchdog is started it cannot be stopped
void enable_watchdog(void)
{
    //timebase = timebase;
    //idleState = idleState;
    //WDTCN = interval;
    //WDTCN = 0xA5;
    
    // 3.2768 seconds
    WDTCN = 0xA6;
}

void disable_watchdog(void)
{
    // pg. 218, sec. 20.3 disable watchdog timer
    EA = 0;
    WDTCN = 0xDE;
    WDTCN = 0xAD;
}

void refresh_watchdog(void)
{
    //WDTCN = 0xA5;
    WDTCN = 0xA5;
}

void reset_mcu(void)
{
    // writing a 1 to this bit forces a system reset
    RSTSRC |= SWRSF;
}

void init_port_pins_for_passthrough(void)
{
    // FIXME: add comment to explain pin functions
    // sec 11.3.1 Port I/O Modes of Operation
    // P0MDIN is configured as digital i/o by default
    // P1MDIN is configured as digital i/o by default
    
    // FIXME: correctly handle LED on sonoff different from LED on EFM8BB1LCK board
    P0MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN | B2__OPEN_DRAIN | B3__OPEN_DRAIN | B4__PUSH_PULL | B5__OPEN_DRAIN | B6__OPEN_DRAIN | B7__OPEN_DRAIN;
    P1MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN | B2__OPEN_DRAIN | B3__OPEN_DRAIN | B4__PUSH_PULL | B5__PUSH_PULL  | B6__PUSH_PULL  | B7__PUSH_PULL;
    
    // FIXME: none of the example code ever does this, needed as stated in datasheet?
    // disables high side driver so uart rx is used as input
    // disables high side driver so radio receive is used as input
    //P0_5 = 1;
    //P1_3 = 1;

    // default is not skipped (i.e. available to crossbar)
    // but we will skip all to use as gpio
    P0SKIP = 0xFF;
    P1SKIP = 0xFF;
    
    // FIXME: why do the examples set this when it is the default?
    // default is weak pullups enabled (makes sure input pins always have a known state even if externally disconnected) 
    // crossbar enabled
    //XBR2 &= ~WEAKPUD__BMASK;
    
    // FIXME: why does enabling the crossbar allow passthrough to work?
    XBR2 |= XBARE__BMASK;
}

void init_port_pins_for_serial(void)
{
    // FIXME: add comment to explain pin functions
    // sec 11.3.1 Port I/O Modes of Operation
    // P0MDIN is configured as digital i/o by default
    // P1MDIN is configured as digital i/o by default
    
    // FIXME: correctly handle LED on sonoff different from LED on EFM8BB1LCK board
    P0MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN | B2__OPEN_DRAIN | B3__OPEN_DRAIN | B4__PUSH_PULL | B5__OPEN_DRAIN | B6__OPEN_DRAIN | B7__OPEN_DRAIN;
    P1MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN | B2__OPEN_DRAIN | B3__OPEN_DRAIN | B4__PUSH_PULL | B5__PUSH_PULL  | B6__PUSH_PULL  | B7__PUSH_PULL;
    
    // disables high side driver so uart rx is used as input
    // disables high side driver so radio receive is used as input
    //P0_5 = 1;
    //P1_3 = 1;

    // default is not skipped (i.e. available to crossbar)
    P0SKIP = B0__SKIPPED | B1__SKIPPED | B2__SKIPPED | B3__SKIPPED     | B4__NOT_SKIPPED | B5__NOT_SKIPPED | B6__SKIPPED | B7__SKIPPED;
    P1SKIP = B0__SKIPPED | B1__SKIPPED | B2__SKIPPED | B3__NOT_SKIPPED | B4__SKIPPED     | B5__SKIPPED     | B6__SKIPPED | B7__SKIPPED;
    
    // UART TX, RX routed to Port pins P0.4 and P0.5
    XBR0 |= URT0E__ENABLED;
    
    // CEX0 routed to port pin
    XBR1 |= PCA0ME__CEX0;
    
    // default is weak pullups enabled (makes sure input pins always have a known state even if externally disconnected) 
    // crossbar enabled
    //XBR2 |= WEAKPUD__PULL_UPS_ENABLED;
    XBR2 |= XBARE__ENABLED;
    
}

void init_uart0(void)
{   
    SCON0 &= ~(SMODE__BMASK | MCE__BMASK | REN__BMASK);
    SCON0 = REN__RECEIVE_ENABLED | SMODE__8_BIT | MCE__MULTI_DISABLED;
}


uint8_t get_capture_flags(void)
{
    return PCA0CPM0;
}

void set_capture_flags(const uint8_t flags)
{
    PCA0CPM0 = flags;
}


//
void init_timer0_8bit_autoreload(const uint8_t value)
{
    // 8-bit with auto-reload
    TMOD |= T0M__MODE2;

    TH0 = value;
    TL0 = value;

}

void init_timer1_8bit_autoreload(const uint8_t value)
{
    // 8-bit Counter/Timer with Auto-Reload
    TMOD |= T1M__MODE2;
    
    // when TLx overflows, TLx is reloaded from THx
    TH1 = value;
    TL1 = value;
}


void pca0_init(void)
{
    // default pca acts normally when controller is idle
    // default timebase is system clock divided by 12
    // default pca CF overflow is disabled
    // use timer0 as pca count timebase because we can obtain exactly 1 microsecond time
    //PCA0MD |= CPS__T0_OVERFLOW;
    
    // enable both positive and negative edge triggers
    PCA0CPM0 |= CAPP__ENABLED;
    PCA0CPM0 |= CAPN__ENABLED;
}

void pca0_run(void)
{
    // enables the PCA counter/timer
    CR = 1;
}

void pca0_halt(void)
{
    CR = 0;
}

// as I recall, we were double clearing this flag
//void clear_capture_flag(void)
//{
//    // overflow flag
//    //PCA0CN0 &= ~CF__SET;
//    CF = 0;
//}


// time conversion is explained in the rcswitch.c file
uint16_t countsToTime(const uint16_t counts)
{
    uint16_t duration;
        
    // this only saves two code bytes on ob38s003 apparently
    // equivalent to divide by two
    //duration = counts / 2;
    duration = counts >> 1;
    
    return duration;
}