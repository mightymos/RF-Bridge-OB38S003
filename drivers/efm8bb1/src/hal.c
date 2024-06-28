/*
 * hal.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */
#include <stdint.h>
#include <stdbool.h>

#include <EFM8BB1.h>

// pg. 3 of OB38S003 datasheet
// high speed architecture of 1 clock/machine cycle runs up to 16MHz.
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
	CKCON0 |= SCA__SYSCLK_DIV_12;
	CKCON0 |= T1M__PRESCALE;
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
	
	
	// UART TX, RX routed to Port pins P0.4 and P0.5
	//XBR0 |= URT0E__ENABLED;
	
	// CEX0 routed to port pin
	//XBR1 |= PCA0ME__CEX0;
	
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

void init_uart(void)
{	
    SCON0 &= ~(SMODE__BMASK | MCE__BMASK | REN__BMASK);
	SCON0 = REN__RECEIVE_ENABLED | SMODE__8_BIT | MCE__MULTI_DISABLED;
}

// this is necessary so that uart ring buffer logic operates correctly the first time it is used
// i.e., appears that last character sent was completed, even though no previous character was actually sent
void init_serial_interrupt(void)
{
    TI = 1;
}

void enable_serial_interrupt(void)
{
    ES0 = 1;
}

void disable_serial_interrupt(void)
{
    ES0 = 0;
}

void enable_capture_interrupt(void)
{
	PCA0CPM0 |= ECCF__ENABLED;
	EIE1 |= EPCA0__ENABLED;
}

void disable_capture_interrupt(void)
{
	PCA0CPM0 &= ~ECCF__ENABLED;
	EIE1 &= ~EPCA0__ENABLED;
}


void init_timer0(const uint16_t value)
{
	// 16-bit
	TMOD |= T0M__MODE1;

	TH0 = (value >> 8) & 0xff;
	TL0 = value & 0xff;

	// enable timer
	TR0 = 1;
}

void init_timer1(const uint8_t value)
{
	// 8-bit Counter/Timer with Auto-Reload
	TMOD |= T1M__MODE2;
	
    TH1 = value;
	
	// enable timer
	TR1 = 1;
}

void init_timer2(const uint16_t value)
{
	TMR2RLH = (value >> 8) & 0xff;
	TMR2RLL = value & 0xff;
	
	// default is 16 bit auto reload mode
	// default clock source is system clock divided by 12
	
	// timer 2 on
	TR2 = 1;
}

void init_timer3(const uint16_t value)
{
	TMR3RLH = (value >> 8) & 0xff;
	TMR3RLL = value & 0xff;
	
	// default is 16 bit auto reload mode
	// default clock source is system clock divided by 12
	
	// timer 3 on
	TMR3CN0 |= TR3__RUN;
}

void load_timer0(const uint16_t value)
{
    TH0 = (value >> 8) & 0xff;
    TL0 = value & 0xff;
}

void load_timer1(const uint16_t value)
{
    TH1 = (value >> 8) & 0xff;
    TL1 = value & 0xff;
}

void pca0_init(void)
{
	// default pca acts normally when controller is idle
	// default pca CF overflow is disabled
	PCA0MD |= CPS__SYSCLK_DIV_12;
	
	// enable both positive and negative edge triggers
	PCA0CPM0 |= CAPP__ENABLED;
	PCA0CPM0 |= CAPN__ENABLED;
}

void pca0_run(void)
{
    PCA0CN0 |= CR__RUN;
}

void pca0_halt(void)
{
    PCA0CN0 &= ~CR__RUN;
}


bool global_interrupts_are_enabled(void)
{
	return EA;
}

unsigned char get_timer2_low(void)
{
	return TMR2L;
}

unsigned char get_timer2_high(void)
{
	return TMR2H;
}

void clear_capture_flag(void)
{
	//PCA0CN0 &= ~CF__SET;
	CCF0 = 0;
}

// FIXME: on counts to time conversion
unsigned long countsToTime(const unsigned long duration)
{
    unsigned long converted;
    converted = duration * 2;
    
    return converted;
}