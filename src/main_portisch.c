//=========================================================
// Jonathan Armstrong - Port to SDCC compiler
// Silicon Labs IDE 5.50.00 seems to the last IDE that makes compiling with SDCC possible
//=========================================================

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#if !defined(TARGET_BOARD_EFM8BB1) && !defined(TARGET_BOARD_OB38S003) && !defined(TARGET_BOARD_EFM8BB1LCB)
	#error Please define TARGET_BOARD in makefile
#endif

#define EFM8PDL_PCA0_USE_ISR        1
#define EFM8PDL_UART0_USE_POLLED    0
// FIXME: not used anywhere else apparently
//#define EFM8PDL_UART0_USE_BUFFER
// this seems to start sniffing by default as opposed to waiting for command over uart
#define SNIFFING_ON_AT_STARTUP

// FIXME: not sure what is desired or expected
#define FIRMWARE_VERSION 0x03

// for printf_tiny()
//#include <stdio.h>

#include "delay.h"
#include "hal.h"
#include "portisch.h"
#include "portisch_macros.h"
#include "portisch_protocols.h"
#include "portisch_states.h"
#include "uart.h"

#include "timer_interrupts.h"

#include <stdint.h>

// uart state machine
__xdata uart_state_t   uart_state = IDLE;
__xdata uart_command_t uart_command = NONE;
__xdata uart_command_t idle_uart_command;

// buffer
__xdata uint8_t uartPacket[10];


// sdcc manual section 3.8.1 general information
#if defined(TARGET_BOARD_OB38S003)
// for software uart
// FIXME: if reset pin is set to reset function, instead of gpio, does this interfere with anything (e.g., software serial?)
extern void tm0(void)        __interrupt (d_T0_Vector);
// tick
extern void timer1_isr(void) __interrupt (d_T1_Vector);
// pca like capture mode for radio decoding
extern void timer2_isr(void) __interrupt (d_T2_Vector);
// hardware uart
extern void uart_isr(void)   __interrupt (d_UART0_Vector);

#elif defined(TARGET_BOARD_EFM8BB1)
extern void tm0(void)        __interrupt (TIMER0_VECTOR);
extern void timer2_isr(void) __interrupt (TIMER2_VECTOR);
extern void uart_isr(void)   __interrupt (UART0_VECTOR);
extern void pca0_isr(void)   __interrupt (PCA0_VECTOR);

#elif defined(TARGET_BOARD_EFM8BB1LCB)

// software uart
extern void tm0(void)        __interrupt (TIMER0_VECTOR);
// tick
extern void timer2_isr(void) __interrupt (TIMER2_VECTOR);
// hardware uart (uses timer 1)
extern void uart_isr(void)   __interrupt (UART0_VECTOR);
// radio decoding
extern void pca0_isr(void)   __interrupt (PCA0_VECTOR);

#else
	#error Please define TARGET_BOARD in makefile
#endif

//-----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
unsigned char __sdcc_external_startup(void)
{
    disable_watchdog();
    
    return 0;
}


#if 0

void serial_loopback(void)
{
	volatile unsigned int rxdata = UART_NO_DATA;

	// check if serial transmit buffer is empty
	if(!is_uart_tx_buffer_empty())
	{
	    if (is_uart_tx_finished())
	    {
	        // if not empty, set TI, which triggers interrupt to actually transmit
	        uart_init_tx_polling();
	    }
	}

	// check if something got received by UART
	// only read data from uart if idle
	if (!is_uart_ignored())
    {
		rxdata = uart_getc();
	} else {
		rxdata = UART_NO_DATA;
    }

	if (rxdata == UART_NO_DATA)
	{
		led_on();
	} else {
		uart_putc(rxdata & 0xff);
	}
    
}

#endif



void uart_state_machine(const unsigned int rxdata)
{
	// debug: echo sent character
	//uart_putc(rxdata & 0xff);

	// FIXME: add comment
	//idleResetCount = 0;

	__xdata rf_state_t rf_state;

    __xdata uint8_t tr_repeats = 0;

	// need to specify volatile so optimizer does not remove
	// given that it is set by external data input to uart
	volatile __xdata uint8_t packetLength = 0;
	// FIXME: add comment
	__xdata uint8_t position = 0;

	// state machine for UART
	switch(uart_state)
	{
		// check if start sequence got received
		case IDLE:
			if ((rxdata & 0xFF) == RF_CODE_START)
            {
				uart_state = SYNC_INIT;
            }
			break;

		// sync byte got received, read command
		case SYNC_INIT:
			uart_command = rxdata & 0xFF;
			uart_state = SYNC_FINISH;

			// check if some data needs to be received
			switch(uart_command)
			{
				case RF_CODE_LEARN:
					break;
				case RF_CODE_RFOUT:
					// stop sniffing while handling received data
					//PCA0_StopSniffing();
					//rf_state = RF_IDLE;
					//uart_state = RECEIVING;
					//tr_repeats = RF_TRANSMIT_REPEATS;
					//position = 0;
					//packetLength = 9;
					break;
				case RF_DO_BEEP:
					// stop sniffing while handling received data
					//PCA0_StopSniffing();
					//rf_state = RF_IDLE;
					uart_state = RECEIVING;
					position = 0;
					packetLength = 2;
					break;
				case RF_ALTERNATIVE_FIRMWARE:
					break;
				case RF_CODE_SNIFFING_ON:
					//sniffing_mode = ADVANCED;
					//PCA0_DoSniffing();
					//last_sniffing_command = RF_CODE_SNIFFING_ON;
					//rf_state = RF_IDLE;
					break;
				case RF_CODE_SNIFFING_OFF:
					// set desired RF protocol PT2260
					//sniffing_mode = STANDARD;
					// re-enable default RF_CODE_RFIN sniffing
					//PCA0_DoSniffing();
					//last_sniffing_command = RF_CODE_RFIN;
					//rf_state = RF_IDLE;
					break;
				case RF_CODE_RFOUT_NEW:
					//tr_repeats = RF_TRANSMIT_REPEATS;
					// no break
				case RF_CODE_RFOUT_BUCKET:
					//uart_state = RECEIVE_LEN;
					break;
				case RF_CODE_SNIFFING_ON_BUCKET:
					//last_sniffing_command = PCA0_DoSniffing();
					//rf_state = RF_IDLE;
					break;
				case RF_CODE_LEARN_NEW:
					break;
				case RF_CODE_ACK:
					// re-enable default RF_CODE_RFIN sniffing
					//last_sniffing_command = PCA0_DoSniffing();
					uart_state = IDLE;
					rf_state = RF_IDLE;
					break;

				// unknown command
				default:
					uart_command = NONE;
					uart_state = IDLE;
					break;
			}
			break;

		// Receiving UART data length
		case RECEIVE_LEN:
			position = 0;
			packetLength = rxdata & 0xFF;
			if (packetLength > 0)
			{
				// stop sniffing while handling received data
				PCA0_StopSniffing();
				rf_state = RF_IDLE;
				uart_state = RECEIVING;
			} else {
				uart_state = SYNC_FINISH;
			}
			break;

		// Receiving UART data
		case RECEIVING:
			uartPacket[position] = rxdata & 0xFF;
			position++;

			if (position >= packetLength)
			{
				uart_state = SYNC_FINISH;
			}
			break;

		// wait and check for UART_SYNC_END
		case SYNC_FINISH:
			if ((rxdata & 0xFF) == RF_CODE_STOP)
			{
				uart_state = IDLE;

				// check if ACK should be sent
				switch(uart_command)
				{
					case RF_CODE_LEARN:
					case RF_CODE_SNIFFING_ON:
					case RF_CODE_SNIFFING_OFF:
					case RF_CODE_RFIN:
					case RF_CODE_SNIFFING_ON_BUCKET:
						// send acknowledge
						uart_put_command(RF_CODE_ACK);
					case RF_CODE_ACK:
						break;
					case RF_CODE_RFOUT_BUCKET:
						tr_repeats = uartPacket[1] + 1;
						break;
				}
			}
			break;
	}
}

//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
void main (void)
{
	// for buzzer (milliseconds)
	//const uint16_t startupDelay = 100;
	// longer for LED
	__code uint16_t startupDelay = 3000;

	// changed by external hardware, so must specify volatile type so not optimized out
	volatile unsigned int rxdata = UART_NO_DATA;


	rf_state_t rf_state = RF_IDLE;
    uint16_t bucket = 0;
    
    //__xdata uint16_t index = 0;

	// FIXME: add comment
    __xdata uint16_t idleResetCount = 0;

	// prefer bool type in internel ram to take advantage of bit addressable locations
	bool result;


	// call hardware initialization routine
	//enter_DefaultMode_from_RESET();
    init_port_pins();

	// set default pin states
	led_on();
	buzzer_off();
	tdata_off();


	// DEBUG:
	//debug_pin0_off();



    init_uart();
    uart_rx_enabled();
    
    // hardware serial interrupt
    init_serial_interrupt();
	enable_serial_interrupt();
    


	// start sniffing if enabled by default
#if defined(SNIFFING_ON_AT_STARTUP)
	// set desired sniffing type to PT2260
	sniffing_mode = STANDARD;
	//sniffing_mode = ADVANCED;
	PCA0_DoSniffing();
	rf_state = RF_IDLE;

	// FIXME: add comment
	idle_uart_command = RF_CODE_RFIN;
	uart_command = RF_CODE_RFIN;
	//uart_command = RF_CODE_SNIFFING_ON_BUCKET;
#else
	PCA0_StopSniffing();
	rf_state = RF_IDLE;
#endif

#if 1
    // startup buzzer (can be annoying during development)
	// use LED instead (for development)
	//buzzer_on();
	led_on();
	delay1ms(startupDelay);

	//buzzer_off();
	led_off();
#endif

    
	// enable global interrupts
	enable_global_interrupts();
    

    
    // startup
    //requires code and memory space, which is in short supply
    //but good to check that polled uart is working
    //printf_tiny("startup...\r\n");
    //uart_put_command(RF_CODE_ACK);

	while (true)
	{
		// reset Watch Dog Timer
		//WDT0_feed();


#if 0
		// DEBUG: infinite loop to echo back serial characters
		while (true)
		{
			serial_loopback();
		}
#endif

#if 1
		// check if something got received by UART
		// only read data from uart if idle
		if (true)
        {
			rxdata = uart_getc();
		} else {
			rxdata = UART_NO_DATA;
        }
        
#endif
        
        

#if 1
        // check if serial transmit buffer is empty
        if(!is_uart_tx_buffer_empty())
        {
            if (is_uart_tx_finished())
            {
                // if not empty, set TI, which triggers interrupt to actually transmit
                uart_init_tx_polling();
            }
        }
        
#endif



		if (rxdata == UART_NO_DATA)
		{

#if 1
			// FIXME: the magic numbers make this difficult to understand
			// but seems to reset uart if it sits in non-idle state
			// for too long without receiving any more data
			if (uart_state == IDLE)
				idleResetCount = 0;
			else
			{
				idleResetCount += 1;
			
				if (idleResetCount > 30000)
				{
					idleResetCount = 0;
					uart_state = IDLE;
					uart_command = NONE;
				}
			}
#endif
		}
		else
		{
			uart_state_machine(rxdata);
		}

		/*------------------------------------------
		 * check command byte
		 ------------------------------------------*/
		switch(uart_command)
		{
			case NONE:
				break;
			// do original sniffing
			case RF_CODE_RFIN:
			case RF_CODE_SNIFFING_ON:

				// check if a RF signal got decoded
				if ((RF_DATA_STATUS & RF_DATA_RECEIVED_MASK) != 0)
				{
					switch(uart_command)
					{
						case RF_CODE_RFIN:
							disable_capture_interrupt();
							uart_put_RF_Data_Standard(RF_CODE_RFIN);
							enable_capture_interrupt();
							break;

						case RF_CODE_SNIFFING_ON:
							uart_put_RF_Data_Advanced(RF_CODE_SNIFFING_ON, RF_DATA_STATUS & 0x7F);
							break;
					}

					// clear RF status
					RF_DATA_STATUS = 0;

					// enable interrupt for RF receiving
					enable_capture_interrupt();
				}
				else
				{
					// disable interrupt for RF receiving while reading buffer
					disable_capture_interrupt();
					result = buffer_out(&bucket);
					// FIXME: reenable (should store previous and just restore that?)
					enable_capture_interrupt();
					// handle new received buckets
					if (result)
                    {
						HandleRFBucket(bucket & 0x7FFF, (bool)((bucket & 0x8000) >> 15));
                    }
				}
				break;
			case RF_CODE_SNIFFING_ON_BUCKET:

				// check if a RF signal got decoded
				if ((RF_DATA_STATUS & RF_DATA_RECEIVED_MASK) != 0)
				{
					// disable interrupt for RF receiving during uart transfer
					disable_capture_interrupt();
					
					uart_put_RF_buckets(RF_CODE_SNIFFING_ON_BUCKET);

					// clear RF status
					RF_DATA_STATUS = 0;

					// enable interrupt for RF receiving
					enable_capture_interrupt();
				}
				else
				{
					// do bucket sniffing handling
					result = buffer_out(&bucket);
					if (result)
					{
						Bucket_Received(bucket & 0x7FFF, (bool)((bucket & 0x8000) >> 15), &rf_state);
					}
				}

			break;

			// do a beep
			case RF_DO_BEEP:
				// only do the job if all data got received by UART
				if (uart_state != IDLE)
					break;

				// this is blocking unfortunately
				buzzer_on();
				delay1ms(*(uint16_t *)&uartPacket[1]);
				buzzer_off();

				// send acknowledge
				// send uart command
				uart_put_command(RF_CODE_ACK);
				uart_command = idle_uart_command;
				break;

			// host was requesting the firmware version
			case RF_ALTERNATIVE_FIRMWARE:

				// send firmware version
				uart_put_command(FIRMWARE_VERSION);
				uart_command = idle_uart_command;
				break;

			default:
				// FIXME: not sure if this makes sense
				uart_command = NONE;
			break;
		} //switch(uart_command)
	} //while (1)
}
