/*
 *  Ported on: 02.18.2023
 *      Author: Jonathan Armstrong
 */

#ifndef RF_PROTOCOLS_H
#define RF_PROTOCOLS_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>

// bucket sniffing constants
#define MIN_FOOTER_LENGTH	300
#define MIN_BUCKET_LENGTH	100

#define PT226x_SYNC_MIN		4500

// sync constants
#define TOLERANCE_MAX		500
#define TOLERANCE_MIN		100

//
#define TOTAL_RF_DATA_BITS 24

// number of repeating by default
#define RF_TRANSMIT_REPEATS 8

typedef struct PROTOCOL_STATUS_T
{
	uint16_t status;
	uint8_t bit_count;
	uint8_t actual_bit_of_byte;
} PROTOCOL_STATUS_T;


#endif // RF_PROTOCOLS_H
