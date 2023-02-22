/*
 * 
 *  Ported on: 02.16.2023
 *      Author: Jonathan
 */

//#include <SI_EFM8BB1_Register_Enums.h>
#include <string.h>
#include <stdlib.h>
#include "..\inc\globals.h"
#include "..\inc\rf_handling.h"
#include "..\inc\rf_protocols.h"
//#include "pca_0.h"
#include "..\inc\uart.h"

// internal ram and external ram are equal size (256 bytes each)
//   so need to place some variables in external ram
__xdata uint8_t gRFData[RF_DATA_BUFFERSIZE];

// use const or __code to place in flash/code space
const static uint16_t bucketsPT226x [] = { 350, 1050, 10850 };
const static uint8_t  start[]   = { HIGH(0), LOW(2) };
const static uint8_t  bit0[]    = { HIGH(0), LOW(1) };
const static uint8_t  bit1[]    = { HIGH(1), LOW(0) };

const BUCKET_PROTOCOL_DATA_T protocols[] = {
    {{&bucketsPT226x[0], 3}, {&start[0], 2}, {&bit0[0], 2}, {&bit1[0], 2}, {NULL, 0}, 24}
};

enum {
   numProtocols = sizeof(protocols) / sizeof(protocols[0])
};

//bool actual_byte_high_nibble = false;
//uint8_t gActualByte = 0;

// status of each protocol
__xdata PROTOCOL_STATUS_T status[numProtocols];

// up to 8 timing buckets for RF_CODE_SNIFFING_ON_BUCKET
// size is one less because of the bucket_sync
__xdata uint16_t gBuckets[7];
__xdata uint16_t gBufferBuckets[4] = {0};

uint8_t old_crc = 0;
uint8_t crc = 0;

uint8_t buffer_buckets_positions = 0;

// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
uint8_t gRFDataStatus;

RF_STATE_T gRFState;

RF_SNIFFING_MODE_T gSniffingMode;

uint8_t gLastSniffingCommand;

uint16_t gSyncLow;
uint16_t gBitHigh;
uint16_t gBitLow;
uint8_t  gActualByte;


uint16_t bucket_sync;
uint8_t bucket_count = 0;
uint8_t bucket_count_sync_1;
uint8_t bucket_count_sync_2;

// FIXME: what does W stand for?
#define GET_W_POSITION(x) (((x) >> 4) & 0x0F)
#define INC_W_POSITION(x) ((x) = ((((x) >> 4) + 1) << 4) | ((x) & 0x0F))
#define DEC_W_POSITION(x) ((x) = ((((x) >> 4) - 1) << 4) | ((x) & 0x0F))
#define CLR_W_POSITION(x) ((x) &= 0x0F)

// FIXME: what does R stand for?
#define GET_R_POSITION(x) ((x) & 0x0F)
#define INC_R_POSITION(x) ((x) = ((x) + 1) | ((x) & 0xF0))
#define DEC_R_POSITION(x) ((x) = ((x) - 1) | ((x) & 0xF0))
#define CLR_R_POSITION(x) ((x) &= 0xF0)


//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------
void pca_overflowCb()
{
    
}

void pca_intermediateOverflowCb()
{
    
}

void pca_channel1EventCb()
{

}

void pca_channel2EventCb()
{
    
}

void set_timer0_overflow(const uint8_t overflow)
{
    // FIXME: i do not know why this is being done this way via define
    // https://www.keil.com/dd/docs/c51/silabs/efm8sb2/inc/si_efm8sb2_register_enums.h
    ///< Timer 0 High Byte
    //#define TH0_TH0__SHIFT 0x00
	// Timer 0 High Byte = T0_Overflow
	TH0 = (overflow << 0x00);
}

uint8_t compute_crc8_simple_onebyte(uint8_t byteVal)
{
    const uint8_t generator = 0x1D;
    uint8_t i;
    uint8_t crc = byteVal; /* init crc directly with input byte instead of 0, avoid useless 8 bitshifts until input byte is in crc register */

    for (i = 0; i < 8; i++)
    {
        if ((crc & 0x80) != 0)
        { /* most significant bit set, shift crc register and perform XOR operation, taking not-saved 9th set bit into account */
            crc = (uint8_t)((crc << 1) ^ generator);
        }
        else
        { /* most significant bit not set, go to next bit */
            crc <<= 1;
        }
    }

    return crc;
}

uint16_t compute_delta(uint16_t bucket)
{
	//return ((bucket >> 2) + (bucket >> 4));
    // 25% delta of bucket for advanced decoding
	return (bucket >> 2);
}

bool check_rf_bucket(uint16_t duration, uint16_t bucket, uint16_t delta)
{
	return (((bucket - delta) < duration) && (duration < (bucket + delta)));
}

bool check_rf_sync_bucket(uint16_t duration, uint16_t bucket)
{
	uint16_t delta = compute_delta(bucket);
	delta = delta > TOLERANCE_MAX ? TOLERANCE_MAX : delta;
	delta = delta < TOLERANCE_MIN ? TOLERANCE_MIN : delta;
	return check_rf_bucket(duration, bucket, delta);
}

bool decode_bucket(uint8_t i, bool high_low, uint16_t duration,
		uint16_t *pulses,
		uint8_t* bit0,
        uint8_t bit0_size,
		uint8_t* bit1,
        uint8_t bit1_size,
		uint8_t bit_count)
{
	uint8_t last_bit = 0;

	// do init before first bit received
	if (BITS_GET(status[i]) == 0)
	{
		ABP_RESET(status[i]);
		memset(gRFData, 0, sizeof(gRFData));
		crc = 0x00;
	}

	// start decoding of the bits in sync of the buckets

	// bit 0
	if (check_rf_sync_bucket(duration, pulses[BUCKET_NR(bit0[BIT0_GET(status[i])])]))
	{
		// decode only if high/low does match
		if (BUCKET_STATE(bit0[BIT0_GET(status[i])]) == high_low)
		{
			if (BIT0_GET(status[i]) == 0)
				gBitLow = duration;

			BIT0_INC(status[i]);
		}
	}
	// bucket does not match bit, reset status
	else
	{
		BIT0_CLEAR(status[i]);
	}

	// bit 1
	if (check_rf_sync_bucket(duration, pulses[BUCKET_NR(bit1[BIT1_GET(status[i])])]))
	{
		// decode only if high/low does match
		if (BUCKET_STATE(bit1[BIT1_GET(status[i])]) == high_low)
		{
			if (BIT1_GET(status[i]) == 0)
				gBitHigh = duration;

			BIT1_INC(status[i]);
		}
	}
	// bucket does not match bit, reset status
	else
	{
		BIT1_CLEAR(status[i]);
	}

	// check if any bucket got decoded, if not restart
	if ((BIT0_GET(status[i]) == 0) && (BIT1_GET(status[i]) == 0))
	{
		START_CLEAR(status[i]);
		return false;
	}

	// on the last bit do not check the last bucket
	// because maybe this is not correct because a
	// repeat delay
	if (BITS_GET(status[i]) == bit_count - 1)
		last_bit = 1;

	// check if bit 0 is finished
	if (BIT0_GET(status[i]) == bit0_size - last_bit)
	{
		BITS_CLEAR(status[i]);
		BITS_INC(status[i]);
		ABP_DEC(status[i]);
	}
	// check if bit 1 is finished
	else if (BIT1_GET(status[i]) == bit1_size - last_bit)
	{
		BITS_CLEAR(status[i]);
		BITS_INC(status[i]);
		ABP_DEC(status[i]);
		gRFData[(BITS_GET(status[i]) - 1) >> 3] |= (1 << ABP_GET(status[i]));
	}

	// 8 bits are done, compute crc of data
	if (ABP_GET(status[i]) == 0)
	{
		crc = Compute_CRC8_Simple_OneByte(crc ^ gRFData[(BITS_GET(status[i]) - 1) >> 3]);
		ABP_RESET(status[i]);
	}

	// check if all bit got collected
	if (BITS_GET(status[i]) >= bit_count)
	{
		// check if timeout timer for crc is finished
		if (IsTimer2Finished())
			old_crc = 0;

		// check new crc on last received data for debounce
		if (crc != old_crc)
		{
			// new data, restart crc timeout
			StopTimer2();
			InitTimer2_ms(1, 800);
			old_crc = crc;

			// disable interrupt for RF receiving while uart transfer
			//PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

			// set status
			gRFDataStatus = i;
			gRFDataStatus |= RF_DATA_RECEIVED_MASK;
		}

		START_CLEAR(status[i]);
		return true;
	}

	return false;
}

void handle_rf_bucket(uint16_t duration, bool high_low)
{
	uint8_t i = 0;

	// if noise got received stop all running decodings
	if (duration < MIN_BUCKET_LENGTH)
	{
		for (i = 0; i < numProtocols; i++)
			START_CLEAR(status[i]);

		return;
	}

	// handle the buckets by standard or advanced decoding
	switch(gSniffingMode)
	{
		case STANDARD:
			// check if protocol was not started
			if (START_GET(status[0]) == 0)
			{
				// if PT226x standard sniffing calculate the pulse time by the longer sync bucket
				// this will enable receive PT226x in a range of PT226x_SYNC_MIN <-> 32767µs
				if (duration > PT226x_SYNC_MIN && !high_low) // && (duration < PT226x_SYNC_MAX))
				{
					// increment start because of the skipped first high bucket
					START_INC(status[0]);
					START_INC(status[0]);
					gSyncLow = duration;

					gBuckets[0] = duration / 31;
					gBuckets[1] = gBuckets[0] * 3;
					gBuckets[2] = duration;
				}
			}
			// if sync is finished check if bit0 or bit1 is starting
			else if (START_GET(status[0]) == 2)
			{
				decode_bucket(0, high_low, duration,
						gBuckets,
						protocols[0].bit0.dat, protocols[0].bit0.size,
						protocols[0].bit1.dat, protocols[0].bit1.size,
						protocols[0].bit_count
						);
			}
			break;

		case ADVANCED:
			// check each protocol for each bucket
			for (i = 0; i < numProtocols; i++)
			{
				// protocol started, check if sync is finished
				if (START_GET(status[i]) < protocols[i].start.size)
				{
					// check if sync bucket high/low is matching
					if (BUCKET_STATE(protocols[i].start.dat[START_GET(status[i])]) != high_low)
						continue;

					if (check_rf_sync_bucket(duration, protocols[i].buckets.dat[BUCKET_NR(protocols[i].start.dat[START_GET(status[i])])]))
					{
						START_INC(status[i]);
						continue;
					}
					else
					{
						START_CLEAR(status[i]);
						continue;
					}
				}
				// if sync is finished check if bit0 or bit1 is starting
				else if (START_GET(status[i]) == protocols[i].start.size)
				{
					if (decode_bucket(i, high_low, duration,
							protocols[i].buckets.dat,
							protocols[i].bit0.dat, protocols[i].bit0.size,
							protocols[i].bit1.dat, protocols[i].bit1.size,
							protocols[i].bit_count
							))
						return;
				}
			}
			break;
	}
}

void buffer_in(uint16_t bucket)
{
	if ((GET_W_POSITION(buffer_buckets_positions) + 1 == GET_R_POSITION(buffer_buckets_positions)) ||
			(GET_R_POSITION(buffer_buckets_positions) == 0 && GET_W_POSITION(buffer_buckets_positions) + 1 == ARRAY_LENGTH(gBufferBuckets)))
		return;

	gBufferBuckets[GET_W_POSITION(buffer_buckets_positions)] = bucket;

	INC_W_POSITION(buffer_buckets_positions);

	if (GET_W_POSITION(buffer_buckets_positions) >= ARRAY_LENGTH(gBufferBuckets))
		CLR_W_POSITION(buffer_buckets_positions);
}

bool buffer_out(uint16_t* bucket)
{
	//uint8_t backup_PCA0CPM0 = PCA0CPM0;

	// check if buffer is empty
	if (GET_W_POSITION(buffer_buckets_positions) == GET_R_POSITION(buffer_buckets_positions))
		return false;

	// disable interrupt for RF receiving while reading buffer
	//PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

	*bucket = gBufferBuckets[GET_R_POSITION(buffer_buckets_positions)];
	INC_R_POSITION(buffer_buckets_positions);

	if (GET_R_POSITION(buffer_buckets_positions) >= ARRAY_LENGTH(gBufferBuckets))
		CLR_R_POSITION(buffer_buckets_positions);

	// reset register
	//PCA0CPM0 = backup_PCA0CPM0;

	return true;
}

void PCA0_channel0EventCb()
{
    // FIXME: restore once pca implemented
	//uint16_t current_capture_value = PCA0CP0 * 10;
    uint16_t current_capture_value = 0;
	//uint8_t flags = PCA0MD;

	// clear counter
	//PCA0MD &= 0xBF;
	//PCA0H = 0x00;
	//PCA0L = 0x00;
	//PCA0MD = flags;

	// if bucket is no noise add it to buffer
    //if (current_capture_value > MIN_PULSE_LENGTH && current_capture_value < 0x8000)
	if (current_capture_value < 0x8000)
	{
        // FIXME: make sure we ported this correctly
		//buffer_in(current_capture_value | ((uint16_t)(!R_DATA) << 15));
        buffer_in(current_capture_value | ((uint16_t)(!isrdataHigh()) << 15));
	}
	else
	{
		// received noise, clear all received buckets
		buffer_buckets_positions = 0;
	}
}



uint8_t pca_start_sniffing(uint8_t activeCommand)
{
	uint8_t ret = gLastSniffingCommand;

	memset(status, 0, sizeof(PROTOCOL_STATUS) * numProtocols);

	// restore timer to 100000Hz, 10µs interval
	SetTimer0Overflow(0x0B);

	// enable interrupt for RF receiving
	//PCA0CPM0 |= PCA0CPM0_ECCF__ENABLED;

	// start PCA
	//PCA0_run();

	//InitTimer3_ms(1, 10);
	// wait until timer has finished
	//WaitTimer3Finished();

	gRFState = RF_IDLE;
	gRFDataStatus = 0;

    // FIXME: setting state machine in a totally different file is too confusing
	// set uart_command back if sniffing was on
	//gUartCommand = activeCommand;

	// backup uart_command to be able to enable the sniffing again
	gLastSniffingCommand = activeCommand;

	return ret;
}

void pca_stop_sniffing(void)
{
	// stop PCA
	//PCA0_halt();

	// clear all interrupt flags of PCA0
	//PCA0CN0 &= ~(PCA0CN0_CF__BMASK
	//  		                       | PCA0CN0_CCF0__BMASK
	//  		                       | PCA0CN0_CCF1__BMASK
	//  		                       | PCA0CN0_CCF2__BMASK);

	// disable interrupt for RF receiving
	//PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

	// be sure the timeout timer is stopped
	StopTimer2();

	gRFState = RF_IDLE;
}

bool send_single_bucket(bool high_low, uint16_t bucket_time)
{
	// switch to high_low
	tdata_off();
	//InitTimer3_us(10, bucket_time);
	// wait until timer has finished
	//WaitTimer3Finished();
	return !high_low;
}

void send_buckets(
		uint16_t *pulses,
		uint8_t* start, uint8_t start_size,
		uint8_t* bit0, uint8_t bit0_size,
		uint8_t* bit1, uint8_t bit1_size,
		uint8_t* end, uint8_t end_size,
		uint8_t bit_count,
		uint8_t* rfdata)
{
	uint8_t i, a;
	uint8_t actual_byte = 0;
	uint8_t actual_bit = 0x80;

	// transmit sync bucket(s)
	for (i = 0; i < start_size; i++)
    {
		send_single_bucket(BUCKET_STATE(start[i]), pulses[BUCKET_NR(start[i])]);
    }

	// transmit bit bucket(s)
	for (i = 0; i < bit_count; i++)
	{
		// send bit 0
		if ((rfdata[actual_byte] & actual_bit) == 0)
		{
			for (a = 0; a < bit0_size; a++)
            {
				send_single_bucket(BUCKET_STATE(bit0[a]), pulses[BUCKET_NR(bit0[a])]);
            }
		}
		// send bit 1
		else
		{
			for (a = 0; a < bit1_size; a++)
            {
				send_single_bucket(BUCKET_STATE(bit1[a]), pulses[BUCKET_NR(bit1[a])]);
            }
		}

		actual_bit >>= 1;

		if (actual_bit == 0)
		{
			actual_byte++;
			actual_bit = 0x80;
		}
	}

	// transmit end bucket(s)
	for (i = 0; i < end_size; i++)
		SendSingleBucket(BUCKET_STATE(end[i]), pulses[BUCKET_NR(end[i])]);

	gRFState = RF_FINISHED;
}

void send_buckets_by_index(uint8_t index, uint8_t* rfdata)
{
	SendBuckets(
			protocols[index].buckets.dat,
			protocols[index].start.dat, protocols[index].start.size,
			protocols[index].bit0.dat, protocols[index].bit0.size,
			protocols[index].bit1.dat, protocols[index].bit1.size,
			protocols[index].end.dat, protocols[index].end.size,
			protocols[index].bit_count,
			rfdata
			);
}


//-----------------------------------------------------------------------------
// Send generic signal based on n time bucket pairs (high/low timing)
//-----------------------------------------------------------------------------
#if INCLUDE_BUCKET_SNIFFING
    void send_rf_buckets(uint16_t* buckets, uint8_t* rfdata, uint8_t data_len)
    {
        // start transmit of the buckets with a high bucket
        bool high_low = true;
        bool high_low_mark = false;
        uint8_t i;

        // check first two buckets if high/low marking is included
        high_low_mark = (rfdata[0] & 0x88) > 0;

        // transmit data
        for (i = 0; i < data_len; i++)
        {
                high_low = SendSingleBucket(high_low_mark ? (bool)(rfdata[i] >> 7) : high_low, gBuckets[(rfdata[i] >> 4) & 0x07]);
                high_low = SendSingleBucket(high_low_mark ? (bool)((rfdata[i] >> 3) & 0x01) : high_low, gBuckets[rfdata[i] & 0x07]);
        }

        gRFState = RF_FINISHED;
    }
#endif

#if INCLUDE_BUCKET_SNIFFING

    bool probably_footer(uint16_t duration)
    {
        return duration >= MIN_FOOTER_LENGTH;
    }

    bool matches_footer(uint16_t duration, bool high_low)
    {
        if (!((bucket_sync & 0x8000) >> 15) && high_low)
            return false;

        return CheckRFSyncBucket(duration, bucket_sync & 0x7FFF);
    }
#endif

#if INCLUDE_BUCKET_SNIFFING
    // FIXME: this function is using up lots of ram?
    bool find_bucket(uint16_t duration, uint8_t *index)
    {
        uint8_t i;
        uint16_t delta;

        for (i = 0; i < bucket_count; i++)
        {
            // calculate delta by the current duration and check if the new duration fits into
            delta = ((duration >> 2) + (duration >> 3));
            delta = delta > gBuckets[i] ? gBuckets[i] : delta;

            if (CheckRFBucket(duration, gBuckets[i], delta))
            {
                *index = i;
                return true;
            }
        }

        return false;
    }
#endif



#if INCLUDE_BUCKET_SNIFFING
    void bucket_received(uint16_t duration, bool high_low)
    {
        bool actualByteHighNibble = false;
        uint8_t bucket_index;

        // if pulse is too short reset status
        if (duration < MIN_BUCKET_LENGTH)
        {
            gRFState = RF_IDLE;
            return;
        }

        switch (gRFState)
        {
            // check if we maybe receive a sync
            case RF_IDLE:

                if (probablyFooter(duration))
                {
                    bucket_sync = duration | ((uint16_t)high_low << 15);
                    bucket_count_sync_1 = 0;
                    gRFState = RF_BUCKET_REPEAT;
                }
                break;

            // check if the same bucket gets received
            case RF_BUCKET_REPEAT:
                if (matchesFooter(duration, high_low))
                {
                    // check if a minimum of buckets where between two sync pulses
                    if (bucket_count_sync_1 > 4)
                    {
                        bucket_count = 0;
                        gActualByte = 0;
                        actualByteHighNibble = false;
                        bucket_count_sync_2 = 0;
                        crc = 0x00;
                        gRFData[0] = 0;
                        gRFState = RF_BUCKET_IN_SYNC;
                    }
                    else
                    {
                        gRFState = RF_IDLE;
                    }
                }
                // check if duration is longer than sync bucket restart
                else if (duration > (bucket_sync & 0x7FFF))
                {
                    // this bucket looks like the sync bucket
                    bucket_sync = duration | ((uint16_t)high_low << 15);
                    bucket_count_sync_1 = 0;
                }
                else
                {
                    bucket_count_sync_1++;
                }

                // no more buckets are possible, reset
                if (bucket_count_sync_1 >= RF_DATA_BUFFERSIZE << 1)
                    gRFState = RF_IDLE;

                break;

            // same sync bucket got received, filter buckets
            case RF_BUCKET_IN_SYNC:
                bucket_count_sync_2++;

                // check if all buckets got received
                if (bucket_count_sync_2 <= bucket_count_sync_1)
                {
                    // check if bucket was already received
                    if (!findBucket(duration, &bucket_index))
                    {
                        // new bucket received, add to array
                        gBuckets[bucket_count] = duration;
                        bucket_index = bucket_count;
                        bucket_count++;

                        // check if maximum of array got reached
                        if (bucket_count > ARRAY_LENGTH(gBuckets))
                        {
                            // restart sync
                            gRFState = RF_IDLE;
                        }
                    }

                    // fill rf data with the current bucket number
                    if (actualByteHighNibble)
                    {
                        gRFData[gActualByte] = (bucket_index << 4) | ((uint8_t)high_low << 7);
                    }
                    else
                    {
                        gRFData[gActualByte] |= (bucket_index | ((uint8_t)high_low << 3));

                        crc = Compute_CRC8_Simple_OneByte(crc ^ gRFData[gActualByte]);

                        gActualByte++;

                        // check if maximum of array got reached
                        if (gActualByte > RF_DATA_BUFFERSIZE)
                        {
                            // restart sync
                            gRFState = RF_IDLE;
                        }
                    }

                    actualByteHighNibble = !actualByteHighNibble;
                }
                // next bucket after data have to be a sync bucket
                else if (matchesFooter(duration, high_low))
                {
                    // check if timeout timer for crc is finished
                    if (IsTimer2Finished())
                        old_crc = 0;

                    // check new crc on last received data for debounce
                    if (crc != old_crc)
                    {
                        // new data, restart crc timeout
                        StopTimer2();
                        InitTimer2_ms(1, 800);
                        old_crc = crc;

                        // disable interrupt for RF receiving while uart transfer
                        //PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

                        // add sync bucket number to data
                        gRFData[0] |= ((bucket_count << 4) | ((bucket_sync & 0x8000) >> 8));

                        // clear high/low flag
                        bucket_sync &= 0x7FFF;

                        gRFDataStatus |= RF_DATA_RECEIVED_MASK;
                    }

                    gRFState = RF_IDLE;
                }
                // next bucket after receiving all data buckets was not a sync bucket, restart
                else
                {
                    // restart sync
                    gRFState = RF_IDLE;
                }
                break;
        }
    }

#endif
