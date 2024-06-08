#ifndef INC_SERIAL_H
#define INC_SERIAL_H

#include <stdint.h>

// tasmota compatible protocol
#define RF_CODE_START		0xAA
#define RF_CODE_STOP		0x55

//-----------------------------------------------------------------------------
// Global Enums
//-----------------------------------------------------------------------------
typedef enum
{
	IDLE,
	SYNC_INIT,
	SYNC_FINISH,
	RECEIVE_LEN,
	RECEIVING,
	TRANSMIT,
	COMMAND
} uart_state_t;

typedef enum
{
	NONE = 0x00,
	RF_CODE_ACK = 0xA0,
	RF_CODE_LEARN = 0xA1,
	RF_CODE_LEARN_KO = 0xA2,
	RF_CODE_LEARN_OK = 0xA3,
	RF_CODE_RFIN = 0xA4,
	RF_CODE_RFOUT = 0xA5,
	RF_CODE_SNIFFING_ON = 0xA6,
	RF_CODE_SNIFFING_OFF = 0xA7,
	RF_CODE_RFOUT_NEW = 0xA8,
	RF_CODE_LEARN_NEW = 0xA9,
	RF_CODE_LEARN_KO_NEW = 0xAA,
	RF_CODE_LEARN_OK_NEW = 0xAB,
	RF_CODE_RFOUT_BUCKET = 0xB0,
	RF_CODE_SNIFFING_ON_BUCKET = 0xB1,
	RF_DO_BEEP = 0xC0,
	RF_ALTERNATIVE_FIRMWARE = 0xFF
} uart_command_t;


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
extern __xdata uart_state_t uart_state;
extern __xdata uart_command_t uart_command;

extern void uart_put_command(uint8_t command);

extern void uart_put_RF_Data_Standard(uint8_t command);
extern void uart_put_RF_Data_Advanced(uint8_t command, uint8_t protocol_index);

#if defined(INCLUDE_BUCKET_SNIFFING)
extern void uart_put_RF_buckets(uint8_t Command);
#endif

#endif