/*
 * hal.h - Hardware abstraction layer
 *
 *  Ported on: 02.16.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_HAL_H_
#define INC_HAL_H_

#include <stdbool.h>
//#include <stdint.h>
//#include <stdio.h>


extern void buzzer_on(void);
extern void buzzer_off(void);
extern bool get_radio_wake(void);
extern unsigned char get_stack_pointer(void);
extern void led_on(void);
extern void led_off(void);
extern void led_toggle(void);
extern void radio_receiver_off(void);
extern void radio_receiver_on(void);
extern bool rdata_level(void);
extern void reset_pin_on(void);
extern void reset_pin_off(void);
extern void reset_pin_toggle(void);
extern void tdata_on(void);
extern void tdata_off(void);
extern void uart_tx_pin_off(void);
extern void uart_tx_pin_on(void);
extern void uart_tx_pin_toggle(void);


extern void set_clock_1t_mode(void);
extern void set_clock_6t_mode(void);
extern void enable_watchdog(void);
extern void refresh_watchdog(void);
extern void init_port_pins(void);
extern void init_serial_interrupt(void);
extern void init_capture_interrupt(void);
extern void init_uart(void);
extern void init_timer0(void);
extern void init_timer1(void);
extern void init_timer2_capture(void);
extern void enable_capture_interrupt(void);
extern void disable_capture_interrupt(void);
extern void enable_global_interrupts(void);
extern void disable_global_interrupts(void);
extern bool global_interrupts_are_enabled(void);

extern void disable_timer0_interrupt(void);
extern void enable_timer0_interrupt(void);

extern void disable_timer1_interrupt(void);
extern void enable_timer1_interrupt(void);

extern void load_timer0(unsigned int value);
extern void load_timer1(unsigned int value);

extern unsigned char get_timer2_low(void);
extern unsigned char get_timer2_high(void);

extern void clear_ccp1_flag(void);


#endif // INC_HAL_H_
