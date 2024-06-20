#ifndef INC_SONOFFR22_PINS_H_
#define INC_SONOFFR22_PINS_H_

// pin defines
#define BUZZER      P0_0

// input to radio transmitter
#define TDATA       P0_7

// hardware uart
#define UART_TX_PIN P1_0
#define UART_RX_PIN P1_1

// reset pin is connected to copper pad so can use as a debug output
// need to set as gpio in SMAP AC utility when flashing (i.e., disable reset function)
// can be used with software uart
#define RESET_PIN P1_5

// output from radio receiver
#define RDATA     P1_6

// connected to resistor R23 which is 1kohm resistor as upper leg of 10kohm bottom resistor divider
#define RF_ENABLE   P1_7

//
#define LED         P3_0

#endif