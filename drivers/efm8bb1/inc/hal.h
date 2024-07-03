/*
 * Globals.h
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#ifndef INC_HAL_H_
#define INC_HAL_H_

#if defined(TARGET_BOARD_EFM8BB1LCB)
    #include "efm8bb1lck_pins.h"
#elif defined(TARGET_BOARD_EFM8BB1)
    #include "sonoffr20_pins.h"
#endif

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

inline void set_tdata(const bool state)
{
    TDATA = state;
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

inline void soft_tx_pin_on(void)
{
    SOFT_TX_PIN = 1;
}

inline void soft_tx_pin_off(void)
{
    SOFT_TX_PIN = 0;
}

inline void set_soft_tx_pin(const bool state)
{
    SOFT_TX_PIN = state;
}

inline void soft_tx_pin_toggle(void)
{
    SOFT_TX_PIN = !SOFT_TX_PIN;
}

inline bool get_soft_rx_pin(void)
{
    // FIXME: undefined for now because no pins available
    //        so force default of nothing received
    //return BUZZER;
    return true;
}

inline void debug_pin01_on(void)
{
    DEBUG_PIN01 = 1;
}

inline void debug_pin01_off(void)
{
    DEBUG_PIN01 = 0;
}

inline void set_debug_pin01(const bool state)
{
    DEBUG_PIN01 = state;
}

inline void debug_pin01_toggle(void)
{
    DEBUG_PIN01 = !DEBUG_PIN01;
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
extern void disable_watchdog(void);
extern void refresh_watchdog(void);
extern void init_port_pins_for_passthrough(void);
extern void init_port_pins_for_serial(void);
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

extern unsigned long countsToTime(const unsigned long duration);


extern void InitTimer2_us(const uint16_t interval, const uint16_t timeout);
extern void InitTimer3_us(const uint16_t interval, const uint16_t timeout);
extern void InitTimer2_ms(const uint16_t interval, const uint16_t timeout);
extern void InitTimer3_ms(const uint16_t interval, const uint16_t timeout);
extern void WaitTimer2Finished(void);
extern void WaitTimer3Finished(void);
extern void StopTimer2(void);
extern void StopTimer3(void);
extern bool IsTimer2Finished(void);
extern bool IsTimer3Finished(void);


#endif // INC_HAL_H_
