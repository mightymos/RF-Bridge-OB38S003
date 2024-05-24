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
	CKCON0 = SCA__SYSCLK_DIV_12 | T0M__SYSCLK | T2MH__EXTERNAL_CLOCK | T2ML__SYSCLK | T3MH__EXTERNAL_CLOCK | T3ML__SYSCLK | T1M__PRESCALE;
}


// pg. 44 - once the watchdog is started it cannot be stopped
void enable_watchdog(void)
{
    //timebase = timebase;
    //idleState = idleState;
    //WDTCN = interval;
	WDTCN = 0xA5;
}

void refresh_watchdog(void)
{
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

	// $[P1SKIP - Port 1 Skip]
	/***********************************************************************
	 - P1.0 pin is skipped by the crossbar
	 - P1.1 pin is skipped by the crossbar
	 - P1.2 pin is skipped by the crossbar
	 - P1.3 pin is not skipped by the crossbar
	 - P1.4 pin is skipped by the crossbar
	 - P1.5 pin is skipped by the crossbar
	 - P1.6 pin is skipped by the crossbar
	 ***********************************************************************/
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


void init_serial_interrupt(void)
{
    TI = 1;
}

void init_capture_interrupt(void)
{
	EIE1 |= EPCA0__ENABLED;
}

void disable_capture_interrupt(void)
{
	EIE1 &= ~EPCA0__ENABLED;
}

void enable_capture_interrupt(void)
{
	EIE1 |= EPCA0__ENABLED;
}

void init_timer0(const uint16_t value)
{
    // pg. 194  Setting the TR0 bit enables the timer when either GATE0 in the TMOD register is logic 0
	TMOD = T0M__MODE2 | T1M__MODE2 | CT0__TIMER | GATE0__DISABLED | CT1__TIMER | GATE1__DISABLED;
	
    TH0 = (value >> 8) & 0xff;
    TL0 = value & 0xff;
	
	// enable timer
	TR0 = true;
}

void init_timer1(const uint16_t value)
{
	// FIXME: actually do what function name says
	TMOD = T0M__MODE2 | T1M__MODE2 | CT0__TIMER | GATE0__DISABLED | CT1__TIMER | GATE1__DISABLED;
	
    TH1 = (value >> 8) & 0xff;
    TL1 = value & 0xff;
	
	// enable timer
	TR1 = true;
}

void pca0_init(void)
{
	// 
	PCA0CN0 = CR__STOP;


	// $[PCA0MD - PCA Mode]
	/***********************************************************************
	 - PCA continues to function normally while the system controller is in
	 Idle Mode
	 - Enable a PCA Counter/Timer Overflow interrupt request when CF is set
	 - Timer 0 overflow
	 ***********************************************************************/
	PCA0MD = CIDL__NORMAL | ECF__OVF_INT_ENABLED | CPS__T0_OVERFLOW;

	/***********************************************************************
	 - PCA Counter/Timer Low Byte = 0xFF
	 ***********************************************************************/
	//PCA0L = (0xFF << PCA0L_PCA0L__SHIFT);
    PCA0L = 0xFF;

	/***********************************************************************
	 - Invert polarity
	 - Use default polarity
	 - Use default polarity
	 ***********************************************************************/
	PCA0POL = CEX0POL__INVERT | CEX1POL__DEFAULT | CEX2POL__DEFAULT;


	// FIXME: comment
	PCA0PWM &= ~ARSEL__BMASK;


	// $[PCA0CPM0 - PCA Channel 0 Capture/Compare Mode]
	/***********************************************************************
	 - Enable negative edge capture
	 - Disable CCF0 interrupts
	 - Disable match function
	 - 8 to 11-bit PWM selected
	 - Enable positive edge capture
	 - Disable comparator function
	 - Disable PWM function
	 - Disable toggle function
	 ***********************************************************************/
	PCA0CPM0 = CAPN__ENABLED | ECCF__DISABLED | MAT__DISABLED | PWM16__8_BIT | CAPP__ENABLED | ECOM__DISABLED | PWM__DISABLED | TOG__DISABLED;
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

void load_timer0(const unsigned int value)
{
	TH0 = (value >> 8) & 0xFF;
	TL0 = value & 0xFF;
}

void load_timer1(const unsigned int value)
{
	TH1 = (value >> 8) & 0xFF;
	TL1 = value & 0xFF;
}

unsigned char get_timer2_low(void)
{
	return TMR2L;
}

unsigned char get_timer2_high(void)
{
	return TMR2H;
}

void clear_ccp1_flag(void)
{
	CCF1 = 0;
}