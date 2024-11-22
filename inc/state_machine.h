/*
 *  Ported on: 02.18.2023
 *      Author: Jonathan Armstrong
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

typedef enum
{
	RF_IDLE                    = 0x00,
	RF_CHECK_REPEATS           = 0x01,
	RF_TRANSMIT_BY_TIMING      = 0x02,
	RF_TRANSMIT_BY_PROTOCOL    = 0x03,
	RF_FINISHED                = 0x04
} RF_STATE_T;

typedef enum
{
	NO_COMMAND                = 0x00,
	RF_RFOUT_NEW_START        = 0x01,
	RF_RFOUT_START            = 0x02
} RF_COMMAND_T;

RF_COMMAND_T uart_state_machine(const unsigned int rxdata);
void rf_state_machine(RF_COMMAND_T command);

void radio_rfin(void);
void radio_decode_debug(void);
void radio_decode_timings(void);

#endif // STATE_MACHINE_H
