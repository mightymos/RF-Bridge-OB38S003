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

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// SFR declarations
#include <stdint.h>

// for printf_tiny()
//#include <stdio.h>

#include "delay.h"
#include "hal.h"
#include "portisch.h"
#include "portisch_command_format.h"
#include "portisch_protocols.h"
#include "portisch_serial.h"
#include "timer_interrupts.h"
#include "uart.h"
#include "util.h"


// example for sniffing (0xB1 command) and reply tranmission (0xB0 command) from my door sensor
//{"RfRaw":{"Data":"AA B1 03 0138 03B7 277C 28181909081908190819090908181908181818181908190818 55"}}

// 0xB1 to 0xB0 web convert tool (json format):
// https://bbconv.hrbl.pl/
// got this output from the tool:
// rfraw AA B0 21 03 08 0138 03B7 277C 28 18 19 09 08 19 08 19 08 19 09 09 08 18 19 08 18 18 18 18 19 08 19 08 18 55

// converter tool for raw byte format
// https://jonajona.nl/convertB1.html

// uart state machine
__xdata uart_state_t uart_state = IDLE;
__xdata uart_command_t uart_command = NONE;
__xdata uart_command_t last_sniffing_command;


// FIXME: add comment
rf_sniffing_mode_t last_sniffing_mode;

// used in multiple functions so made global for now
static __xdata uint8_t packetLength = 0;

// used for count down radio transmissions
__xdata uint8_t tr_repeats = 0;


// FIXME: comment on what this really does
bool blockReadingUART = false;

// sdcc manual section 3.8.1 general information
// requires interrupt definition to appear or be included in main
// sdccman sec. 3.8.1 indicates isr prototype must appear or be included in the file containing main
// it is probably more proper to achieve this through include files but also easy to omit
// and then things just will not work with no clear reason why, even though compilation is succcessful
#if defined(TARGET_BOARD_OB38S003)
    // supports tens of microseconds delays
    void timer0_isr(void) __interrupt (d_T0_Vector);
    // supports  milliseconds delays
    void timer1_isr(void) __interrupt (d_T1_Vector);
    // pca like capture mode for radio decoding
    void timer2_isr(void) __interrupt (d_T2_Vector);
    // hardware uart
    void uart_isr(void)   __interrupt (d_UART0_Vector);

#elif defined(TARGET_BOARD_EFM8BB1) || defined(TARGET_BOARD_EFM8BB1LCB)
    // timer0 was used for PCA in portisch, instead just use system clock
    // timer1 must be used for uart on this controller
    void uart_isr(void) __interrupt (UART0_VECTOR);
    // timer2 is used on demand to provide tens of microseconds delays
    void timer2_isr(void) __interrupt (TIMER2_VECTOR);
    // timer3 is used on demand to provide milliseconds delays
    void timer3_isr(void) __interrupt (TIMER3_VECTOR);
    // programmable counter for measuring time between edge transitions of radio pulses
    void pca0_isr(void) __interrupt (PCA0_VECTOR);

#else
    #error Please define TARGET_BOARD in makefile
#endif

//-----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enabled by default
// and may trigger before main() if for example external ram is cleared and takes too long
//-----------------------------------------------------------------------------
unsigned char __sdcc_external_startup(void)
{
    // pg. 218, sec. 20.3 disable watchdog timer
    disable_global_interrupts();
    
    // using hardware abstraction
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
	// tracks which byte we are currently receiving from uart
	static uint8_t position = 0;

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
                    init_delay_timer_ms(50);
                    buzzer_on();
                    // wait until timer has finished
                    wait_delay_timer_ms_finished();
                    buzzer_off();

                    // set desired RF protocol PT2260
                    sniffing_mode = STANDARD;
                    PCA0_DoSniffing();
                    
                    // redundant
                    //uart_command = RF_CODE_LEARN;

                    // start timeout timer
                    init_delay_timer_ms(30000);
                    break;
				case RF_CODE_RFOUT:
					// stop sniffing while handling received data
					PCA0_StopSniffing();

					uart_state = RECEIVING;
					tr_repeats = RF_TRANSMIT_REPEATS;
					position = 0;
					packetLength = 9;
					break;
				case RF_DO_BEEP:
					// stop sniffing while handling received data
					PCA0_StopSniffing();

					uart_state = RECEIVING;
					position = 0;
					packetLength = 2;
					break;
				case RF_ALTERNATIVE_FIRMWARE:
                    // we want an empty switch entry because
                    // no further data is expected but we want uart_command to not be set
                    // to NONE as in default switch entry
					break;
				case RF_CODE_SNIFFING_ON:
					sniffing_mode = ADVANCED;
					PCA0_DoSniffing();
                    // redundant
                    //uart_command          = RF_CODE_SNIFFING_ON;
					last_sniffing_command = RF_CODE_SNIFFING_ON;
					break;
				case RF_CODE_SNIFFING_OFF:
					// set desired RF protocol PT2260
					sniffing_mode = STANDARD;
					// re-enable default RF_CODE_RFIN sniffing
					PCA0_DoSniffing();
					uart_command          = RF_CODE_RFIN;
					last_sniffing_command = RF_CODE_RFIN;
					break;
				case RF_CODE_RFOUT_NEW:
					tr_repeats = RF_TRANSMIT_REPEATS;
					uart_state = RECEIVE_LENGTH;
					break;
				case RF_CODE_RFOUT_BUCKET:
					uart_state = RECEIVE_LENGTH;
					break;
				case RF_CODE_SNIFFING_ON_BUCKET:
					PCA0_DoSniffing();
                    // redundant
					//uart_command = RF_CODE_SNIFFING_ON_BUCKET;
					break;
				case RF_CODE_LEARN_NEW:
                    init_delay_timer_ms(50);
                    buzzer_on();
                    // wait until timer has finished
                    wait_delay_timer_ms_finished();
                    buzzer_off();

                    // enable sniffing for all known protocols
                    last_sniffing_mode = sniffing_mode;
                    sniffing_mode = ADVANCED;
                    PCA0_DoSniffing();
                    // redundant
                    //uart_command = RF_CODE_LEARN_NEW;

                    // start timeout timer
                    init_delay_timer_ms(30000);
                    break;
				case RF_CODE_ACK:
                    // FIXME: I do not think this comment matches what happens in code, need to examine
					// re-enable default RF_CODE_RFIN sniffing
                    PCA0_DoSniffing();
					uart_command = last_sniffing_command;
					uart_state = IDLE;
					break;
                case RF_RESET_MCU:
                    break;
				// unknown command
				default:
					uart_command = NONE;
					uart_state = IDLE;
					break;
			}
			break;

		// Receiving UART data length
		case RECEIVE_LENGTH:
			position = 0;
			packetLength = rxdata & 0xFF;
			if (packetLength > 0)
			{
				// stop sniffing while handling received data
                // FIXME: comment on why
				PCA0_StopSniffing();
                
				uart_state = RECEIVING;
			} else {
				uart_state = SYNC_FINISH;
			}
			break;

		// Receiving UART data
		case RECEIVING:
			RF_DATA[position] = rxdata & 0xFF;

			// DEBUG:
			//puthex2(RF_DATA[position]);
			//uart_putc(RF_DATA[position]);
			
			position++;

			// FIXME: uartPacket[] versus RF_DATA[] array length
			if (position == packetLength)
			{
				uart_state = SYNC_FINISH;
			} else if (position >= RF_DATA_BUFFERSIZE)
			{
                packetLength = RF_DATA_BUFFERSIZE;
                uart_state = SYNC_FINISH;
			}
			break;

		// wait and check for UART_SYNC_END
		case SYNC_FINISH:
			if ((rxdata & 0xFF) == RF_CODE_STOP)
			{
				uart_state = IDLE;
                
                // FIXME: comment on what this really does
                blockReadingUART = true;

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
                        blockReadingUART = false;
						break;
					case RF_CODE_RFOUT_BUCKET:
                        // conversion tool seems to show data length, then number of buckets, then number of repeats after 0xB0 command
                        // FIXME: why plus one?
						tr_repeats = RF_DATA[1] + 1;
                        
                        uint8_t num_buckets = RF_DATA[0];
                        
                        
                        uint8_t countIndex = 0;
                        uint8_t byteIndex = 0;
                        
                        uint8_t temp;
                        
                        
                        // because sdcc is little endian for 8051, we need to swap bytes to access integer bucket values by pointer later
                        while (countIndex < num_buckets)
                        {
                            //buckets_pointer[byteIndex] = ((buckets_pointer[byteIndex] << 8) | (buckets_pointer[byteIndex] >> 8));
                            //byteIndex++;
                            
                            // the location of the zeroth (first) timing bucket in RF_DATA[] starts at byte 2
                            byteIndex += 2;
                            
                            //
                            temp  = RF_DATA[byteIndex];
                            
                            // swap
                            RF_DATA[byteIndex]     = RF_DATA[byteIndex + 1];
                            RF_DATA[byteIndex + 1] = temp;
                            
                            countIndex++;
                        }
                        
                        // DEBUG:
                        //uart_putc(tr_repeats);
                        
						break;
				}
			}
			break;
	}
}

bool radio_state_machine(const uart_command_t command)
{
	bool completed = false;

	// DEBUG:
	//const uint16_t pulsewidths_dummy[3] = {350, 1050, 10850};
	//const uint8_t  packet_dummy[3]  = {0xA5, 0x5A, 0xA5};

    // for transmission
    uint16_t pulsewidths[3];
    uint8_t  tr_packet[3];

	// helps allow sendbuckets call to be more readable
	uint8_t start_size;
	uint8_t bit0_size;
	uint8_t bit1_size;
	uint8_t end_size;
	uint8_t bitcount;

	// do transmit of the data
	switch(rf_state)
	{
		// init and start RF transmit
		case RF_IDLE:

			tr_repeats--;

			PCA0_StopSniffing();

            // this chooses between the different transmission types
            switch (command)
            {
                case RF_CODE_RFOUT:
                    // byte 0..1:	Tsyn
                    // byte 2..3:	Tlow
                    // byte 4..5:	Thigh
                    // byte 6..8:	24bit Data
                    // sdcc is little endian so we would need to swap bytes for pointers to work
                    // low, high, sync order in array (from uart order is sync, low, high)
                    pulsewidths[0] = (RF_DATA[2] << 8) | RF_DATA[3];
                    pulsewidths[1] = (RF_DATA[4] << 8) | RF_DATA[5];
                    pulsewidths[2] = (RF_DATA[0] << 8) | RF_DATA[1];
                    
                    // data
                    tr_packet[0] = RF_DATA[6];
                    tr_packet[1] = RF_DATA[7];
                    tr_packet[2] = RF_DATA[8];
                    
                    // help make function call more readable
                    start_size = PROTOCOL_DATA[0].start.size;
                    bit0_size  = PROTOCOL_DATA[0].bit0.size;
                    bit1_size  = PROTOCOL_DATA[0].bit1.size;
                    end_size   = PROTOCOL_DATA[0].end.size;
                    bitcount   = PROTOCOL_DATA[0].bit_count;

                    // user supplied timings
                    SendBuckets(pulsewidths, PROTOCOL_DATA[0].start.dat, start_size, PROTOCOL_DATA[0].bit0.dat, bit0_size, PROTOCOL_DATA[0].bit1.dat, bit1_size, PROTOCOL_DATA[0].end.dat, end_size, bitcount, tr_packet);
                    
                    break;
                case RF_CODE_RFOUT_NEW:
                    // byte 0:		PROTOCOL_DATA index
                    // byte 1..:	Data
                    // FIXME: rcswitch treats "protocol 1" as index 0, so might need to make consistent with portisch
                    SendBucketsByIndex(RF_DATA[0], &RF_DATA[1]);
                    break;
                case RF_CODE_RFOUT_BUCKET:
                    uint8_t num_buckets = RF_DATA[0];
                    
                    // FIXME: I do not know what format this is, does it match 0xB0 on the wiki?
                    // byte 0:				number of buckets: k
                    // byte 1:				number of repeats: r
                    // byte 2*(1..k):		bucket time high
                    // byte 2*(1..k)+1:		bucket time low
                    // byte 2*k+2..N:		RF buckets to send
                    //uint16_t* buckets = (uint16_t *)(RF_DATA + 2);
                    
                    // find the start of the data by skipping over the number of buckets times two and two bytes for numbers of buckets and number of repeats
                    uint8_t* rfdata = RF_DATA + (num_buckets << 1) + 2;
                    
                    // subtract out two bytes for number of buckets and number of repeats
                    // then subtract out number of buckets multiplied by 2
                    uint8_t data_len = packetLength - 2 - (num_buckets << 1);

                    // pointer
                    // this is a global variable with maximum size seven
                    uint16_t* buckets_pointer = (uint16_t *)(RF_DATA + 2);

                    // 0xB0 transmission using timings that generally would have been sniffed with 0xB1 mode
                    SendRFBuckets(buckets_pointer, rfdata, data_len);
                    
                    break;
            }
            
            
			// causes ping pong between idle and finished state until we reach zero repeat index
			rf_state = RF_FINISHED;
			
			break;

		// wait until data got transfered
		case RF_FINISHED:
			if (tr_repeats == 0)
			{
				// disable RF transmit
				tdata_off();

				completed = true;
			} else {
				rf_state = RF_IDLE;
			}
			break;
	}

	return completed;
}

// uses timer based delays so timer 0,1 must be setup before use
void startup_blink(void)
{
    // double blink
    led_on();
    init_delay_timer_ms(1000);
    wait_delay_timer_ms_finished();
    led_off();
    
    led_on();
    init_delay_timer_ms(1000);
    wait_delay_timer_ms_finished();
    led_off();
}

//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
void main (void)
{
	// for buzzer (milliseconds)
	//const uint16_t startupDelay = 100;
	// longer for LED
	//const uint16_t startupDelay = 3000;

	// 
	unsigned int rxdata = UART_NO_DATA;

	// FIXME: add comment
    uint16_t bucket = 0;
    

	// FIXME: add comment
    uint16_t idleResetCount = 0;


	// prefer bool type in internel ram to take advantage of bit addressable locations
	bool result;

    // add comment?
    set_clock_mode();

	// call hardware initialization routine
#if defined(TARGET_BOARD_OB38S003)
    init_port_pins();
#elif defined(TARGET_BOARD_EFM8BB1) || defined(TARGET_BOARD_EFM8BB1LCB)
    // the crossbar on this microcontroller makes initialization more complicated
    init_port_pins_for_serial();
#else
    #error Please define TARGET_BOARD in makefile
#endif

	// set default pin states
	led_on();
	buzzer_off();
	tdata_off();
    
    // DEBUG:
    // on some boards, "debug pin" is actually buzzer
    // so we do not want to use it for debugging unless buzzer has been removed
    //debug_pin01_off();
    
    
	// baud rate is 19200, 8 data bits, 1 stop bit, no parity
	// polled version basically sets TI flag so putchar() does not get stuck in an infinite loop
	//UART0_initStdio();
	// enable uart (with interrupts)
	//UART0_init(UART0_RX_ENABLE, UART0_WIDTH_8, UART0_MULTIPROC_DISABLE);
    init_uart();
    uart_rx_enabled();
    
    // FIXME: despite rcswitch doing it this way, it seems like we only want to enable after setting uart timer baud rate
    // sets TI=1 so ring buffer logic works
    init_serial_interrupt();
    // enable actual interrupt
    enable_serial_interrupt();
    
#if defined(TARGET_BOARD_OB38S003)
    // supports microseconds and milliseconds delays respectively
    init_timer0_8bit_autoreload();
    init_timer1_8bit_autoreload();
    // pca like capture
    init_timer2_as_capture();
    // enable for eventually use once timer(s) are enabled for a delay
    enable_timer0_interrupt();
    enable_timer1_interrupt();
#elif defined(TARGET_BOARD_EFM8BB1) || defined(TARGET_BOARD_EFM8BB1LCB)
    // pca used timer0 on portisch, we just use the pca counter itself now
    // uart with 19200 baud, uart must use timer1 on efm8bb1
    init_timer1_8bit_autoreload(TIMER1_UART0);
    timer1_run();
    
    //
    pca0_init();
    
    // enable interrupts in anticipation of enabling timer for on demand delays
    enable_timer2_interrupt();
    enable_timer3_interrupt();
    
    //FIXME: in rcswitch we did pca0_run() here, but it happens in DoSniffing() for portisch
#endif

    // FIXME: this is slightly different to rcswitch initialization, need to decide what makes the most sense
    enable_capture_interrupt();


	// start sniffing be default
	// set desired sniffing type to PT2260
	sniffing_mode = STANDARD;
    
    
	// enable global interrupts needs to happen prior to use of timer based delays (e.g., PCA0_Sniffing() uses delay)
	enable_global_interrupts();
    
    // uses timer based delays so timer 0,1 must be setup before use
    startup_blink();
    
#if 1
    //
	//sniffing_mode = ADVANCED;
	PCA0_DoSniffing();

    // FIXME: double check if this comment is accurate
	// basically chooses between standard (i.e., three buckets) and advanced output (i.e. up to seven buckets)
	uart_command          = RF_CODE_RFIN;
	last_sniffing_command = RF_CODE_RFIN;
	//last_sniffing_command = RF_CODE_SNIFFING_ON;
	//uart_command          = RF_CODE_SNIFFING_ON;
#else
    PCA0_StopSniffing();
#endif


    
    // FIXME: function empty on efm8bb1, because unknown if receiver has enable pin
    radio_receiver_on();

    
    // DEBUG:
    // startup
    //requires code and memory space, which is in short supply
    //but good to check that polled uart is working
    //printf_tiny("startup...\r\n");
    //uart_put_command(RF_CODE_ACK);

	// FIXME: this is essentially our watchdog timer initialization
	//        but it needs to be done with the abstraction layer
	// instead of the default divide by 8, we do divide by 1 so that low frequency oscillator is at 80 kHz
	// sec. 20.4.1 WDTCN watchdog timer control
	// e.g., (1/80000)*4^(5+3) = 0.8192
	// so that watchdog timer will reset if not fed every 819 millisecs
	//LFO0CN |= OSCLD__DIVIDE_BY_1;

	// we disabled watchdog at startup in case external ram needs to be cleared etc.
	// now we enable
	//WDT0_start();
    enable_watchdog();
	
	while (true)
	{
		// reset Watch Dog Timer
		refresh_watchdog();


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
		if (!blockReadingUART)
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
            {
				idleResetCount = 0;
			} else {
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
			// do original learning, new RF code learning
			case RF_CODE_LEARN:
			case RF_CODE_LEARN_NEW:

#if 1
				// check if a RF signal got decoded
				if ((RF_DATA_STATUS & RF_DATA_RECEIVED_MASK) != 0)
				{
					init_delay_timer_ms(200);
					buzzer_on();
					// wait until timer has finished
					wait_delay_timer_ms_finished();
					buzzer_off();

					switch(uart_command)
					{
						case RF_CODE_LEARN:
							PCA0_DoSniffing();
                            uart_command = last_sniffing_command;
                            
							uart_put_RF_Data_Standard(RF_CODE_LEARN_OK);
							break;

						case RF_CODE_LEARN_NEW:
							sniffing_mode = last_sniffing_mode;
							PCA0_DoSniffing();
                            uart_command = last_sniffing_command;
							uart_put_RF_Data_Advanced(RF_CODE_LEARN_OK_NEW, RF_DATA_STATUS & 0x7F);
							break;
					}

					// clear RF status
					RF_DATA_STATUS = 0;

					// enable interrupt for RF receiving
					//PCA0CPM0 |= PCA0CPM0_ECCF__ENABLED;
                    enable_capture_interrupt();

					// enable UART again
					blockReadingUART = false;
				}
				// check for learning timeout
				else if (is_delay_timer_ms_finished())
				{
					init_delay_timer_ms(1000);
					buzzer_on();
					// wait until timer has finished
					wait_delay_timer_ms_finished();
					buzzer_off();

					// send not-acknowledge
					switch(uart_command)
					{
						case RF_CODE_LEARN:
                            // send uart command
                            uart_put_command(RF_CODE_LEARN_KO);

                            // enable UART again
                            blockReadingUART = false;

                            // restart sniffing in its previous mode
                            PCA0_DoSniffing();
                            uart_command = last_sniffing_command;
							break;

						case RF_CODE_LEARN_NEW:
                            // send uart command
                            uart_put_command(RF_CODE_LEARN_KO_NEW);

                            // enable UART again
                            blockReadingUART = false;

                            // restart sniffing in its previous mode
                            PCA0_DoSniffing();
                            uart_command = last_sniffing_command;
							break;
					}
				}
				else
				{
					// handle new received buckets
					if (buffer_out(&bucket))
                    {
						HandleRFBucket(bucket & 0x7FFF, (bool)((bucket & 0x8000) >> 15));
                    }
				}
				break;
                
#endif
                
			// do original sniffing
			case RF_CODE_RFIN:
			case RF_CODE_SNIFFING_ON:

				// check if a RF signal got decoded
				if ((RF_DATA_STATUS & RF_DATA_RECEIVED_MASK) != 0)
				{
					switch(uart_command)
					{
						case RF_CODE_RFIN:
							// we share a buffer between radio and uart - assume an atomic lock would work better here
							disable_capture_interrupt();
							
							//we read RF_DATA[] so do not want decoding writing to it while trying to read it
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
					// disable interrupt for radio receiving while reading buffer
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
			case RF_CODE_RFOUT:
			case RF_CODE_RFOUT_NEW:
			case RF_CODE_RFOUT_BUCKET:
			{
				// only do the job if all data got received by UART
				if (uart_state != IDLE)
					break;

				// if statement allows repeat transmissions
                // and pass in the type of transmission to the state machine (e.g. 0xA5 by timing, 0xA8 by protocol, or 0xB0 by sniffed bucket timing)
				if (radio_state_machine(uart_command))
				{
					// indicate completed all transmissions
					uart_put_command(RF_CODE_ACK);

					// FIXME: need to examine this logic
					// restart sniffing in its previous mode
					PCA0_DoSniffing();
                    
                    blockReadingUART = false;

					// change back to previous command (i.e., not rfout)
					uart_command = last_sniffing_command;
				}
				break;
			}
			case RF_CODE_SNIFFING_ON_BUCKET:

				// check if a RF signal got decoded
				if ((RF_DATA_STATUS & RF_DATA_RECEIVED_MASK) != 0)
				{
					//
					disable_capture_interrupt();
					
					//
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
						Bucket_Received(bucket & 0x7FFF, (bool)((bucket & 0x8000) >> 15));
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

                // as best I can tell, cast using pointers do not work because sdcc is little endian on 8051
                // so we would need to swap bytes and then cast to uint16
                // but since it's just two bytes anyway, do it with bit shifts instead
                //bucket = *(uint16_t *)&RF_DATA[0];
                //bucket = *(uint16_t *)&RF_DATA[1];
                
                // reuse uint16_t variable used elsewhere
                bucket = (RF_DATA[0] << 8) | RF_DATA[1];
                
                
                // we finally avoided nop based delays to save on code space
                //delay1ms(bucket);
                init_delay_timer_ms(bucket);
                wait_delay_timer_ms_finished();

                
				buzzer_off();

				// send acknowledge
				// send uart command
				uart_put_command(RF_CODE_ACK);
                
                blockReadingUART = false;
                
				uart_command = last_sniffing_command;
				break;
            case RF_RESET_MCU:
                
                // force the microcontroller to reset
                reset_mcu();
                
                // we should never reach this because mcu should reset
                while (1)
                {
                    startup_blink();
                }

                break;
			// host was requesting the firmware version
			case RF_ALTERNATIVE_FIRMWARE:

				// send firmware version
				uart_put_command(FIRMWARE_VERSION);
                
                blockReadingUART = false;
                
				uart_command = last_sniffing_command;
				break;

			default:
				// FIXME: not sure if this makes sense
				uart_command = NONE;
			break;
		} //switch(uart_command)
	} //while (1)
}
