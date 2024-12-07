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
// hex(0xFFFF - (10*10^-6)/(1/16000000)) = 65375 (0xFF5F)
//#define TIMER1_RELOAD_10MICROS  0xFFEA
//hex(0xFF - (10*10^-6)/(1/16000000)) = 0x5F
//#define TIMER1_RELOAD_10MICROS 0x5F

// we need to use the prescaler to allow both microsecond and millisecond timings
// hex(0xFF - (10*10^-6)/(1/(16000000/96))) = 0xFD.55555555555555555555
#define TIMER1_RELOAD_10MICROS 0xFD

//hex(0xFF - (1*10^-3)/(1/(16000000/96))) = 0x58.55555555555555555555
#define TIMER1_RELOAD_1MILLIS 0x58

void init_delay_timer_us(const uint16_t interval, const uint16_t timeout);
void init_delay_timer_ms(const uint16_t interval, const uint16_t timeout);
void wait_delay_timer_finished(void);
void stop_delay_timer(void);
bool is_delay_timer_finished(void);

void clear_interrupt_flags_pca(void);
void clear_pca_counter(void);

#endif