/*
 * hal.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */
#include <stdint.h>
#include <stdbool.h>

#include <EFM8BB1.h>

//==============================================================================
// enter_DefaultMode_from_RESET
//==============================================================================
void enter_DefaultMode_from_RESET(void)
{

	// FIXME: add comment to explain pin functions
	P0MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN | B2__OPEN_DRAIN | B3__OPEN_DRAIN | B4__PUSH_PULL | B5__OPEN_DRAIN | B6__OPEN_DRAIN | B7__OPEN_DRAIN;

	// add explanation
	P0SKIP = B0__SKIPPED | B1__SKIPPED | B2__SKIPPED | B3__SKIPPED | B4__NOT_SKIPPED | B5__NOT_SKIPPED | B6__SKIPPED | B7__SKIPPED;



	// FIXME: correctly handle LED on sonoff different from LED on EFM8BB1LCK board
	P1MDOUT = B0__PUSH_PULL | B1__OPEN_DRAIN| B2__OPEN_DRAIN | B3__OPEN_DRAIN| B4__PUSH_PULL | B5__PUSH_PULL| B6__PUSH_PULL | B7__PUSH_PULL;

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
	P1SKIP = B0__SKIPPED | B1__SKIPPED | B2__SKIPPED| B3__NOT_SKIPPED | B4__SKIPPED | B5__SKIPPED| B6__SKIPPED | B7__SKIPPED;


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


    //*****************************************
	// - Clock derived from the Internal High-Frequency Oscillator
	// - SYSCLK is equal to selected clock source divided by 1
	//*****************************************
	CLKSEL = CLKSL__HFOSC | CLKDIV__SYSCLK_DIV_1;


	// FIXME: what is this timer used for?
    // FIXME: what are the shifts used for?
	//TH0 = (0xA0 << TH0_TH0__SHIFT);
    //TH0 = 0xA0;
	// FIXME: we later load this with 0x0B;
	// 1/((24500000)/(256-0x0B)) = 0.00001
	// which seems to correspond with 10 microsec counts
    TH0 = 0x0B;

	// FIXME:
    // sec. 19.3.1 Baud Rate Generation uart0 baud rate is set by timer 1 in 8-bit auto-reload mode
    //TH1 = (0xCB << TH1_TH1__SHIFT);
    TH1 = 0xCB;

	// FIXME: we removed timer3 resource to save on code space



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


	/***********************************************************************
	 - Mode 2, 8-bit Counter/Timer with Auto-Reload
	 - Mode 2, 8-bit Counter/Timer with Auto-Reload
	 - Timer Mode
	 - Timer 0 enabled when TR0 = 1 irrespective of INT0 logic level
	 - Timer Mode
	 - Timer 1 enabled when TR1 = 1 irrespective of INT1 logic level
	 ***********************************************************************/
    // pg. 194  Setting the TR0 bit enables the timer when either GATE0 in the TMOD register is logic 0
	TMOD = T0M__MODE2 | T1M__MODE2 | CT0__TIMER | GATE0__DISABLED | CT1__TIMER | GATE1__DISABLED;


	/***********************************************************************
	 - Start Timer 0 running
	 - Start Timer 1 running
	 ***********************************************************************/
	TCON |= TR0__RUN | TR1__RUN;

    
	// Stop Timer
	TMR2CN0 &= ~(TR2__BMASK);


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
	 // FIXME: invert polarity seems to apply to output mode only
	 // so why was this inverted?
	 // standard decoding appears to continue working with default (non-inverting)
	//PCA0POL = CEX0POL__INVERT | CEX1POL__DEFAULT | CEX2POL__DEFAULT;


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


	// $[EIE1 - Extended Interrupt Enable 1]
	/***********************************************************************
	 - Disable ADC0 Conversion Complete interrupt
	 - Disable ADC0 Window Comparison interrupt
	 - Disable CP0 interrupts
	 - Disable CP1 interrupts
	 - Disable all Port Match interrupts
	 - Enable interrupt requests generated by PCA0
	 - Disable all SMB0 interrupts
	 - Enable interrupt requests generated by the TF3L or TF3H flags
	 ***********************************************************************/
	EIE1 = EADC0__DISABLED | EWADC0__DISABLED | ECP0__DISABLED | ECP1__DISABLED | EMAT__DISABLED | EPCA0__ENABLED | ESMB0__DISABLED | ET3__ENABLED;

	/***********************************************************************
	 - Enable each interrupt according to its individual mask setting
	 - Disable external interrupt 0
	 - Disable external interrupt 1
	 - Disable all SPI0 interrupts
	 - Disable all Timer 0 interrupt
	 - Disable all Timer 1 interrupt
	 - Enable interrupt requests generated by the TF2L or TF2H flags
	 - Enable UART0 interrupt
	 ***********************************************************************/
	IE = EA__ENABLED | EX0__DISABLED | EX1__DISABLED | ESPI0__DISABLED | ET0__DISABLED | ET1__DISABLED | ET2__ENABLED | ES0__ENABLED;
}

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
// i.e., flag is set as though the last character sent was completed, even though no previous character was actually sent
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
    EIE1     |= EPCA0__ENABLED;
}

void disable_capture_interrupt(void)
{
    PCA0CPM0 &= ~ECCF__ENABLED;
    EIE1     &= ~EPCA0__ENABLED;
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
    // default source is system clock divided by 12
    // default pca CF overflow is disabled
    //PCA0MD &= ~CPS__SYSCLK_DIV_12;
    
    // enable both positive and negative edge triggers
    PCA0CPM0 |= CAPP__ENABLED;
    PCA0CPM0 |= CAPN__ENABLED;
}

void pca0_run(void)
{
    CR = true;
}

void pca0_halt(void)
{
    CR = false;
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

// the time constant is explained in the rcswitch.c file

unsigned long countsToTime(const unsigned long duration)
{
    unsigned long converted;
    
    converted = duration / 2;
    
    return converted;
}