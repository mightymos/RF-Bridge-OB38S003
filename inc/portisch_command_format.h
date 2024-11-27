#ifndef PORTISCH_COMMAND_H
#define PORTISCH_COMMAND_H

// common uart packet format for both rcswitch and portisch (despite the file name)

// this is returned along with an ack in response to a request firmware version command
// which is a good demonstration that serial pins and state machine are working
#define FIRMWARE_VERSION 0x04


// start and end of uart packet
#define RF_CODE_START 0xAA
#define RF_CODE_STOP  0x55

// uart packet format
typedef enum
{
	IDLE,
	SYNC_INIT,
	SYNC_FINISH,
	RECEIVE_LENGTH,
	RECEIVING,
	TRANSMIT,
	COMMAND
} uart_state_t;


// commands which should be supported partially by rcswitch and mostly by portisch are given here (learning are not supported as of writing):
// https://github.com/Portisch/RF-Bridge-EFM8BB1/wiki/Commands
typedef enum
{
	NONE                       = 0x00,
	RF_CODE_ACK                = 0xA0,
	RF_CODE_LEARN              = 0xA1,
	RF_CODE_LEARN_KO           = 0xA2,
	RF_CODE_LEARN_OK           = 0xA3,
	RF_CODE_RFIN               = 0xA4,
	RF_CODE_RFOUT              = 0xA5,
	RF_CODE_SNIFFING_ON        = 0xA6,
	RF_CODE_SNIFFING_OFF       = 0xA7,
	RF_CODE_RFOUT_NEW          = 0xA8,
	RF_CODE_LEARN_NEW          = 0xA9,
	RF_CODE_LEARN_KO_NEW       = 0xAA,
	RF_CODE_LEARN_OK_NEW       = 0xAB,
	RF_CODE_RFOUT_BUCKET       = 0xB0,
	RF_CODE_SNIFFING_ON_BUCKET = 0xB1,
	RF_DO_BEEP                 = 0xC0,
    RF_RESET_MCU               = 0xFE,
	RF_ALTERNATIVE_FIRMWARE    = 0xFF
} uart_command_t;


#endif