/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMER_INTERRUPT_H_
#define INC_TIMER_INTERRUPT_H_

//#define TH0_RELOAD_1MILLIS 0xC1
//#define TL0_RELOAD_1MILLIS 0x7F

// use separate defines to avoid any kind of bit manipulation for speed
// hex(0xFFFF - (10*10^-6)/(1/16000000)) = 65375 (0xFF7F)
#define TH1_RELOAD_10MICROS 0xFF
#define TL1_RELOAD_10MICROS 0x60

//uint16_t get_time_milliseconds(void);
//uint16_t get_time_ten_microseconds(void);

void init_timer1_us(const uint16_t interval, const uint16_t timeout);
void wait_timer1_finished(void);

#endif