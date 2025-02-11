/*
 * hal.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */
#include <stdbool.h>
#include <stdint.h>

#include "SI_EFM8BB52_Register_Enums.h"

void set_clock_mode(void)
{
    //*****************************************
    // - Clock derived from the Internal High-Frequency Oscillator
    // - SYSCLK is equal to selected clock source divided by 1
    //*****************************************
    CLKSEL = CLKSEL_CLKSL__HFOSC0_clk24p5 | CLKSEL_CLKDIV__SYSCLK_DIV_1;
    
    // from example code
    while ((CLKSEL & CLKSEL_DIVRDY__BMASK) == CLKSEL_DIVRDY__NOT_READY);
    
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
    
    // allows 115200 baud rate on UART0 with TIMER1
    CKCON0 |= CKCON0_SCA__SYSCLK_DIV_4;
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
    IE_EA = 0;
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
    RSTSRC |= RSTSRC_SWRSF__SET;
}

void init_port_pins_for_passthrough(void)
{
    // FIXME: add comment to explain pin functions
    // sec 11.3.1 Port I/O Modes of Operation
    // P0MDIN is configured as digital i/o by default
    // P1MDIN is configured as digital i/o by default
    
    // FIXME: correctly handle LED on sonoff different from LED on EFM8BB1LCK board
    P0MDOUT = P0MDOUT_B0__PUSH_PULL | P0MDOUT_B1__OPEN_DRAIN | P0MDOUT_B2__OPEN_DRAIN | P0MDOUT_B3__OPEN_DRAIN | P0MDOUT_B4__PUSH_PULL | P0MDOUT_B5__OPEN_DRAIN | P0MDOUT_B6__OPEN_DRAIN | P0MDOUT_B7__OPEN_DRAIN;
    P1MDOUT = P1MDOUT_B0__PUSH_PULL | P1MDOUT_B1__OPEN_DRAIN | P1MDOUT_B2__OPEN_DRAIN | P1MDOUT_B3__OPEN_DRAIN | P1MDOUT_B4__PUSH_PULL | P1MDOUT_B5__PUSH_PULL  | P1MDOUT_B6__PUSH_PULL  | P1MDOUT_B7__PUSH_PULL;
    
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
    XBR2 |= XBR2_XBARE__ENABLED;
}

void init_port_pins_for_serial(void)
{
    // need to save and later restore page because P2MDIN, P2SKIP are on page 0x20
    uint8_t SFRPAGE_save = SFRPAGE;
    
    // FIXME: add comment to explain pin functions
    // sec 11.3.1 Port I/O Modes of Operation
    // P0MDIN is configured as digital i/o by default
    // P1MDIN is configured as digital i/o by default
    
    // FIXME: correctly handle LED on sonoff different from LED on EFM8BB1LCK board
    P0MDOUT = P0MDOUT_B0__PUSH_PULL | P0MDOUT_B1__OPEN_DRAIN | P0MDOUT_B2__OPEN_DRAIN | P0MDOUT_B3__OPEN_DRAIN | P0MDOUT_B4__PUSH_PULL | P0MDOUT_B5__OPEN_DRAIN | P0MDOUT_B6__OPEN_DRAIN | P0MDOUT_B7__OPEN_DRAIN;
    P1MDOUT = P1MDOUT_B0__PUSH_PULL | P1MDOUT_B1__OPEN_DRAIN | P1MDOUT_B2__OPEN_DRAIN | P1MDOUT_B3__OPEN_DRAIN | P1MDOUT_B4__PUSH_PULL | P1MDOUT_B5__PUSH_PULL  | P1MDOUT_B6__PUSH_PULL  | P1MDOUT_B7__PUSH_PULL;
    P2MDOUT = P2MDOUT_B0__OPEN_DRAIN | P2MDOUT_B1__OPEN_DRAIN | P2MDOUT_B2__PUSH_PULL | P2MDOUT_B3__OPEN_DRAIN | P2MDOUT_B4__OPEN_DRAIN | P2MDOUT_B5__OPEN_DRAIN  | P2MDOUT_B6__OPEN_DRAIN  | P2MDOUT_B7__OPEN_DRAIN;
        
    // this is different from efm8bb1 which is digital mode by default
    P0MDIN = P0MDIN_B0__DIGITAL | P0MDIN_B1__ANALOG | P0MDIN_B2__ANALOG | P0MDIN_B3__ANALOG | P0MDIN_B4__DIGITAL | P0MDIN_B5__DIGITAL | P0MDIN_B6__ANALOG | P0MDIN_B7__ANALOG;
    P1MDIN = P1MDIN_B0__ANALOG | P1MDIN_B1__ANALOG | P1MDIN_B2__ANALOG | P1MDIN_B3__DIGITAL | P1MDIN_B4__DIGITAL | P1MDIN_B5__ANALOG | P1MDIN_B6__DIGITAL | P1MDIN_B7__ANALOG;
    
    // disables high side driver so uart rx is used as input
    // disables high side driver so radio receive is used as input
    //P0_5 = 1;
    //P1_3 = 1;

    // default is not skipped (i.e. available to crossbar)
    P0SKIP = P0SKIP_B0__SKIPPED | P0SKIP_B1__SKIPPED | P0SKIP_B2__SKIPPED | P0SKIP_B3__SKIPPED | P0SKIP_B4__NOT_SKIPPED | P0SKIP_B5__NOT_SKIPPED | P0SKIP_B6__SKIPPED | P0SKIP_B7__SKIPPED;
    P1SKIP = P1SKIP_B0__SKIPPED | P1SKIP_B1__SKIPPED | P1SKIP_B2__SKIPPED | P1SKIP_B3__NOT_SKIPPED | P1SKIP_B4__SKIPPED | P1SKIP_B5__SKIPPED | P1SKIP_B6__SKIPPED | P1SKIP_B7__SKIPPED;
    
    
    // basically we are just supporting UART1 for debugging output
    SFRPAGE = 0x20;

    P2MDIN  = P2MDIN_B0__ANALOG | P2MDIN_B1__ANALOG | P2MDIN_B2__DIGITAL | P2MDIN_B3__DIGITAL | P2MDIN_B4__ANALOG | P2MDIN_B5__ANALOG | P2MDIN_B6__ANALOG | P2MDIN_B7__ANALOG;
    P2SKIP  = P2SKIP_B0__SKIPPED | P2SKIP_B1__SKIPPED | P2SKIP_B2__NOT_SKIPPED | P2SKIP_B3__NOT_SKIPPED | P2SKIP_B4__SKIPPED | P2SKIP_B5__SKIPPED | P2SKIP_B6__SKIPPED | P2SKIP_B7__SKIPPED;
    
    SFRPAGE = 0x00;
    
    // UART TX, RX routed to Port pins P0.4 and P0.5
    XBR0 |= XBR0_URT0E__ENABLED;
    
    // CEX0 routed to port pin
    XBR1 |= XBR1_PCA0ME__CEX0;
    
    // default is weak pullups enabled (makes sure input pins always have a known state even if externally disconnected) 
    //XBR2 |= WEAKPUD__PULL_UPS_ENABLED;
    // enable uart1
    XBR2 |= XBR2_URT1E__ENABLED;
    
    // crossbar enabled
    XBR2 |= XBR2_XBARE__ENABLED;

    
    // restore
    SFRPAGE = SFRPAGE_save;
}

// FIXME: consider naming convention
void init_uart0(void)
{   
    SCON0 &= ~(SCON0_SMODE__BMASK | SCON0_MCE__BMASK | SCON0_REN__BMASK);
    SCON0 = SCON0_REN__RECEIVE_ENABLED | SCON0_SMODE__8_BIT | SCON0_MCE__MULTI_DISABLED;
}

// FIXME: consider naming convention
void init_uart1(void)
{   
    // need to save and later restore page because SCON1 is on page 0x20
    uint8_t SFRPAGE_save = SFRPAGE;
    
    SFRPAGE = 0x20;
    
    // default prescaler is divide by 12
    // (24500000/ (65536 - 0xFFCB)) * ((1 / 2) * (1 / 12)) = 19261.00628930817610062893
    SBCON1 |= SBCON1_BREN__ENABLED;
    
    // 19200 baud rate for prescaler of 12
    SBRLH1 = 0xFF;
    SBRLL1 = 0xCB;
    
    //SCON1 &= ~(SCON1_SMODE__BMASK | SCON1_MCE__BMASK | SCON1_REN__BMASK);
    //SCON1 = SCON1_REN__RECEIVE_ENABLED | SCON1_SMODE__8_BIT | SCON1_MCE__MULTI_DISABLED;
    SCON1 |= SCON1_REN__RECEIVE_ENABLED;
    
    
	/***********************************************************************
	 - A receive timeout will occur after 16 idle periods on the UART RX line
	 ***********************************************************************/
	UART1FCN1 |= UART1FCN1_RXTO__TIMEOUT_16;
    
    // restore
    SFRPAGE = SFRPAGE_save;
}

void init_uart1_transmit_interrupt_flag(void)
{
    // need to save and later restore page because SCON1 is on page 0x20
    uint8_t SFRPAGE_save = SFRPAGE;
    
    SFRPAGE = 0x20;
    
    // initialite TI = 1 so that putchar() loop is skipped on first usage
    SCON1_TI = 1;

    SFRPAGE = SFRPAGE_save;
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
    TMOD |= TMOD_T0M__MODE2;

    TH0 = value;
    TL0 = value;

}

void init_timer1_8bit_autoreload(const uint8_t value)
{
    // 8-bit Counter/Timer with Auto-Reload
    TMOD |= TMOD_T1M__MODE2;
    
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
    PCA0CPM0 |= PCA0CPM0_CAPP__ENABLED;
    PCA0CPM0 |= PCA0CPM0_CAPN__ENABLED;
}

void pca0_run(void)
{
    // enables the PCA counter/timer
    PCA0CN0_CR = 1;
}

void pca0_halt(void)
{
    PCA0CN0_CR = 0;
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