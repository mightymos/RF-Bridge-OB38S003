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

inline void set_led(const bool state)
{
    LED = state;
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
    RESET_PIN = 1;
}

inline void soft_tx_pin_off(void)
{
    RESET_PIN = 0;
}

inline void soft_tx_pin_toggle(void)
{
    RESET_PIN = !RESET_PIN;
}

inline void set_soft_tx_pin(const bool state)
{
    RESET_PIN = state;
}

inline bool get_soft_rx_pin(void)
{
    // FIXME: undefined for now because no pins available
    //        so force default of nothing received
    //return BUZZER;
    return true;
}

inline void tdata_on(void)
{
    TDATA = 1;
}


inline void tdata_off(void)
{
    TDATA = 0;
}

inline void set_tdata(const bool state)
{
    TDATA = state;
}

inline void debug_pin01_on(void)
{
    // blank function until pin is assigned
    //__asm
    //  nop
    //__endasm;
    // buzzer pin is inverted
    BUZZER = 0;
}

inline void debug_pin01_off(void)
{
    // buzzer has an external transistor which inverts output
    BUZZER = 1;
}

inline void set_debug_pin01(const bool state)
{
    BUZZER = !state;
}

inline void debug_pin01_toggle(void)
{
    // blank function until pin is assigned
    BUZZER = !BUZZER;
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

inline void timer0_run(void)
{
    // enable timer0
    TR0 = 1;
}

inline void timer0_stop(void)
{
    // disable timer0
    TR0 = 0;
}

extern void set_clock_mode(void);
extern void enable_watchdog(void);
extern void disable_watchdog(void);
extern void refresh_watchdog(void);
extern void reset_mcu(void);
extern void init_port_pins(void);
extern void init_serial_interrupt(void);
extern void init_uart(void);
extern void init_timer0_8bit_autoreload(void);
extern void init_timer1_8bit_autoreload(void);
extern void init_timer2_as_capture(void);
extern void enable_capture_interrupt(void);
extern void disable_capture_interrupt(void);
extern void enable_serial_interrupt(void);
extern void disable_serial_interrupt(void);

void pca0_run(void);
void pca0_halt(void);

extern uint16_t get_capture_mode(void);

extern void clear_capture_flag(void);

extern uint16_t countsToTime(const uint16_t duration);



#endif // INC_HAL_H_
