/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMER_INTERRUPT_H_
#define INC_TIMER_INTERRUPT_H_




// original portisch used 0xA0 which seems incorrect (they used 0x0B later)
// and 0x0B value makes more sense:
// 1/((24500000)/(256-0x0B)) = 0.00001
#define TIMER0_PCA0  0x0B

// 19200 baud, same on portisch
#define TIMER1_UART0 0xCB

// 1 microsec
// hex(0xFFFF - (1*10^-6)/(1/24500000))  = 0xFFE6.8
// hex(0xFFFF - (1*10^-6)/(1/(24500000/12)))  = 0xFFFC.F5
//

// hex(0xFFFF - (10*10^-6)/(1/24500000)) = 0xFF0A
// hex(0xFFFF - (10*10^-6)/(1/(24500000/12))) = 0xFFEA.95
//
#define TIMER2_RELOAD_10MICROS  0xFFEB

// hex(0xFFFF - (1*10^-3)/(1/24500000))  = 0xA04B
// hex(0xFFFF - (1*10^-3)/(1/(24500000/12)))  = 0xF805
//
#define TIMER3_RELOAD_1MILLIS   0xF805


void init_delay_timer_us(const uint16_t timeout);
void init_delay_timer_ms(const uint16_t timeout);
void wait_delay_timer_us_finished(void);
void wait_delay_timer_ms_finished(void);
void stop_delay_timer_us(void);
void stop_delay_timer_ms(void);
bool is_delay_timer_us_finished(void);
bool is_delay_timer_ms_finished(void);

void clear_interrupt_flags_pca(void);
void clear_pca_counter(void);

#endif