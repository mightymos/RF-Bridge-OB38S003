/*
 * Globals.h
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include "sonoffr20_pins.h"

#include <stdbool.h>
#include <stdint.h>
#include <EFM8BB1.h>


inline void buzzer_on(void)
{
    BUZZER = 1;
}

inline void buzzer_off(void)
{
    BUZZER = 0;
}

inline bool rdata_level(void)
{
    return RDATA;
}

// setter prototypes
inline void led_on(void)
{
	// sonoff bridge
    //LED = 1;
	// EFM8BB1LCK board
	LED = 0;
}

inline void led_off(void)
{
	// sonoff bridge
    //LED = 0;
	// EFM8BB1LCK board
	LED = 1;
}

inline void led_toggle(void)
{
    LED = !LED;
}

inline void radio_receiver_off(void)
{
    //FIXME: do not know if this hardware has radio enable pin requirement or not
}


inline void radio_receiver_on(void)
{
    //FIXME: do not know if this hardware has radio enable pin requirement or not
}

inline void tdata_on(void)
{
    TDATA = 1;
}


inline void tdata_off(void)
{
    TDATA = 0;
}

inline void uart_tx_pin_off(void)
{
    UART_TX_PIN = 0;
}


inline void uart_tx_pin_on(void)
{
    UART_TX_PIN = 1;
}

inline void uart_tx_pin_toggle(void)
{
    UART_TX_PIN = !UART_TX_PIN;
}

inline bool uart_rx_pin_level(void)
{
    return UART_RX_PIN;
}

inline void uart_rx_enabled(void)
{
	// uart reception enabled
    SCON0 |= REN__RECEIVE_ENABLED;
}

inline void uart_rx_disabled(void)
{
    SCON0 &= ~REN__RECEIVE_ENABLED;
}

inline void debug_pin0_on(void)
{
	DEBUG_PIN0 = 1;
}

inline void debug_pin0_off(void)
{
	DEBUG_PIN0 = 0;
}

inline void debug_pin0_toggle(void)
{
	DEBUG_PIN0 = !DEBUG_PIN0;
}

inline void debug_pin1_on(void)
{
	DEBUG_PIN1 = 1;
}

inline void debug_pin1_off(void)
{
	DEBUG_PIN1 = 0;
}

inline void debug_pin1_toggle(void)
{
	DEBUG_PIN1 = !DEBUG_PIN1;
}


inline void enable_global_interrupts(void)
{
    EA = 1;
}

inline void disable_global_interrupts(void)
{
    EA = 0;
}

inline void enable_timer0_interrupt(void)
{
    ET0 = 1;
}

inline void disable_timer0_interrupt(void)
{
    ET0 = 0;
}

inline void enable_timer1_interrupt(void)
{
    ET1 = 1;
}

inline void disable_timer1_interrupt(void)
{
    ET1 = 0;
}

inline void enable_timer2_interrupt(void)
{
	ET2 = 1;
}

inline void disable_timer2_interrupt(void)
{
	ET2 = 0;
}

inline void enable_timer3_interrupt(void)
{
	EIE1 |= ET3__ENABLED;
}

inline void disable_timer3_interrupt(void)
{
	EIE1 |= ET3__DISABLED;
}

extern void set_clock_mode(void);
extern void enable_watchdog(void);
extern void refresh_watchdog(void);
extern void init_port_pins(void);
extern void init_serial_interrupt(void);
extern void init_uart(void);
extern void init_timer0(const uint16_t);
extern void init_timer1(const uint8_t);
extern void init_timer2(const uint16_t);
extern void init_timer3(const uint16_t);

extern void load_timer0(const uint16_t value);
extern void load_timer1(const uint16_t value);

extern void pca0_init(void);
extern void pca0_run(void);
extern void pca0_halt(void);

extern void enable_capture_interrupt(void);
extern void disable_capture_interrupt(void);
extern void enable_serial_interrupt(void);
extern void disable_serial_interrupt(void);

extern bool global_interrupts_are_enabled(void);


extern unsigned char get_timer2_low(void);
extern unsigned char get_timer2_high(void);

extern void clear_capture_flag(void);

#endif // INC_GLOBALS_H_
