#ifndef INC_SONOFFR20_PINS_H_
#define INC_SONOFFR20_PINS_H_

// Sonoff with black case
#define TDATA  P0_0

// error in UG377: EFM8BB1LCK Users Guide ??
// lists PCA0 and LED pin as the same
// but SonOfPortisch works with these pins
// on development board
// FIXME: handle sonoff versus development board
#define LED    P1_4
#define RDATA  P1_3


#define BUZZER P1_6

// UART TX, RX routed to Port pins P0.4 and P0.5
#define UART_TX_PIN P0_4
#define UART_RX_PIN P0_5

// may be used for software uart
#define DEBUG_PIN0 P1_5
#define DEBUG_PIN1 P1_6

#endif