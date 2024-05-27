/*
 * ticks.h
 *
 *  Ported on: 02.17.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TICKS_H_
#define INC_TICKS_H_


//-----------------------------------------------------------------------------
// public variables
//-----------------------------------------------------------------------------

// public prototypes
unsigned long get_current_timer0(void);
unsigned long get_elapsed_timer0(unsigned long previousTime);
unsigned long get_current_timer1(void);
unsigned long get_elapsed_timer1(unsigned long previousTime);
unsigned long get_current_timer2(void);
unsigned long get_elapsed_timer2(unsigned long previousTime);


#endif // INC_TICKS_H_
