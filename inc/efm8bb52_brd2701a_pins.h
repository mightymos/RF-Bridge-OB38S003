#ifndef INC_EFM8BB52_BRD2701A_PINS_H_
#define INC_EFM8BB52_BRD2701A_PINS_H_

// EFM8BB52 Explorer Kit board
#define TDATA  P0_B0

// UG512: BB52 Explorer Kit User's Guide
#define LED    P1_B4
#define RDATA  P1_B3
#define BUZZER P1_B6

// UART TX, RX routed to Port pins P0.4 and P0.5
#define UART_TX_PIN P0_B4
#define UART_RX_PIN P0_B5

// table 3.1 of UG512: BB52 guide
#define UART1_TX_PIN P2_B2
#define UART1_RX_PIN P2_B3

// for reading by oscilloscope
#define DEBUG_PIN01  P1_B7

#endif