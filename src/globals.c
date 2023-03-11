/*
 *
 *  Ported on: 02.16.2023
 *      Author: Jonathan Armstrong
 */
#include "..\inc\globals.h"

// pin defines
// avoid exposing to other compilation units
#define BUZZER      P0_0
#define TDATA       P0_7
#define UART_TX_PIN P1_0

// pin does not appear to be connected to anything except copper pad
//  so can use as a debug output
// apparently need to set as gpio when programming (i.e., disable reset function)
#define RESET_PIN   P1_5
#define RDATA       P1_6
#define RF_ENABLE   P1_7
#define LED         P3_0

inline void buzzer_on(void)
{
    BUZZER = 1;
}

inline void buzzer_off(void)
{
    BUZZER = 0;
}

inline bool is_rdata_low(void)
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


inline void radio_off(void)
{
    RF_ENABLE = 1;
}


inline void radio_on(void)
{
    RF_ENABLE = 0;
}

inline void reset_pin_on(void)
{
    RESET_PIN = 1;
}

inline void reset_pin_off(void)
{
    RESET_PIN = 0;
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