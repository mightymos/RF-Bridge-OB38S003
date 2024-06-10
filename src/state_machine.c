#include "delay.h"
#include "hal.h"
#include "rcswitch.h"
#include "state_machine.h"
#include "uart.h"

#include <stdio.h>
#include <stdint.h>

// hold past bytes so we can parse when sync end is received
#define PACKET_MAX_SIZE  12

//-----------------------------------------------------------------------------
// Portisch generally used this type of state machine, which I had a lot of trouble reading
//-----------------------------------------------------------------------------
void uart_state_machine(const unsigned int rxdataWithFlags)
{
    // FIXME: need to check what appropriate initialization values are
    static UART_STATE_T state = IDLE;
    static UART_COMMAND_T command = NONE;

	static uint8_t packet[PACKET_MAX_SIZE];
	
    // FIXME: need to know what initialization value is appropriate
    static uint8_t position = 0;
	static uint8_t lengthExpected = 0;

    // FIXME: needed?
    //uint16_t bucket = 0;
	
	// strip uart flags to 
	const uint8_t rxdata = rxdata & 0xFF;


    // track count of entries into function
    static uint16_t idleResetCount = 0;

    // FIXME: this seems to reset state machine if we do not receive data after some time
	// FIXME: but we need to determine this outside of state machine I think ?
    if (rxdataWithFlags == UART_NO_DATA)
    {
        if (state == IDLE)
            idleResetCount = 0;
        else
        {
            idleResetCount++;
            
            // FIXME: no magic numbers
            if (idleResetCount > 30000)
            {
                idleResetCount = 0;
                
                state = IDLE;
                command = NONE;
            }
        }
    }
    else
    {
        idleResetCount = 0;
        
    }
    
    // state machine for UART
    switch(state)
    {
        // check if UART_SYNC_INIT got received
        case IDLE:
            if (rxdata == RF_CODE_START)
            {
                state = SYNC_INIT;
				
				// go ahead and store the sync start (should be 0xAA)
				packet[position] = SYNC_INIT;
				position++;
            }
            
            break;

        // sync byte got received, so next read command
        case SYNC_INIT:
            command = rxdata;
            
            // FIXME: not sure if setting this here is correct
            //state = SYNC_FINISH;

            // check if some data needs to be received
            switch(command)
            {
                case RF_CODE_LEARN:
                    break;
                // do original sniffing
                case RF_CODE_RFIN:
                    break;
                case RF_CODE_RFOUT:
					packet[position] = command;
					position++;
					
					lengthExpected = 11;
					state = RECEIVING;
                    break;
                case RF_DO_BEEP:
                    // FIXME: replace with timer rather than delay(), although appears original code was blocking too
                    buzzer_on();
                    delay1ms(50);
                    buzzer_off();

                    // send acknowledge
                    uart_put_command(RF_CODE_ACK);
					
					state = SYNC_FINISH;
                    break;
                case RF_ALTERNATIVE_FIRMWARE:
                    uart_put_command(RF_CODE_ACK);
                    uart_put_command(FIRMWARE_VERSION);
					
					state = SYNC_FINISH;
                    break;
                case RF_CODE_SNIFFING_ON:
                    //gSniffingMode = ADVANCED;
                    //PCA0_DoSniffing(RF_CODE_SNIFFING_ON);
                    //gLastSniffingCommand = RF_CODE_SNIFFING_ON;
                    break;
                case RF_CODE_SNIFFING_OFF:
                    // set desired RF protocol PT2260
                    //gSniffingMode = STANDARD;
                    // re-enable default RF_CODE_RFIN sniffing
                    //pca_start_sniffing(RF_CODE_RFIN);
                    //gLastSniffingCommand = RF_CODE_RFIN;
                    break;
                case RF_CODE_ACK:
                    // re-enable default RF_CODE_RFIN sniffing
                    //gLastSniffingCommand = PCA0_DoSniffing(gLastSniffingCommand);
                    //state = IDLE;
                    break;
                case SINGLE_STEP_DEBUG:
                    //gSingleStep = true;
                    break;

                    
                // wait until data got transfered
                case RF_FINISHED:
                    //if (trRepeats == 0)
                    //{
                    //    // disable RF transmit
                    //    tdata_off();
                    //
                    //    uart_put_command(RF_CODE_ACK);
                    //} else {
                    //    gRFState = RF_IDLE;
                    //}
                    break;

                // unknown command
                default:
                    state = IDLE;
                    command = NONE;
                    break;
            }
            break;

        // Receiving UART data length
        case RECEIVE_LEN:
            //position = 0;
            //gLength = rxdata & 0xFF;
            //if (gLength > 0)
            //{
            //    // stop sniffing while handling received data
            //    pca_stop_sniffing();
            //    state = RECEIVING;
            //} else {
            //    state = SYNC_FINISH;
            //}
            
            break;

        // receiving UART data
        case RECEIVING:
            packet[position] = rxdata;
            position++;

            if (position == lengthExpected)
            {
                state = SYNC_FINISH;
            }
            else if (position >= PACKET_MAX_SIZE)
            {
				// FIXME: review this logic
                lengthExpected = PACKET_MAX_SIZE;
                state = SYNC_FINISH;
            }
            break;

        // wait and check for UART_SYNC_END
        case SYNC_FINISH:
            if (rxdata == RF_CODE_STOP)
            {
                state = IDLE;
            }
            break;
    }
	
	// this should be occuring after an entire uart packet is received (i.e., after SYNC_FINISH)
	switch(command)
	{
		case RF_CODE_RFOUT:
		
				// only do the job if all data got received by UART
				if (state != IDLE)
					break;

				// do transmit of the data
				switch(rf_state)
				{
					// init and start RF transmit
					case RF_IDLE:
						transmissionRepeats--;
						//PCA0_StopSniffing();

						// bytes 0..1:	Tsyn
						// bytes 2..3:	Tlow
						// bytes 4..5:	Thigh
						// bytes 6..7:	24bit Data
						buckets[0] = *(uint16_t *)&packet[2];
						buckets[1] = *(uint16_t *)&packet[4];
						buckets[2] = *(uint16_t *)&packet[0];

						FIXME: send
						//SendBuckets(
						//		buckets,
						//		PROTOCOL_DATA[0].start.dat, PROTOCOL_DATA[0].start.size,
						//		PROTOCOL_DATA[0].bit0.dat, PROTOCOL_DATA[0].bit0.size,
						//		PROTOCOL_DATA[0].bit1.dat, PROTOCOL_DATA[0].bit1.size,
						//		PROTOCOL_DATA[0].end.dat, PROTOCOL_DATA[0].end.size,
						//		PROTOCOL_DATA[0].bit_count,
						//		RF_DATA + 6
						//		);
						break;

					// wait until data got transfered
					case RF_FINISHED:
						// FIXME: how do we ever get to this case in original portisch?
						if (transmissionRepeats == 0)
						{
							// disable RF transmit
							tdata_off();

							finish_command(RF_CODE_ACK);
						}
						else
						{
							rf_state = RF_IDLE;
						}
						break;
				}
				break;
	}
}



// FIXME: some of these function names really need fixing
void radio_decode_report(void)
{
    uint8_t i = 0;
    uint8_t b = 0;

    // packet start sequence
    putchar(RF_CODE_START);
    putchar(RF_CODE_RFIN);
    
    // sync, low, high timings
    putchar((timings[0] >> 8) & 0xFF);
    putchar(timings[0] & 0xFF);

    
    // FIXME: not sure if we should compute an average or something
    // FIXME: handle inverted signal?
    putchar((timings[2] >> 8) & 0xFF);
    putchar( timings[2] & 0xFF);
    putchar((timings[1] >> 8) & 0xFF);
    putchar( timings[1] & 0xFF);
    
    // data
    // FIXME: strange that shifting by ZERO works but omitting the shift does not
    putchar((get_received_value() >> 16) & 0xFF);
    putchar((get_received_value() >>  8) & 0xFF);
    putchar((get_received_value() >>  0) & 0xFF);
    
    // packet stop
    putchar(RF_CODE_STOP);
}

// FIXME: think Tasmota ignores this for now because command is unknown?
void radio_timings(void)
{
    unsigned int index;
    
    for (index = 0; index < get_received_bitlength() * 2; index++)
    {
        // packet start sequence
        putchar(RF_CODE_START);
        putchar(0xAF);
        
        // sync, low, high timings
        putchar((timings[index] >> 8) & 0xFF);
        putchar(timings[index] & 0xFF);
    }
    
    putchar(RF_CODE_STOP);
}

#if 0
    // we avoid use of printf but may be able to adapt this to wifi serial protocol format?
    void radio_decode_debug(void)
    {
        printf_fast("Received: ");
        printf_fast("0x%lx", get_received_value());
        printf_fast(" / ");
        printf_fast("%u", get_received_bitlength() );
        printf_fast("bit ");
        printf_fast("Protocol: ");
        printf_fast("%u", get_received_protocol() );
        
        printf_fast("\r\n");
    }
#endif