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
// for 1 microsecond
// hex(256 - (1*10^-6)/(1/(24500000))) = 0xE7.8
//#define TIMER0_PCA0  0xE8
#define TIMER0_PCA0  0x0B

// 19200 baud, same on portisch
//(24500000/12)/(256-0xCB)/2 = 19261.00628930817610062893
// note: this assumes the default system clock divided by 12
//#define TIMER1_UART0 0xCB

// 115200 baud
//(24500000/4)/(256-0xE5)/2 = 113425.92592592592592592593
// note: this requires system clock divided by 4
#define TIMER1_UART0 0xE5

// 1 microsec
// hex(0xFFFF - (1*10^-6)/(1/24500000))  = 0xFFE6.8
// hex(0xFFFF - (1*10^-6)/(1/(24500000/12)))  = 0xFFFC.F5
//

// 10 microsecs
// T2XCLK default is system clock divided by 12
// hex(0xFFFF - (10*10^-6)/(1/24500000)) = 0xFF0A
// hex(0xFFFF - (10*10^-6)/(1/(24500000/12))) = 0xFFEA.95
//
#define TIMER2_RELOAD_10MICROS  0xFFEB
#define TIMER2_RELOAD_1MILLIS   0xF805

// 1 milliseconds
// T3XCLK default is system clock divided by 12
// hex(0xFFFF - (1*10^-3)/(1/24500000))  = 0xA04B
// hex(0xFFFF - (1*10^-3)/(1/(24500000/12)))  = 0xF805
//
#define TIMER3_RELOAD_10MICROS  0xFFEB
#define TIMER3_RELOAD_1MILLIS   0xF805

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