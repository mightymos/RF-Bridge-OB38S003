/*
 * Globals.h
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#ifndef INC_HAL_H_
#define INC_HAL_H_

#if defined(TARGET_MCU_EFM8BB52)
    #include "efm8bb52_brd2701a_pins.h"
#else
    #error TARGET_MCU is undefined in makefile or inconsistent with board type
#endif

#include <stdbool.h>
#include <stdint.h>

#include "SI_EFM8BB52_Register_Enums.h"


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
    // BB52 Explorer Kit
    // schematic: https://www.silabs.com/documents/public/schematic-files/BRD2701A-A01-schematic.pdf
    LED = 1;
}

inline void led_off(void)
{
    // BB52 Explorer Kit
    LED = 0;
}


inline void led_toggle(void)
{
    LED = !LED;
}

inline void set_led(const bool state)
{
    LED = state;
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
    SCON0 |= SCON0_REN__RECEIVE_ENABLED;
}

inline void uart_rx_disabled(void)
{
    SCON0 &= ~SCON0_REN__RECEIVE_ENABLED;
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
    IE_EA = 1;
}

inline void disable_global_interrupts(void)
{
    IE_EA = 0;
}

inline void enable_timer0_interrupt(void)
{
    IE_ET0 = 1;
}

inline void disable_timer0_interrupt(void)
{
    IE_ET0 = 0;
}

inline void enable_timer1_interrupt(void)
{
    IE_ET1 = 1;
}

inline void disable_timer1_interrupt(void)
{
    IE_ET1 = 0;
}

inline void enable_timer2_interrupt(void)
{
    IE_ET2 = 1;
}

inline void disable_timer2_interrupt(void)
{
    IE_ET2 = 0;
}

inline void enable_timer3_interrupt(void)
{
    EIE1 |= EIE1_ET3__ENABLED;
}

inline void disable_timer3_interrupt(void)
{
    EIE1 &= ~EIE1_ET3__ENABLED;
}

inline void enable_capture_interrupt(void)
{
    // channel 0 capture flag interrupt enable
    PCA0CPM0 |= PCA0CPM0_ECCF__ENABLED;
}

inline void disable_capture_interrupt(void)
{
    PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;
}

inline void enable_pca0_interrupt(void)
{
    // PCA0 interrupt enable
    EIE1 |= EIE1_EPCA0__ENABLED;
}

inline void disable_pca0_interrupt(void)
{
    EIE1 &= ~EIE1_EPCA0__ENABLED;
}

// this is necessary so that uart ring buffer logic operates correctly the first time it is used
// i.e., flag is set as though the last character sent was completed, even though no previous character was actually sent
inline void init_uart0_transmit_interrupt_flag(void)
{
    SCON0_TI = 1;
}


inline void enable_serial_interrupt(void)
{
    IE_ES0 = 1;
}

inline void disable_serial_interrupt(void)
{
    IE_ES0 = 0;
}

inline void timer0_run(void)
{
    TCON_TR0 = 1;
}

inline void timer0_stop(void)
{
    TCON_TR0 = 0;
}

inline void timer1_run(void)
{
    TCON_TR1 = 1;
}

inline void timer1_stop(void)
{
    TCON_TR1 = 0;
}

inline void timer2_run(void)
{
    TMR2CN0_TR2 = 1;
}

inline void timer2_stop(void)
{
    TMR2CN0_TR2 = 0;
}

inline void timer3_run(void)
{
    // timer 3 on
    TMR3CN0 |= TMR3CN0_TR3__RUN;
}

inline void timer3_stop(void)
{
    // timer 3 stop
    TMR3CN0 &= ~TMR3CN0_TR3__RUN;
}

void set_clock_mode(void);
void enable_watchdog(void);
void disable_watchdog(void);
void refresh_watchdog(void);
void reset_mcu(void);
void init_port_pins_for_passthrough(void);
void init_port_pins_for_serial(void);
void init_uart0(void);
void init_uart1(void);
void init_timer0_8bit_autoreload(const uint8_t);
void init_timer1_8bit_autoreload(const uint8_t);

void init_uart1_transmit_interrupt_flag(void);

void pca0_init(void);
void pca0_run(void);
void pca0_halt(void);

uint8_t get_capture_flags(void);
void    set_capture_flags(const uint8_t flags);

//extern void clear_capture_flag(void);

uint16_t countsToTime(const uint16_t duration);

#endif // INC_HAL_H_
