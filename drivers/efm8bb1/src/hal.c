/*
 * Globals.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */

#include <stdint.h>
#include <EFM8BB1.h>


#include "globals.h"

__xdata uint16_t Timer_2_Timeout  = 0x0000;
__xdata uint16_t Timer_2_Interval = 0x0000;
__xdata uint16_t Timer_3_Timeout  = 0x0000;
__xdata uint16_t Timer_3_Interval = 0x0000;



void SetTimer2Reload(uint16_t reload)
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

#if 0
void SetTimer3Reload(uint16_t reload)
{
	/***********************************************************************
	 - Timer 3 Reload High Byte
	 ***********************************************************************/
	TMR3RLH = ((reload >> 8) & 0xFF);
	/***********************************************************************
	 - Timer 3 Reload Low Byte = 0x86
	 ***********************************************************************/
	TMR3RLL = (reload & 0xFF);
}
#endif

/*
 * Init Timer 2 with microseconds interval, maximum is 65535탎.
 */
void InitTimer2_us(uint16_t interval, uint16_t timeout)
{
	SetTimer2Reload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000000 / (uint32_t)interval))));

	// remove 65탎 because of startup delay
	Timer_2_Timeout = timeout - 65;
	Timer_2_Interval = interval;

	// start timer
	TMR2CN0 |= TR2__RUN;
}

#if 0
/*
 * Init Timer 3 with microseconds interval, maximum is 65535탎.
 */
void InitTimer3_us(uint16_t interval, uint16_t timeout)
{
	SetTimerReload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000000 / (uint32_t)interval))));

	// remove 65탎 because of startup delay
	Timer_3_Timeout = timeout - 65;
	Timer_3_Interval = interval;

	// start timer
	TMR3CN0 |= TR3__RUN;
}
#endif

/*
 * Init Timer 2 with milliseconds interval, maximum is ~2.5ms.
 */
void InitTimer2_ms(uint16_t interval, uint16_t timeout)
{
	SetTimer2Reload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000 / (uint32_t)interval))));

	Timer_2_Timeout = timeout;
	Timer_2_Interval = interval;

	// start timer
	TMR2CN0 |= TR2__RUN;
}

#if 0
/*
 * Init Timer 3 with milliseconds interval, maximum is ~2.5ms.
 */
void InitTimer3_ms(uint16_t interval, uint16_t timeout)
{
	SetTimerReload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000 / (uint32_t)interval))));

	Timer_3_Timeout = timeout;
	Timer_3_Interval = interval;

	// start timer
	TMR3CN0 |= TR3__RUN;
}
#endif

void WaitTimer2Finished(void)
{
	// wait until timer has finished
	while((TMR2CN0 & TR2__BMASK) == TR2__RUN);
}

#if 0
void WaitTimer3Finished(void)
{
	// wait until timer has finished
	while((TMR3CN0 & TR3__BMASK) == TR3__RUN);
}
#endif

#if 1
void StopTimer2(void)
{
	// stop timer
	TMR2CN0 &= ~TR2__RUN;
	// Clear Timer 2 high overflow flag
	TMR2CN0 &= ~TF2H__SET;
}
#endif

#if 0
void StopTimer3(void)
{
	// stop timer
	TMR3CN0 &= ~TR3__RUN;
	// Clear Timer 3 high overflow flag
	TMR3CN0 &= ~TF3H__SET;
}
#endif

bool IsTimer2Finished(void)
{
	return ((TMR2CN0 & TR2__BMASK) != TR2__RUN);
}

#if 0
bool IsTimer3Finished(void)
{
	return ((TMR3CN0 & TR3__BMASK) != TR3__RUN);
}
#endif

//-----------------------------------------------------------------------------
// TIMER2_ISR
//-----------------------------------------------------------------------------
//
// TIMER2 ISR Content goes here. Remember to clear flag bits:
// TMR2CN0::TF2H (Timer # High Byte Overflow Flag)
// TMR2CN0::TF2L (Timer # Low Byte Overflow Flag)
//
//-----------------------------------------------------------------------------
void TIMER2_ISR(void) __interrupt (5)
{
	// Clear Timer 2 high overflow flag
	TMR2CN0 &= ~TF2H__SET;

	// check if pulse time is over
	if(Timer_2_Timeout == 0)
	{
		// stop timer
		TMR2CN0 &= ~TR2__RUN;
	}

	if (Timer_2_Timeout < Timer_2_Interval)
		Timer_2_Timeout = 0;
	else
		Timer_2_Timeout -= Timer_2_Interval;
}

#if 0
//-----------------------------------------------------------------------------
// TIMER3_ISR
//-----------------------------------------------------------------------------
//
// TIMER3 ISR Content goes here. Remember to clear flag bits:
// TMR3CN0::TF3H (Timer # High Byte Overflow Flag)
// TMR3CN0::TF3L (Timer # Low Byte Overflow Flag)
//
//-----------------------------------------------------------------------------
void TIMER3_ISR(void) __interrupt (TIMER3_IRQn)
{
	// Clear Timer 3 high overflow flag
	TMR3CN0 &= ~TF3H__SET;

	// check if pulse time is over
	if(Timer_3_Timeout == 0)
	{
		// stop timer
		TMR3CN0 &= ~TR3__RUN;
	}

	if (Timer_3_Timeout < Timer_3_Interval)
		Timer_3_Timeout = 0;
	else
		Timer_3_Timeout -= Timer_3_Interval;
}
#endif
