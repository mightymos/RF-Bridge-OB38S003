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


extern unsigned long get_current_time(void);
extern unsigned long get_elapsed_time(unsigned long previousTime);
//extern void reload_timer1(unsigned int reload);
//extern void reload_timer1(unsigned char high, unsigned char low);



#endif