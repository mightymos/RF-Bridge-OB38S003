#ifndef INC_EFM8BB1LCK_PINS_H_
#define INC_EFM8BB1LCK_PINS_H_

// EFM8BB1LCK board
#define TDATA  P0_0

// error in UG377: EFM8BB1LCK Users Guide ??
// lists PCA0 and LED pin as the same
#define LED    P1_4
#define RDATA  P1_3
#define BUZZER P1_6

// UART TX, RX routed to Port pins P0.4 and P0.5
#define UART_TX_PIN P0_4
#define UART_RX_PIN P0_5

// software uart
#define SOFT_TX_PIN P1_5

// for reading by oscilloscope
#define DEBUG_PIN_01 P1_7

#endif