/*
 * RF_Handling.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_RF_HANDLING_H_
#define INC_RF_HANDLING_H_

#include <stdbool.h>
#include <stdint.h>

#include "portisch_config.h"

// 112 byte == 896 bits, so a RF signal with maximum of 896 bits is possible
// for bucket transmission, this depends on the number of buckets.
// E.g. 4 buckets have a total overhead of 11, allowing 101 bits (high/low pairs)
#if defined(INCLUDE_BUCKET_SNIFFING)
    #define RF_DATA_BUFFERSIZE 112
#else
    #define RF_DATA_BUFFERSIZE  32
#endif

typedef enum
{
	RF_IDLE,
	RF_IN_SYNC,
	RF_BUCKET_REPEAT,
	RF_BUCKET_IN_SYNC,
	RF_DECODE_BUCKET,
	RF_FINISHED
} rf_state_t;

typedef enum
{
	STANDARD,
	ADVANCED
} rf_sniffing_mode_t;

// FIXME: add comment
#define RF_DATA_RECEIVED_MASK	0x80

extern __xdata uint8_t RF_DATA[RF_DATA_BUFFERSIZE];
// RF_DATA_STATUS
// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
extern __xdata uint8_t RF_DATA_STATUS;
extern __xdata rf_sniffing_mode_t sniffing_mode;

extern __xdata uint16_t SYNC_LOW;
extern __xdata uint16_t BIT_HIGH;
extern __xdata uint16_t BIT_LOW;

extern __xdata uint8_t actual_byte;

extern __xdata uint16_t buckets[7];

#if defined(INCLUDE_BUCKET_SNIFFING)
extern __xdata uint16_t bucket_sync;
extern __xdata uint8_t bucket_count;
#endif


extern bool buffer_out(uint16_t* bucket);
extern void HandleRFBucket(uint16_t duration, bool high_low);
extern uint8_t PCA0_DoSniffing(void);
extern void PCA0_StopSniffing(void);
extern void SendRFBuckets(uint16_t* buckets, uint8_t* rfdata, uint8_t data_len);
extern void SendBuckets(uint16_t *pulses, uint8_t* start, uint8_t start_size, uint8_t* bit0, uint8_t bit0_size, uint8_t* bit1, uint8_t bit1_size, uint8_t* end, uint8_t end_size, uint8_t bit_count, uint8_t* rfdata);
extern void SendBucketsByIndex(uint8_t index, uint8_t* rfdata);
extern void Bucket_Received(uint16_t duration, bool high_low, rf_state_t* rf_state);

#endif // INC_RF_HANDLING_H_
