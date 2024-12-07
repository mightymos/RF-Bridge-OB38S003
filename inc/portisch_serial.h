#ifndef INC_SERIAL_H
#define INC_SERIAL_H

#include <stdint.h>

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------


extern void uart_put_RF_Data_Standard(uint8_t command);
extern void uart_put_RF_Data_Advanced(uint8_t command, uint8_t protocol_index);

// bucket sniffing
extern void uart_put_RF_buckets(uint8_t Command);

#endif // INC_SERIAL_H