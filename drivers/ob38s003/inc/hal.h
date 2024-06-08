/*
 * hal.h - Hardware abstraction layer
 *
 *  Ported on: 02.16.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_HAL_H_
#define INC_HAL_H_

// Sonoff (white box)
#include "OB38S003.h"
#include "sonoffr22_pins.h"


#include <stdbool.h>
#include <stdint.h>
//#include <stdio.h>

// we place these directly in include file because
// if placed in source file they do not get inlined
// and so take up additional code space
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
    LED = 1;
}

inline void led_off(void)
{
    LED = 0;
}

inline void led_toggle(void)
{
    LED = !LED;
}


inline void radio_receiver_off(void)
{
    RF_ENABLE = 1;
}


inline void radio_receiver_on(void)
{
    RF_ENABLE = 0;
}

inline bool get_radio_wake(void)
{
    return RF_ENABLE;
}

inline void soft_tx_pin_on(void)
{
    SOFT_TX_PIN = 1;
}

inline void soft_tx_pin_off(void)
{
    SOFT_TX_PIN = 0;
}

inline void soft_tx_pin_toggle(void)
{
    SOFT_TX_PIN = !SOFT_TX_PIN;
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
    REN = 1;
}

inline void uart_rx_disabled(void)
{
    REN = 0;
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

extern bool get_radio_wake(void);
extern unsigned char get_stack_pointer(void);


extern void set_clock_1t_mode(void);
extern void set_clock_6t_mode(void);
extern void enable_watchdog(void);
extern void disable_watchdog(void);
extern void refresh_watchdog(void);
extern void init_port_pins(void);
extern void init_serial_interrupt(void);
extern void init_uart(void);
extern void init_timer0(const uint16_t);
extern void init_timer1(const uint16_t);
extern void init_timer2_as_capture(void);
extern void enable_capture_interrupt(void);
extern void disable_capture_interrupt(void);
extern void enable_serial_interrupt(void);
extern void disable_serial_interrupt(void);

extern bool global_interrupts_are_enabled(void);


extern void load_timer0(const uint16_t value);
extern void load_timer1(const uint16_t value);

extern uint16_t get_timer2(void);

extern void clear_capture_flag(void);

extern unsigned long countsToTime(const unsigned long duration);



#endif // INC_HAL_H_
