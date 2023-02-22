/*
 *
 *  Ported on: 02.16.2023
 *      Author: Jonathan Armstrong
 */
#include "..\inc\globals.h"


uint16_t Timer_2_Timeout;
uint16_t Timer_2_Interval;
uint16_t Timer_3_Timeout;
uint16_t Timer_3_Interval;


inline void led_on(void)
{
    LED = 1;
}

inline void led_off(void)
{
    LED = 0;
}

inline void led_toggle(void)
{
    LED = !LED;
}

inline void buzzer_on(void)
{
    BUZZER = 1;
}

inline void buzzer_off(void)
{
    BUZZER = 0;
}

inline void tdata_on(void)
{
    TDATA = 1;
}


inline void tdata_off(void)
{
    TDATA = 0;
}

inline bool isrdataHigh(void)
{
    return RDATA;
}

void SetTimer2Reload(uint16_t reload)
{
	// - Timer 2 Reload High Byte
//	TMR2RLH = (((reload >> 8) & 0xFF) << TMR2RLH_TMR2RLH__SHIFT);
	// - Timer 2 Reload Low Byte = 0x86
//	TMR2RLL = ((reload & 0xFF) << TMR2RLL_TMR2RLL__SHIFT);
}


/*
 * Init Timer 2 with microseconds interval, maximum is 65535µs.
 */
void InitTimer2_us(uint16_t interval, uint16_t timeout)
{
//	SetTimer2Reload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000000 / (uint32_t)interval))));

	// remove 65µs because of startup delay
//	Timer_2_Timeout = timeout - 65;
//	Timer_2_Interval = interval;

	// start timer
//	TMR2CN0 |= TMR2CN0_TR2__RUN;
}


/*
 * Init Timer 2 with milliseconds interval, maximum is ~2.5ms.
 */
void InitTimer2_ms(uint16_t interval, uint16_t timeout)
{
//	SetTimer2Reload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000 / (uint32_t)interval))));

//	Timer_2_Timeout = timeout;
//	Timer_2_Interval = interval;

	// start timer
//	TMR2CN0 |= TMR2CN0_TR2__RUN;
}


void WaitTimer2Finished(void)
{
	// wait until timer has finished
//	while((TMR2CN0 & TMR2CN0_TR2__BMASK) == TMR2CN0_TR2__RUN);
}



void StopTimer2(void)
{
	// stop timer
//	TMR2CN0 &= ~TMR2CN0_TR2__RUN;
	// Clear Timer 2 high overflow flag
//	TMR2CN0 &= ~TMR2CN0_TF2H__SET;
}


bool IsTimer2Finished(void)
{
//	return ((TMR2CN0 & TMR2CN0_TR2__BMASK) != TMR2CN0_TR2__RUN);
}

bool IsTimer3Finished(void)
{
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//SI_INTERRUPT (TIMER2_ISR, TIMER2_IRQn)
void timer2_isr(void) __interrupt (5)
{

}

