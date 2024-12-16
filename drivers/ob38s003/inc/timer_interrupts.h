/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMER_INTERRUPT_H_
#define INC_TIMER_INTERRUPT_H_


// use separate defines to avoid any need for bit manipulation for speed

//hex(0xFF - (10*10^-6)/(1/16000000)) = 0x5F
#define TIMER0_RELOAD_10MICROS 0x5F

//hex(0xFF - (1*10^-3)/(1/(16000000/96))) = 0x58.55555555555555555555
#define TIMER0_RELOAD_1MILLIS 0x58

// we need to use the prescaler to allow both microsecond and millisecond timings
//hex(0xFF - (10*10^-6)/(1/16000000)) = 0x5F
#define TIMER1_RELOAD_10MICROS 0x5F

//hex(0xFF - (1*10^-3)/(1/(16000000/96))) = 0x58.55555555555555555555
#define TIMER1_RELOAD_1MILLIS 0x58

//
//void init_first_delay_us(const uint16_t timeout);
void init_first_delay_ms(const uint16_t timeout);
void init_second_delay_us(const uint16_t timeout);
void init_second_delay_ms(const uint16_t timeout);

void wait_first_delay_finished(void);
void wait_second_delay_finished(void);

void stop_first_delay(void);
void stop_second_delay(void);

bool is_first_delay_finished(void);
bool is_second_delay_finished(void);

void clear_interrupt_flags_pca(void);
void clear_pca_counter(void);

#endif