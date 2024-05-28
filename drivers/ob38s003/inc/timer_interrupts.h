/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMER_INTERRUPT_H_
#define INC_TIMER_INTERRUPT_H_

#define TIMER0_RELOAD_1MILLIS   0xC17F

// hex(0xFFFF - (10*10^-6)/(1/16000000))
#define TIMER1_RELOAD_10MICROS  0xFF5F

unsigned long get_time_milliseconds(void);
unsigned long get_time_ten_microseconds(void);

#endif