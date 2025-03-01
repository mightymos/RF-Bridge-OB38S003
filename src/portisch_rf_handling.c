/*
 * RF_Handling.c
 *
 *  Created on: 27.11.2017
 *      Author:
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

//#include "capture_interrupt.h"
#include "delay.h"
#include "hal.h"
#include "portisch_rf_handling.h"
#include "portisch_protocols.h"
#include "project-defs.h"
#include "timer_interrupts.h"
//#include "pca_0.h"
//#include "timers.h"

// FIXME: not clear why this is four as opposed to anything else
#define BUFFER_BUCKETS_SIZE 4

// FIXME: add comment
__xdata uint8_t RF_DATA[RF_DATA_BUFFERSIZE];

// RF_DATA_STATUS
// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
__xdata uint8_t RF_DATA_STATUS = 0;
__xdata rf_sniffing_mode_t sniffing_mode = STANDARD;


// PT226x variables
__xdata uint16_t SYNC_LOW = 0x00;
__xdata uint16_t BIT_HIGH = 0x00;
__xdata uint16_t BIT_LOW  = 0x00;

__xdata rf_state_t rf_state = RF_IDLE;

__xdata bool actual_byte_high_nibble = false;

__xdata uint8_t actual_byte = 0;

// status of each protocol
__xdata PROTOCOL_STATUS status[NUM_OF_PROTOCOLS];


__xdata uint8_t old_crc = 0;
__xdata uint8_t crc = 0;

// up to 8 timing buckets for RF_CODE_SNIFFING_ON_BUCKET
// -1 because of the bucket_sync
__xdata uint16_t buckets[7];

// bucket sniffing
__xdata uint16_t bucket_sync;
__xdata uint8_t bucket_count = 0;
__xdata uint8_t bucket_count_sync_1;
__xdata uint8_t bucket_count_sync_2;


// stores measured durations temporarily
__xdata uint16_t buffer_buckets[BUFFER_BUCKETS_SIZE] = {0};

// use separate read and write "pointers" into circular buffer
__xdata uint8_t buffer_buckets_read = 0;
__xdata uint8_t buffer_buckets_write = 0;

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------
void PCA0_overflowCb(void) { }
void PCA0_intermediateOverflowCb(void) { }

uint8_t Compute_CRC8_Simple_OneByte(uint8_t byteVal)
{
    const uint8_t generator = 0x1D;
    uint8_t i;
	// init crc directly with input byte instead of 0, avoid useless 8 bitshifts until input byte is in crc register
    uint8_t crc = byteVal;

    for (i = 0; i < 8; i++)
    {
        if ((crc & 0x80) != 0)
        { 
			// most significant bit set, shift crc register and perform XOR operation, taking not-saved 9th set bit into account
            crc = (uint8_t)((crc << 1) ^ generator);
        }
        else
        { 
			// most significant bit not set, go to next bit
            crc <<= 1;
        }
    }

    return crc;
}

uint16_t compute_delta(uint16_t bucket)
{
	//return ((bucket >> 2) + (bucket >> 4));
	return (bucket >> 2); // 25% delta of bucket for advanced decoding
}

bool CheckRFBucket(uint16_t duration, uint16_t bucket, uint16_t delta)
{
	return (((bucket - delta) < duration) && (duration < (bucket + delta)));
}

bool CheckRFSyncBucket(uint16_t duration, uint16_t bucket)
{
	uint16_t delta = compute_delta(bucket);
	delta = delta > TOLERANCE_MAX ? TOLERANCE_MAX : delta;
	delta = delta < TOLERANCE_MIN ? TOLERANCE_MIN : delta;
	return CheckRFBucket(duration, bucket, delta);
}

bool DecodeBucket(const uint8_t id, const bool high_low, const uint16_t duration, uint16_t *pulses, uint8_t* bit0, uint8_t bit0_size, uint8_t* bit1, uint8_t bit1_size, uint8_t bit_count)
{
	uint8_t last_bit = 0;
    bool status_bit;
    
    uint8_t index;
    uint8_t data;

	// do init before first bit received
	if (status[id].bit_count == 0)
	{
		status[id].actual_bit_of_byte = 8;
		memset(RF_DATA, 0, sizeof(RF_DATA));
		crc = 0x00;
	}

	// start decoding of the bits in sync of the buckets

	// bit 0
	if (CheckRFSyncBucket(duration, pulses[bit0[status[id].bit0_status] & 0x07]))
	{
		// decode only if high/low does match
        status_bit = (bit0[status[id].bit0_status] & 0x08) >> 3;
		if (status_bit == high_low)
		{
			if (status[id].bit0_status == 0)
				BIT_LOW = duration;

			status[id].bit0_status += 1;
		}
	}
	else
	{	
		// bucket does not match bit, reset status
		status[id].bit0_status = 0;
	}

	// bit 1
	if (CheckRFSyncBucket(duration, pulses[bit1[status[id].bit1_status] & 0x07]))
	{
		// decode only if high/low does match
        status_bit = (bit1[status[id].bit1_status] & 0x08) >> 3;
		if (status_bit == high_low)
		{
			if (status[id].bit1_status == 0)
			{
				BIT_HIGH = duration;
			}

			status[id].bit1_status += 1;
		}
	}
	else
	{
		// bucket does not match bit, reset status
		status[id].bit1_status = 0;
	}

	// check if any bucket got decoded, if not restart
	if ((status[id].bit0_status == 0) && (status[id].bit1_status == 0))
	{
		led_off();
		
		status[id].sync_status = 0;
		status[id].bit0_status = 0;
		status[id].bit1_status = 0;
		status[id].end_status = 0;
		status[id].bit_count = 0;
		status[id].actual_bit_of_byte = 0;

		return false;
	}

	// on the last bit do not check the last bucket
	// because maybe this is not correct because a
	// repeat delay
	if (status[id].bit_count == bit_count - 1)
    {
		last_bit = 1;
    }

	// check if bit 0 is finished
	if (status[id].bit0_status == bit0_size - last_bit)
	{
		led_on();
		//BITS_CLEAR(status[i]);
		status[id].bit0_status = 0;
		status[id].bit1_status = 0;
		//BITS_INC(status[i]);
		status[id].bit_count += 1;
		status[id].actual_bit_of_byte -= 1;
	}
	// check if bit 1 is finished
	else if (status[id].bit1_status == bit1_size - last_bit)
	{
		led_on();
		//BITS_CLEAR(status[i]);
		status[id].bit0_status = 0;
		status[id].bit1_status = 0;
		//BITS_INC(status[i]);
		status[id].bit_count += 1;
		status[id].actual_bit_of_byte -= 1;
        
        //
        index = (status[id].bit_count - 1) >> 3;
		RF_DATA[index] |= (1 << status[id].actual_bit_of_byte);
	}

	// 8 bits are done, compute crc of data
	if (status[id].actual_bit_of_byte == 0)
	{
        // shift by three is equivalent to divide by eight
        index = (status[id].bit_count - 1) >> 3;
        data = RF_DATA[index];
        
		crc = Compute_CRC8_Simple_OneByte(crc ^ data);
		status[id].actual_bit_of_byte = 8;
        
//#if defined(UART_LOGGING_ENABLED)
//        printf_tiny("RF[%d]: 0x%x\r\n", index, data);
//#endif
	}

	// check if all bit got collected
	if (status[id].bit_count >= bit_count)
	{
		// check if timeout timer for crc is finished
		if (is_first_delay_finished())
		{
			old_crc = 0;
		}

		// check new crc on last received data for debounce
		if (crc != old_crc)
		{
			// new data, restart crc timeout
			stop_first_delay();
			init_first_delay_ms(800);
			old_crc = crc;

			// FIXME: it can be confusing to bury things like this in functions
			// disable interrupt for RF receiving while uart transfer
			disable_capture_interrupt();
            //disable_pca0_interrupt();

			// set status
			RF_DATA_STATUS = id;
			RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
		}

		led_off();

		status[id].sync_status = 0;
		status[id].bit0_status = 0;
		status[id].bit1_status = 0;
		status[id].end_status  = 0;
		status[id].bit_count = 0;
		status[id].actual_bit_of_byte = 0;

		return true;
	}

	return false;
}

void HandleRFBucket(const uint16_t duration, const bool high_low)
{
	uint8_t i = 0;
    bool sync_bit;

	// if noise got received stop all running decodings
	if (duration < MIN_BUCKET_LENGTH)
	{
		// compiler will optimize this out if NUM_OF_PROTOCOLS = 1
		for (i = 0; i < NUM_OF_PROTOCOLS; i++)
		{
			status[i].sync_status = 0;
			status[i].bit0_status = 0;
			status[i].bit1_status = 0;
			status[i].end_status  = 0;
			status[i].bit_count = 0;
			status[i].actual_bit_of_byte = 0;
		}

		led_off();
		return;
	}

	// handle the buckets by standard or advanced decoding
	switch(sniffing_mode)
	{
		case STANDARD:
			// check if protocol was not started
			if (status[0].sync_status == 0)
			{
				// if PT226x standard sniffing calculate the pulse time by the longer sync bucket
				// this will enable receive PT226x in a range of PT226x_SYNC_MIN <-> 32767 microsec
				if (duration > PT226x_SYNC_MIN && !high_low) // && (duration < PT226x_SYNC_MAX))
				{
					// increment start twice because of the skipped first high bucket
                    status[0].sync_status += 2;
					SYNC_LOW = duration;

                    //FIXME: change to eliminate divides and multiplies if possible
					buckets[0] = duration / 31;
                    // shift and add is equivalent of multiple by three
					//buckets[1] = buckets[0] * 3;
                    buckets[1] = (buckets[0] << 1) + buckets[0];
					buckets[2] = duration;
				}
			}
			// if sync is finished check if bit0 or bit1 is starting
			else if (status[0].sync_status == 2)
			{
				// place all on one line so debugger does not get confused
				DecodeBucket(0, high_low, duration, buckets, PROTOCOL_DATA[0].bit0.dat, PROTOCOL_DATA[0].bit0.size, PROTOCOL_DATA[0].bit1.dat, PROTOCOL_DATA[0].bit1.size, PROTOCOL_DATA[0].bit_count);
			}
			break;

		case ADVANCED:
			// check each protocol for each bucket
			for (i = 0; i < NUM_OF_PROTOCOLS; i++)
			{
				// protocol started, check if sync is finished
				if (status[i].sync_status < PROTOCOL_DATA[i].start.size)
				{
					// check if sync bucket high/low is matching
                    sync_bit = (PROTOCOL_DATA[i].start.dat[status[i].sync_status] & 0x08) >> 3;
					if (sync_bit != high_low)
						continue;

					if (CheckRFSyncBucket(duration, PROTOCOL_DATA[i].buckets.dat[PROTOCOL_DATA[i].start.dat[status[i].sync_status] & 0x07]))
					{
						status[i].sync_status += 1;
						continue;
					}
					else
					{
						status[i].sync_status = 0;
						status[i].bit0_status = 0;
						status[i].bit1_status = 0;
						status[i].end_status  = 0;
						status[i].bit_count = 0;
						status[i].actual_bit_of_byte = 0;
						continue;
					}
				}
				// if sync is finished check if bit0 or bit1 is starting
				else if (status[i].sync_status == PROTOCOL_DATA[i].start.size)
				{
					if (DecodeBucket(i, high_low, duration, PROTOCOL_DATA[i].buckets.dat,PROTOCOL_DATA[i].bit0.dat, PROTOCOL_DATA[i].bit0.size,PROTOCOL_DATA[i].bit1.dat, PROTOCOL_DATA[i].bit1.size,PROTOCOL_DATA[i].bit_count))
						return;
				}
			}
			break;
	}	// switch(sniffing_mode)
}

void buffer_in(uint16_t bucket)
{
	// check if writing next byte into circular buffer will catch up to read position, and if so bail out
	if ( (buffer_buckets_write + 1) == buffer_buckets_read )
	{

#if 0
//#if defined(UART_LOGGING_ENABLED)
                
        //printf_tiny("%s\r\n", __LINE__);
        printf_tiny("bucket buffer over 00 \r\n");

#endif

		return;
	}
    
    //
	if ( (buffer_buckets_read == 0) && ((buffer_buckets_write + 1) == BUFFER_BUCKETS_SIZE) )
	{


#if 0
//#if defined(UART_LOGGING_ENABLED)        
        // DEBUG: this just gets triggered no matter how large I make the buffers, can I explain if it matters or not?
        //printf_tiny("%s\r\n", __LINE__);
        printf_tiny("bbr: %c \r\n", buffer_buckets_read);
        printf_tiny("bbw: %c \r\n", buffer_buckets_write);

#endif

		return;
	}

	buffer_buckets[buffer_buckets_write] = bucket;

	buffer_buckets_write++;


	if (buffer_buckets_write >= BUFFER_BUCKETS_SIZE)
	{
		buffer_buckets_write = 0;
	}
}

bool buffer_out(uint16_t* bucket)
{
	//FIXME: may need to do this type of save outside of function call
	uint8_t backup_Flags = get_capture_flags();

	// check if buffer is empty
	if (buffer_buckets_write == buffer_buckets_read)
	{
		return false;
	}

	// disable interrupt for RF receiving while reading buffer
	disable_capture_interrupt();
    //disable_pca0_interrupt();


	*bucket = buffer_buckets[buffer_buckets_read];
	buffer_buckets_read++;


	if (buffer_buckets_read >= BUFFER_BUCKETS_SIZE)
	{
		buffer_buckets_read = 0;
	}

	// reset register
	set_capture_flags(backup_Flags);

	return true;
}

void capture_handler(uint16_t current_capture_value)
{
    // makes code more readable
	bool pin;
    
    //FIXME: make hardware abstraction
	//uint16_t current_capture_value = get_capture_value();
    //current_capture_value = current_capture_value / 2;
    current_capture_value = countsToTime(current_capture_value);

    clear_pca_counter();

	// FIXME: additional comments; if bucket is not noise add it to buffer
	if (current_capture_value < 0x8000)
	{
		// FIXME: add comment
		pin = rdata_level();
		buffer_in(current_capture_value | ((uint16_t)(!pin) << 15));
        
#if defined(DEBUG_PINS_ENABLED)
        set_debug_pin01(pin);
#endif

	}
	else
	{
		// received noise, so clear all received buckets
		buffer_buckets_read = 0;
		buffer_buckets_write = 0;
	}
}

// original logic
//out = PCA0_DoSniffing(in)
//
//out = last_sniffing_command;
//uart_command          = in;
//last_sniffing_command = in;
//return out;
void PCA0_DoSniffing(void)
{    
	memset(status, 0, sizeof(PROTOCOL_STATUS) * NUM_OF_PROTOCOLS);

    // FIXME: need to understand if or when this was or is needed
	// restore timer to 100000Hz, 10�s interval
	//SetTimer0Overflow(0x0B);

	// enable interrupt for RF receiving
	enable_capture_interrupt();
    //enable_pca0_interrupt();

	// start PCA
	pca0_run();

	// 
	// wait until timer has finished
	//delay1ms(10);
    init_second_delay_ms(10);
    wait_second_delay_finished();


    // FIXME: add comment
    // FIXME: eventually move setting radio state outside of function
    rf_state = RF_IDLE;
    
	// FIXME: add comment
	RF_DATA_STATUS = 0;
}

void PCA0_StopSniffing(void)
{
	// stop PCA
	pca0_halt();

	// clear all interrupt flags of PCA0
    clear_interrupt_flags_pca();

	// disable interrupt for RF receiving
	disable_capture_interrupt();
    //disable_pca0_interrupt();

	// be sure the timeout timer is stopped
	stop_first_delay();
    
    // FIXME: eventually move setting radio state outside of function
    rf_state = RF_IDLE;
}


// returns the inverted bit of what was passed in to high_low
bool SendSingleBucket(const bool high_low, uint16_t bucket_time)
{
    // FIXME: improve this comment
	// switch to high_low
    set_led(high_low);
    set_tdata(high_low);
    
#if defined(DEBUG_PINS_ENABLED)

    // DEBUG: mirror radio transmit output to a free gpio
    set_debug_pin01(high_low);
    
#endif
	
	// FIXME: nop style delay seems to basically work (with first protocol)
	// but bucket timings measured at receiver are inaccurate due to delay_us inaccuracy
	//efm8_delay_us(bucket_time);
	// FIXME: so maybe just use timer delays instead
	init_second_delay_us(bucket_time / 10);
	wait_second_delay_finished();

	return !high_low;
}

//-----------------------------------------------------------------------------
// Send generic signal based on n time bucket pairs (high/low timing)
//-----------------------------------------------------------------------------
void SendRFBuckets(uint16_t* buckets, uint8_t* rfdata, uint8_t data_len)
{
	// start transmit of the buckets with a high bucket
	bool high_low = true;
	bool high_low_mark = false;
	uint8_t i;
    
    // help human readability
    bool level;
    uint16_t bucket_time;

	// check first two buckets if high/low marking is included
	high_low_mark = (rfdata[0] & 0x88) > 0;

	// transmit data
	for (i = 0; i < data_len; i++)
	{
        //
        level = high_low_mark ? (bool)(rfdata[i] >> 7) : high_low;
        bucket_time = buckets[(rfdata[i] >> 4) & 0x07];
		high_low = SendSingleBucket(level, bucket_time);
        
        //
        level = high_low_mark ? (bool)((rfdata[i] >> 3) & 0x01) : high_low;
        bucket_time = buckets[rfdata[i] & 0x07];
		high_low = SendSingleBucket(level, bucket_time);
	}

	led_off();
}


void SendBuckets(uint16_t *pulses, uint8_t* start, uint8_t start_size, uint8_t* bit0, uint8_t bit0_size, uint8_t* bit1, uint8_t bit1_size, uint8_t* end, uint8_t end_size, uint8_t bit_count, uint8_t* rfdata)
{
	uint8_t i, a;
	uint8_t actual_byte = 0;
	uint8_t actual_bit = 0x80;
    
    bool level;
    uint16_t bucket_time;

	// transmit sync bucket(s)
	for (i = 0; i < start_size; i++)
    {
        level = (start[i] & 0x08) >> 3;
        bucket_time = pulses[start[i] & 0x07];
		SendSingleBucket(level, bucket_time);
    }

	// transmit bit bucket(s)
	for (i = 0; i < bit_count; i++)
	{
		// send bit 0
		if ((rfdata[actual_byte] & actual_bit) == 0)
		{
			for (a = 0; a < bit0_size; a++)
			{
                level = (bit0[a] & 0x08) >> 3;
                bucket_time = pulses[bit0[a] & 0x07];
				SendSingleBucket(level, bucket_time);
			}
		}
		else
		{	// send bit 1
			for (a = 0; a < bit1_size; a++)
			{
                level = (bit1[a] & 0x08) >> 3;
                bucket_time = pulses[bit1[a] & 0x07];
				SendSingleBucket(level, bucket_time);
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
	{
        level = (end[i] & 0x08) >> 3;
        bucket_time = pulses[end[i] & 0x07];
		SendSingleBucket(level, bucket_time);
	}

	led_off();
}

void SendBucketsByIndex(uint8_t index, uint8_t* rfdata)
{
	// helps allow sendbuckets call to be more readable
	uint8_t start_size = PROTOCOL_DATA[index].start.size;
	uint8_t bit0_size  = PROTOCOL_DATA[index].bit0.size;
	uint8_t bit1_size  = PROTOCOL_DATA[index].bit1.size;
	uint8_t end_size   = PROTOCOL_DATA[index].end.size;
	uint8_t bitcount   = PROTOCOL_DATA[index].bit_count;
    
	SendBuckets(PROTOCOL_DATA[index].buckets.dat, PROTOCOL_DATA[index].start.dat, start_size, PROTOCOL_DATA[index].bit0.dat, bit0_size, PROTOCOL_DATA[index].bit1.dat, bit1_size, PROTOCOL_DATA[index].end.dat, end_size, bitcount, rfdata);
}


// defined in Makefile
#if defined(BUCKET_SNIFFING_INCLUDED)

    // probablyFooter(), matchesFooter(), findBucket(), and Bucket_Received() are all related to bucket sniffing feature
    bool probablyFooter(uint16_t duration)
    {
        return duration >= MIN_FOOTER_LENGTH;
    }

    bool matchesFooter(uint16_t duration, bool high_low)
    {
        if (!((bucket_sync & 0x8000) >> 15) && high_low)
            return false;

        return CheckRFSyncBucket(duration, bucket_sync & 0x7FFF);
    }

    bool findBucket(uint16_t duration, uint8_t *index)
    {
        uint8_t i;
        uint16_t delta;

        for (i = 0; i < bucket_count; i++)
        {
            // calculate delta by the current duration and check if the new duration fits into
            delta = ((duration >> 2) + (duration >> 3));
            delta = delta > buckets[i] ? buckets[i] : delta;

            if (CheckRFBucket(duration, buckets[i], delta))
            {
                *index = i;
                return true;
            }
        }

        return false;
    }

    // this is for sniffing on bucket (0xB1 command)
    void Bucket_Received(const uint16_t duration, const bool high_low)
    {
        uint8_t bucket_index;

        // if pulse is too short reset status
        if (duration < MIN_BUCKET_LENGTH)
        {
            rf_state = RF_IDLE;
            return;
        }

        switch (rf_state)
        {
            // check if we maybe receive a sync
            case RF_IDLE:
                led_off();

                if (probablyFooter(duration))
                {
                    bucket_sync = duration | ((uint16_t)high_low << 15);
                    bucket_count_sync_1 = 0;
                    rf_state = RF_BUCKET_REPEAT;
                }
                break;

            // check if the same bucket gets received
            case RF_BUCKET_REPEAT:
                if (matchesFooter(duration, high_low))
                {
                    // check if a minimum of buckets where between two sync pulses
                    if (bucket_count_sync_1 > 4)
                    {
                        led_on();
                        bucket_count = 0;
                        actual_byte = 0;
                        actual_byte_high_nibble = false;
                        bucket_count_sync_2 = 0;
                        crc = 0x00;
                        RF_DATA[0] = 0;
                        rf_state = RF_BUCKET_IN_SYNC;
                    }
                    else
                    {
                        rf_state = RF_IDLE;
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
                {
                    rf_state = RF_IDLE;
                }

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
                        buckets[bucket_count] = duration;
                        bucket_index = bucket_count;
                        bucket_count++;

                        // check if maximum of array got reached
                        if (bucket_count > ARRAY_LENGTH(buckets))
                        {
                            // restart sync
                            rf_state = RF_IDLE;
                            
//#if defined(UART_LOGGING_ENABLED)
//                            // DEBUG: if included this triggers regularly, so exclude for now
//                            printf_tiny("bucket_count > ARRAY_LENGTH(buckets)\r\n");
//#endif
                        }
                    }

                    // fill rf data with the current bucket number
                    if (actual_byte_high_nibble)
                    {
                        RF_DATA[actual_byte] = (bucket_index << 4) | ((uint8_t)high_low << 7);
                    }
                    else
                    {
                        RF_DATA[actual_byte] |= (bucket_index | ((uint8_t)high_low << 3));

                        crc = Compute_CRC8_Simple_OneByte(crc ^ RF_DATA[actual_byte]);

                        actual_byte++;

                        // check if maximum of array got reached
                        // FIXME: greater than or greater than or equal to?
                        if (actual_byte >= RF_DATA_BUFFERSIZE)
                        {
                            // restart sync
                            rf_state = RF_IDLE;
                            
#if defined(UART_LOGGING_ENABLED)
                
                            // FIXME:
                            //printf_tiny("%s\r\n", __LINE__);
                            printf_tiny("actual_byte >= RF_DATA_BUFFERSIZE\r\n");
#endif
                        }
                    }

                    actual_byte_high_nibble = !actual_byte_high_nibble;
                }
                // next bucket after data have to be a sync bucket
                else if (matchesFooter(duration, high_low))
                {
                    // check if timeout timer for crc is finished
                    if (is_first_delay_finished())
                    {
                        old_crc = 0;
                    }

                    // check new crc on last received data for debounce
                    if (crc != old_crc)
                    {
                        // new data, restart crc timeout
                        stop_first_delay();
                        init_first_delay_ms(800);
                        old_crc = crc;

                        // disable interrupt for RF receiving while uart transfer
                        //FIXME: want to move outside of buried function eventually?
                        disable_capture_interrupt();
                        //disable_pca0_interrupt();

                        // add sync bucket number to data
                        RF_DATA[0] |= ((bucket_count << 4) | ((bucket_sync & 0x8000) >> 8));

                        // clear high/low flag
                        bucket_sync &= 0x7FFF;

                        RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
                    }

                    led_off();
                    rf_state = RF_IDLE;
                }
                // next bucket after receiving all data buckets was not a sync bucket, restart
                else
                {
                    // restart sync
                    rf_state = RF_IDLE;
                }
                break;
        }
    }


#endif