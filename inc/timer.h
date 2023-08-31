/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

//#define MAX_PERIOD_COUNT 48
//#define BUFFER_SIZE 24


extern unsigned long get_current_timer0(void);
extern unsigned long get_elapsed_timer0(unsigned long previousTime);

extern unsigned long get_current_timer1(void);
extern unsigned long get_elapsed_timer1(unsigned long previousTime);


#endif