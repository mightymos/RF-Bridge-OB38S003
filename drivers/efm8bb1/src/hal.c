/*
 * hal.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */
#include <stdint.h>
#include <stdbool.h>

#include <EFM8BB1.h>

#if 0
    // FIXME: it would be more useful to eventually have tick system like rtos/rcswitch style
__xdata uint16_t Timer_2_Timeout  = 0x0000;
__xdata uint16_t Timer_2_Interval = 0x0000;
__xdata uint16_t Timer_3_Timeout  = 0x0000;
__xdata uint16_t Timer_3_Interval = 0x0000;

// useful delay() discussion using timer2 for this processor
// https://forum.digikey.com/t/how-to-program-a-blocking-delay-function-in-the-8051-microcontroller/37051
#endif

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

void init_port_pins(void)
{
	// FIXME: add comment to explain pin functions
	P0MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN | B2__OPEN_DRAIN | B3__OPEN_DRAIN | B4__PUSH_PULL | B5__OPEN_DRAIN | B6__OPEN_DRAIN | B7__OPEN_DRAIN;

	// add explanation
	P0SKIP = B0__SKIPPED | B1__SKIPPED | B2__SKIPPED | B3__SKIPPED | B4__NOT_SKIPPED | B5__NOT_SKIPPED | B6__SKIPPED | B7__SKIPPED;

	// FIXME: correctly handle LED on sonoff different from LED on EFM8BB1LCK board
	P1MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN | B2__OPEN_DRAIN | B3__OPEN_DRAIN | B4__PUSH_PULL | B5__PUSH_PULL | B6__PUSH_PULL | B7__PUSH_PULL;


	P1SKIP = B0__SKIPPED | B1__SKIPPED | B2__SKIPPED | B3__NOT_SKIPPED | B4__SKIPPED | B5__SKIPPED | B6__SKIPPED | B7__SKIPPED;


	// $[XBR2 - Port I/O Crossbar 2]
	/***********************************************************************
	 - Weak Pullups enabled 
	 - Crossbar enabled
	 ***********************************************************************/
	XBR2 = WEAKPUD__PULL_UPS_ENABLED | XBARE__ENABLED;


	// $[XBR0 - Port I/O Crossbar 0]
	/***********************************************************************
	 - UART TX, RX routed to Port pins P0.4 and P0.5
	 - SPI I/O unavailable at Port pins
	 - SMBus 0 I/O unavailable at Port pins
	 - CP0 unavailable at Port pin
	 - Asynchronous CP0 unavailable at Port pin
	 - CP1 unavailable at Port pin
	 - Asynchronous CP1 unavailable at Port pin
	 - SYSCLK unavailable at Port pin
	 ***********************************************************************/
	XBR0 = URT0E__ENABLED | SPI0E__DISABLED | SMB0E__DISABLED | CP0E__DISABLED | CP0AE__DISABLED | CP1E__DISABLED | CP1AE__DISABLED | SYSCKE__DISABLED;

	// $[XBR1 - Port I/O Crossbar 1]
	/***********************************************************************
	 - CEX0 routed to Port pin
	 - ECI unavailable at Port pin
	 - T0 unavailable at Port pin
	 - T1 unavailable at Port pin
	 - T2 unavailable at Port pin
	 ***********************************************************************/
	XBR1 = PCA0ME__CEX0 | ECIE__DISABLED | T0E__DISABLED | T1E__DISABLED | T2E__DISABLED;
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