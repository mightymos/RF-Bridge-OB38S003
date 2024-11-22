#ifndef INC_SERIAL_H
#define INC_SERIAL_H

#include <stdint.h>

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
//extern __xdata uart_state_t uart_state;
//extern __xdata uart_command_t uart_command;

//extern void uart_put_command(uint8_t command);

extern void uart_put_RF_Data_Standard(uint8_t command);
extern void uart_put_RF_Data_Advanced(uint8_t command, uint8_t protocol_index);

// bucket sniffing
extern void uart_put_RF_buckets(uint8_t Command);

#endif // INC_SERIAL_H