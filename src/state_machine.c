#include "delay.h"
#include "hal.h"
#include "rcswitch.h"
#include "state_machine.h"
#include "uart.h"

#include <stdio.h>
#include <stdint.h>

#include "uart_software.h"


// FIXME: I think this is incorrect because we can receive variable data length with some commands
#define PACKET_MAX_SIZE  12

__xdata uint8_t uartPacket[PACKET_MAX_SIZE];
__xdata uint8_t rfTXRepeats;

//-----------------------------------------------------------------------------
// Portisch generally used this type of state machine
//-----------------------------------------------------------------------------
RF_COMMAND_T uart_state_machine(const unsigned int rxdataWithFlags)
{
    // FIXME: eventually an appropriate initialization value might be RF_RFIN for decoding by default
	// but right now we decode outside of state machine
    __xdata static UART_STATE_T     state = IDLE;
    __xdata static UART_COMMAND_T command = NONE;

	// return this value when we need the radio state machine to do something
	__xdata RF_COMMAND_T rfCommand = NO_COMMAND;
	
    // FIXME: need to know what initialization value is appropriate
    __xdata static uint8_t position = 0;
	__xdata static uint8_t lengthExpected = 0;

    // FIXME: needed?
    //uint16_t bucket = 0;
	
    // track count of entries into function
    __xdata static uint16_t idleResetCount = 0;

	// strip uart flags to 
	__xdata const uint8_t rxdata = rxdataWithFlags & 0xFF;	


    // if we do not receive any data over uart in a long time, we reset state machine
	// also, if we do not receive data when we check, we do not enter state machine
    if (rxdataWithFlags == UART_NO_DATA)
    {
        if (state == IDLE)
		{
            idleResetCount = 0;
		}
        else
        {
            idleResetCount++;
            
            // FIXME: we need some documentation of how count relates to real world time
            if (idleResetCount > 30000)
            {
                idleResetCount = 0;
                
                state = IDLE;
                command = NONE;
				
				// DEBUG:
				putstring("idleReset\r\n");
            }
        }
    }
    else
    {
        idleResetCount = 0;

    
		// state machine for UART
		switch(state)
		{
			// check if UART_SYNC_INIT got received
			case IDLE:
				if (rxdata == RF_CODE_START)
				{
					state = SYNC_INIT;
					
					// DEBUG: over software uart
					puthex2(rxdata);
				}
				
				break;

			// sync byte got received, so next read command
			case SYNC_INIT:
			
				// FIXME: add comment
				command = rxdata;
				
				// DEBUG
				puthex2(command);

				// FIXME: comment
				switch(command)
				{
					case RF_CODE_LEARN:
						break;
					// do original sniffing
					case RF_CODE_RFIN:
						// we do this outside of state machine for now
						break;
					case RF_CODE_RFOUT:
						// reset index into packet storage array
						position = 0;
						// only count data bytes, we exclude start sync, end, and command bytes
						lengthExpected = 9;
						state = RECEIVING;
						
						break;
					case RF_DO_BEEP:
						position = 0;
						lengthExpected = 2;
						state = RECEIVING;
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

					// unknown command
					default:
						state = IDLE;
						command = NONE;
						break;
				}
				break;

			// this is used for commands that have a variable length
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
				uartPacket[position] = rxdata;
				position++;

				// DEBUG:
				puthex2(rxdata);
				
				// look for expected end of packet and also avoid buffer overflow
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
					// DEBUG:
					puthex2(rxdata);
					putc('\r');
					putc('\n');
					
					// indicate this round of receiving uart packet is finished
					state = IDLE;

					// the last command received should still be saved from before we received any additional data bytes
					switch(command)
					{
						case RF_CODE_RFOUT:
							rfCommand = RF_RFOUT_START;
							rfTXRepeats = 8;
							break;
						case RF_DO_BEEP:
							uint16_t delay = *(uint16_t *)&uartPacket[0];
							
							// FIXME: will blocking trip watchdog timer? 
							buzzer_on();
							delay1ms(delay);
							buzzer_off();

							// send acknowledge
							uart_put_command(RF_CODE_ACK);
							break;
					}
					

				} else {
					// we should receive stop code at this point
					// if we do not, assume something was mangled and just go back to idle
					state = IDLE;
				}
				break;
		}
	}
	
	return rfCommand;
}

void rf_state_machine(RF_COMMAND_T command)
{
	static RF_STATE_T state = RF_IDLE;
	
	unsigned long txdata = 0;

	
	// FIXME: send() handles transmission repeats in rcswitch, so this might not be needed
	static uint8_t transmissionRepeats = 0;
	
	// this should be occuring after an entire uart packet is received (i.e., after SYNC_FINISH)
	switch(state)
	{
		case RF_IDLE:
			if (command == RF_RFOUT_START)
			{				
				state = RF_CHECK_REPEATS;
//				transmissionRepeats = 1;
			}
			break;

		// init and start RF transmit
		case RF_CHECK_REPEATS:			
			state = RF_TRANSMIT;
			
//			if (transmissionRepeats > 0)
//			{
//				state = RF_TRANSMIT;
//			}
			break;
		case RF_TRANSMIT:
				
			//PCA0_StopSniffing();
			disable_capture_interrupt();

			// bytes 0..1:	Tsyn
			// bytes 2..3:	Tlow
			// bytes 4..5:	Thigh
			// bytes 6..8:	24bit Data
			//buckets[0] = *(uint16_t *)&uartPacket[2];
			//buckets[1] = *(uint16_t *)&uartPacket[4];
			//buckets[2] = *(uint16_t *)&uartPacket[0];
			
			// FIXME: this may need some scrutiny for correctness
			txdata = ((unsigned long)uartPacket[6] << 16) | (uartPacket[7] << 8) | (uartPacket[8]);
			
			// FIXME: this is a bit of a hack, and does not seem to work!
			// we ultimately only keep bytes 6, 7, and 8
			//txdata = *(unsigned long *)&uartPacket[5];
			//txdata &= ~0xFF000000;

#if 1
			putstring("rf\r\n");
#endif
			
			//
			send(1, txdata, 24);
			
#if 1
			putstring("end\r\n");
#endif
			
			enable_capture_interrupt();
			
			state = RF_FINISHED;
			
//			transmissionRepeats--;
			
//			if (transmissionRepeats == 0)
//			{
//				state = RF_FINISHED;
//			}
			break;

		// wait until data got transfered
		case RF_FINISHED:
			
			// disable RF transmit
			tdata_off();

			uart_put_command(RF_CODE_ACK);

			state = RF_IDLE;

			break;

	}
}

// FIXME: consider calling this from within state machine
void radio_rfin(void)
{
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


#if 1

// send out over software uart for debugging help
void radio_decode_debug(void)
{
	// protocol index
	putstring("p:0x");
	puthex2(get_received_protocol());
	putc(' ');

	// bits received
	putstring("b:0x");
	puthex2(get_received_bitlength());
	putc(' ');
	
	// data
	putstring("d:0x");
	puthex2((get_received_value() >> 16) & 0xFF);
	puthex2((get_received_value() >>  8) & 0xFF);
	puthex2((get_received_value() >>  0) & 0xFF);
	
	// newline
	putc('\r');
	putc('\n');
}

#endif

#if 1

// FIXME: adapt to send out over software uart or for sniffing mode
void radio_decode_timings(void)
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

#endif