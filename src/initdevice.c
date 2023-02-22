//=========================================================
//=========================================================

// USER INCLUDES
//#include <SI_EFM8BB1_Register_Enums.h>
#include <8052.h>
#include <stdint.h>
#include "..\inc\globals.h"
#include "..\inc\initdevice.h"


void init_port_pins(void)
{
    // buzzer push pull
    P0M1 &= ~0x01;
    P0M0 |=  0x01;
    
    // uart tx push pull
    //P1M1 &= ~0x01;
    //P1M0 |=  0x01;
    
    // led push pull
    P3M1 &= ~0x01;
    P3M0 |=  0x01;
    
}


void init_uart(void)
{
    // BRGS = 1 so uart baud rate generator uses SREL (instead of timer 1)
    AUX |= 0x80;
    
    // mode 1, no parity bit SM0 = 0 and SM2 = 0 by default
    SM1 = 1;
    
	// uart reception enabled
    REN = 1;
    
    // see formula below with SMOD = 1
    PCON |= SMOD;
    
    // SRELPS0 = 1;
    PFCON |= 0x10;
    
    // pg. 43, sec. 8.4.1.2
    // baud rate = (2^SMOD x Fosc) / ((32 or 64) * (2^10 - SREL))
    // SRELPS[1:0] = 00 divisor is 64, 01 divisor is 32
    // (2^1 * 16000000)/(32*(2^10 - 920)) = 9615
    // FIXME: consider computing with macro dependent on clock frequency
    //SRELH = 0x03;
    //SRELL = 0x98;
    
    // 19200
    SRELH = 0x03;
    SRELL = 0xcc;
}


//================================================================================
//================================================================================
void init_interrupts(void)
{
    // enable serial interrupt
    ES = 1;
    
    // global enable interrupts
    EA = 1;

}


//================================================================================
//================================================================================
void init_timers(void)
{
	// save timer configuration
	//uint8_t TCON_save;
    //uint8_t TMR3CN0_TR3_save;
	//uint8_t TMR2CN0_TR2_save;


	//TCON_save = TCON;
    
	// stop timers
	//TCON &= ~TCON_TR0__BMASK & ~TCON_TR1__BMASK;

	// timer initialization

	// Timer 0 High Byte = 0xA0
	//TH0 = (0xA0 << TH0_TH0__SHIFT);
    TH0 = 0xA0;

	// - Timer 1 High Byte = 0xCB
	//TH1 = (0xCB << TH1_TH1__SHIFT);
    TH1 = 0xCB;


	// restore timer configuration
	//TCON |= (TCON_save & TCON_TR0__BMASK) | (TCON_save & TCON_TR1__BMASK);

	// Save Timer Configuration
	//TMR3CN0_TR3_save = TMR3CN0 & TMR3CN0_TR3__BMASK;
	// Stop Timer
	//TMR3CN0 &= ~(TMR3CN0_TR3__BMASK);


	// Restore Timer Configuration
	//TMR3CN0 |= TMR3CN0_TR3_save;
    
	// CKCON0 - Clock Control 0
	 // - System clock divided by 12
	 // - Counter/Timer 0 uses the system clock
	 // - Timer 2 high byte uses the clock defined by T2XCLK in TMR2CN0
	 // - Timer 2 low byte uses the system clock
	 // - Timer 3 high byte uses the clock defined by T3XCLK in TMR3CN0
	 // - Timer 3 low byte uses the system clock
	 // - Timer 1 uses the clock defined by the prescale field, SCA
	//CKCON0 = CKCON0_SCA__SYSCLK_DIV_12 | CKCON0_T0M__SYSCLK
	//		| CKCON0_T2MH__EXTERNAL_CLOCK | CKCON0_T2ML__SYSCLK
	//		| CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__SYSCLK
	//		| CKCON0_T1M__PRESCALE;

	// TMOD - Timer 0/1 Mode
	 // - Mode 2, 8-bit Counter/Timer with Auto-Reload
	 // - Mode 2, 8-bit Counter/Timer with Auto-Reload
	 // - Timer Mode
	 // - Timer 0 enabled when TR0 = 1 irrespective of INT0 logic level
	 // - Timer Mode
	 // - Timer 1 enabled when TR1 = 1 irrespective of INT1 logic level
	//TMOD = TMOD_T0M__MODE2 | TMOD_T1M__MODE2 | TMOD_CT0__TIMER
	//		| TMOD_GATE0__DISABLED | TMOD_CT1__TIMER | TMOD_GATE1__DISABLED;

	// start Timer 0 running
	// start Timer 1 running
	//TCON |= TCON_TR0__RUN | TCON_TR1__RUN;
    TR0 = 1;
    TR1 = 1;
    
	// Timer Initialization
	// Save Timer Configuration
	//TMR2CN0_TR2_save = TMR2CN0 & TMR2CN0_TR2__BMASK;
    
	// stop timer
	//TMR2CN0 &= ~(TMR2CN0_TR2__BMASK);


	// Timer Restoration
	// Restore Timer Configuration
	//TMR2CN0 |= TMR2CN0_TR2_save;
}


//================================================================================
// FIXME: add credit to GL if necessary
//================================================================================
void init_pca(void)
{
	// PCA Off
	//PCA0CN0_CR = PCA0CN0_CR__STOP;

	// PCA0MD - PCA Mode
	/***********************************************************************
	 - PCA continues to function normally while the system controller is in
	 Idle Mode
	 - Enable a PCA Counter/Timer Overflow interrupt request when CF is set
	 - Timer 0 overflow
	 ***********************************************************************/
	//PCA0MD = PCA0MD_CIDL__NORMAL | PCA0MD_ECF__OVF_INT_ENABLED
	//		| PCA0MD_CPS__T0_OVERFLOW;

	// PCA Counter/Timer Low Byte = 0xFF
	//PCA0L = (0xFF << PCA0L_PCA0L__SHIFT);


	// PCA0POL - PCA Output Polarity
	// - Invert polarity
	// - Use default polarity
	// - Use default polarity
	//PCA0POL = PCA0POL_CEX0POL__INVERT | PCA0POL_CEX1POL__DEFAULT
	//		| PCA0POL_CEX2POL__DEFAULT;
            
	// PCA0 Settings Save
	// Select Capture/Compare register
	//PCA0PWM &= ~PCA0PWM_ARSEL__BMASK;

	// PCA0CPM0 - PCA Channel 0 Capture/Compare Mode
	 // - Enable negative edge capture
	 // - Disable CCF0 interrupts
	 // - Disable match function
	 // - 8 to 11-bit PWM selected
	 // - Enable positive edge capture
	 // - Disable comparator function
	 // - Disable PWM function
	 // - Disable toggle function
	//PCA0CPM0 = PCA0CPM0_CAPN__ENABLED | PCA0CPM0_ECCF__DISABLED
	//		| PCA0CPM0_MAT__DISABLED | PCA0CPM0_PWM16__8_BIT
	//		| PCA0CPM0_CAPP__ENABLED | PCA0CPM0_ECOM__DISABLED
	//		| PCA0CPM0_PWM__DISABLED | PCA0CPM0_TOG__DISABLED;
    
    //T2    = d_T2value;
    CCEN  = 0x60;
    CCEN2 = 0x00;
    CCCON = 0x20;
    T2CON = 0x51;
    IEN0 |= 0xA0;
}