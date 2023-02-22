/*
 *  Ported on: 02.17.2023
 *      Author: Jonathan Armstrong
 */

#ifndef RF_HANDLING_H
#define RF_HANDLING_H

#include <stdbool.h>
#include <stdint.h>

#include "globals.h"


// 112 byte == 896 bits, so a RF signal with maximum of 896 bits is possible
// for bucket transmission, this depends on the number of buckets.
// E.g. 4 buckets have a total overhead of 11, allowing 101 bits (high/low pairs)
#define RF_DATA_BUFFERSIZE 112

// FIXME: this needs a comment to explain
#define RF_DATA_RECEIVED_MASK	0x80

typedef enum
{
	RF_IDLE,
	RF_IN_SYNC,
	RF_BUCKET_REPEAT,
	RF_BUCKET_IN_SYNC,
	RF_DECODE_BUCKET,
	RF_FINISHED
} RF_STATE_T;

typedef enum
{
	STANDARD,
	ADVANCED
} RF_SNIFFING_MODE_T;


typedef struct PROTOCOL_STATUS
{
	uint16_t status;
	uint8_t bit_count;
	uint8_t actual_bit_of_byte;
} PROTOCOL_STATUS;

typedef struct PROTOCOL_DATA_UINT8_T
{
	// pointer to array of uint8_t elements
	uint8_t* dat;
	// size of the array
	uint8_t size;
} PROTOCOL_DATA_UINT8_T;

typedef struct PROTOCOL_DATA_UINT16_T
{
	// pointer to array of uint16_t elements
	uint16_t* dat;
	// size of the array
	uint8_t size;
} PROTOCOL_DATA_UINT16_T;

typedef struct BUCKET_PROTOCOL_DATA_T
{
	// array and array size of buckets/start/bit0/bit1/end
	PROTOCOL_DATA_UINT16_T buckets;
	PROTOCOL_DATA_UINT8_T start;
	PROTOCOL_DATA_UINT8_T bit0;
	PROTOCOL_DATA_UINT8_T bit1;
	PROTOCOL_DATA_UINT8_T end;
    
	// bit count for this protocol
	uint8_t bit_count;
} BUCKET_PROTOCOL_DATA_T;



// for accessing global variables in other compilation units
extern __xdata uint8_t gRFData[RF_DATA_BUFFERSIZE];
extern __xdata uint16_t gBuckets[7];

extern const BUCKET_PROTOCOL_DATA_T protocols[];
extern RF_STATE_T gRFState;
extern uint8_t gLastSniffingCommand;
extern RF_SNIFFING_MODE_T gSniffingMode;

extern uint8_t  gRFDataStatus;
extern uint16_t gSyncLow;
extern uint16_t gBitHigh;
extern uint16_t gBitLow;
extern uint8_t  gActualByte;
extern uint16_t bucket_sync;
extern uint8_t  bucket_count;

// function prototypes
extern bool buffer_out(uint16_t* bucket);
extern void handle_rf_bucket(uint16_t duration, bool high_low);
extern uint8_t pca_start_sniffing(uint8_t active_command);
extern void pca_stop_sniffing(void);
extern void send_buckets(uint16_t* pulses, uint8_t* start, uint8_t start_size, uint8_t* bit0, uint8_t bit0_size, uint8_t* bit1, uint8_t bit1_size, uint8_t* end, uint8_t end_size, uint8_t bit_count, uint8_t* rfdata);
extern void send_buckets_by_index(uint8_t index, uint8_t* rfdata);


#endif // RF_HANDLING_H
