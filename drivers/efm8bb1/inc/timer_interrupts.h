/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMER_INTERRUPT_H_
#define INC_TIMER_INTERRUPT_H_

// hex(0xFFFF - (10*10^-6)/(1/24500000))
// hex(0xFFFF - (1*10^-3)/(1/24500000))
//#define TIMER0_RELOAD_1MILLIS   0xA04B
//#define TIMER1_RELOAD_10MICROS  0xFF0A

#define TIMER0_PCA0  0xA0
#define TIMER1_UART0 0xCB
#define TIMER2_RELOAD_10MICROS  0xFFEA

unsigned long get_time_milliseconds(void);
unsigned long get_time_ten_microseconds(void);

void init_timer2_us(const uint16_t interval, const uint16_t timeout);
void wait_timer2_finished(void);


#endif