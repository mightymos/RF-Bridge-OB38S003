//=========================================================
//=========================================================

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// SFR declarations
#include <8052.h>

#include <stdint.h>
#include <stdlib.h>

// printf() requires a decent amount of code space and ram which we would like to avoid
// and printf is not particularly helpful once we follow serial packet structure required of esp8286 wifi chip
//#include <stdio.h>

// borrowed from area-8051 uni-stc HAL...
#include "delay.h"

#include "globals.h"
#include "initdevice.h"
#include "rcswitch.h"
#include "timer.h"
#include "uart.h"


// DEBUG: bench sensor
//21:47:05.533 RSL: RESULT = {"Time":"2023-08-05T21:47:05","RfReceived":{"Sync":10280,"Low":340,"High":1010,"Data":"80650A","RfKey":"None"}}
//19:16:47.313 RSL: RESULT = {"Time":"2023-08-05T19:16:47","RfReceived":{"Sync":10280,"Low":340,"High":1010,"Data":"80650E","RfKey":"None"}}


// FIXME: move or remove this comment
// removed SYN470R radio receiver on one of my boards and tied reset pin to rdata input pin for testing purposes
// so basically enabling loopback uses reset pin to apply radio transmission signals, as opposed to tdata pin


// sdccman sec. 3.8.1 indicates isr prototype must appear in the file containing main
extern void timer0_isr(void) __interrupt (1);
//extern void timer1_isr(void) __interrupt (3);
extern void uart_isr(void)   __interrupt (4);
extern void timer2_isr(void) __interrupt (5);


//-----------------------------------------------------------------------------
// FIXME: this is sometimes needed to initialize external ram, etc.
//-----------------------------------------------------------------------------
void _sdcc_external_startup(void)
{

}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void uart_state_machine(const unsigned int rxdata)
{
    // FIXME: need to check what appropriate initialization values are
    static UART_STATE_T state = IDLE;
    UART_COMMAND_T command = NONE;
    
    uint16_t bucket = 0;

    // FIXME: need to know what initialization value is appropriate
	uint8_t position = 0;
    
    // track count of entries into function
    static uint16_t idleResetCount = 0;

    // FIXME: this seems to reset state machine if we do not receive data after some time
    if (rxdata == UART_NO_DATA)
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
            if ((rxdata & 0xFF) == RF_CODE_START)
            {
                state = SYNC_INIT;
            }
            
            break;

        // sync byte got received, read command
        case SYNC_INIT:
            command = rxdata & 0xFF;
            
            // FIXME: not sure if setting this here is correct
            state = SYNC_FINISH;

            // check if some data needs to be received
            switch(command)
            {
                case RF_CODE_LEARN:
                    break;
                // do original sniffing
                case RF_CODE_RFIN:
                    break;
                case RF_CODE_RFOUT:
                    break;
                case RF_DO_BEEP:
                    // FIXME: replace with timer rather than delay(), although appears original code was blocking too
                    buzzer_on();
                    delay1ms(50);
                    buzzer_off();

                    // send acknowledge
                    uart_put_command(RF_CODE_ACK);
                    break;
                case RF_ALTERNATIVE_FIRMWARE:
                    uart_put_command(RF_CODE_ACK);
                    uart_put_command(FIRMWARE_VERSION);
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
            //gRFData[position] = rxdata & 0xFF;
            //position++;

            //if (position == gLength)
            //{
            //    state = SYNC_FINISH;
            //}
            //else if (position >= RF_DATA_BUFFERSIZE)
            //{
            //    gLength = RF_DATA_BUFFERSIZE;
            //    state = SYNC_FINISH;
            //}
            break;

        // wait and check for UART_SYNC_END
        case SYNC_FINISH:
            if ((rxdata & 0xFF) == RF_CODE_STOP)
            {
                state = IDLE;
            }
            break;
    }
}

//-----------------------------------------------------------------------------
// toggle reset and led pins so that timing can be inspected on oscilloscope
// ----------------------------------------------------------------------------
void blink_task(void)
{
    static bool pinState = true;
    
    // DEBUG:
    //printf("blink_task()\r\n");
    
    if (pinState)
    {
        led_on();
        reset_pin_on();
        //tdata_on();
    } else {
        led_off();
        reset_pin_off();
        //tdata_off();
    }
    
    pinState = !pinState;
    
    // about half a second
    //reload_timer1(50000);
    
    // about six milliseconds
    delay1ms(6);
}



// FIXME: some of these function names really need fixing
void radio_decode_report(void)
{
	uint8_t i = 0;
	uint8_t b = 0;

    //printf("%c", RF_CODE_START);
    //printf("%c", RF_CODE_RFIN);
    uart_putc(RF_CODE_START);
    uart_putc(RF_CODE_RFIN);
    
    // sync, low, high timings
    //printf("%c", (timings[0] >> 8) & 0xFF);
    //printf("%c", timings[0] & 0xFF);
    uart_putc((timings[0] >> 8) & 0xFF);
    uart_putc(timings[0] & 0xFF);

    
    // FIXME: not sure if we should compute an average or something
    // FIXME: handle inverted signal?
    //printf("%c", (timings[2] >> 8) & 0xFF);
    //printf("%c",  timings[2] & 0xFF);
    //printf("%c", (timings[1] >> 8) & 0xFF);
    //printf("%c",  timings[1] & 0xFF);
    uart_putc((timings[2] >> 8) & 0xFF);
    uart_putc( timings[2] & 0xFF);
    uart_putc((timings[1] >> 8) & 0xFF);
    uart_putc( timings[1] & 0xFF);
    
    // data
    // FIXME: super strange that shifting by ZERO works but omitting the shift does not
    //printf("%c", (get_received_value() >> 16) & 0xFF);
    //printf("%c", (get_received_value() >>  8) & 0xFF);
    //printf("%c", (get_received_value() >>  0) & 0xFF);
    uart_putc((get_received_value() >> 16) & 0xFF);
    uart_putc((get_received_value() >>  8) & 0xFF);
    uart_putc((get_received_value() >>  0) & 0xFF);
    
    
    //printf("%c", RF_CODE_STOP);
    uart_putc(RF_CODE_STOP);
}

#if 0
    // we avoid use of printf but may be able to adapt this to wifi serial protocol format?
    void radio_decode_debug(void)
    {
        printf("Received: ");
        printf("0x%lx", get_received_value() );
        printf(" / ");
        printf("%u", get_received_bitlength() );
        printf("bit ");
        printf("Protocol: ");
        printf("%u", get_received_protocol() );
        
        printf("\r\n");
    }
#endif

#if 0
    void startup_debug(const __idata unsigned char* stackStart)
    {
        // just demonstrate serial uart is working basically
        //printf("Startup...\r\n");
        
        //printf("Start of stack: %p\r\n", stackStart);
        //printf("num. of protocols: %u\r\n", numProto);

        // DEBUG: demonstrates that we cannot write above SP (stack pointer)
        //*gStackStart       = 0x5a;
        //*(gStackStart + 1) = 0x5a;
        //printf("gStackStart[%p]: 0x%02x\r\n", gStackStart,   *gStackStart);
        //printf("gStackStart[%p]: 0x%02x\r\n", gStackStart+1, *(gStackStart + 1));
    }
#endif

void startup_beep()
{
    // FIXME: startup beep helpful or annoying?
    buzzer_on();
    delay1ms(20);
    buzzer_off();   
}

void startup_blink()
{
    // startup blink
    led_on();
    delay1ms(500);
    led_off();
}

//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
    // FIXME: avoid magic numbers
    //__xdata uint8_t radioBytes[3];

    // holdover from when we considered using rtos
    const __idata unsigned char* stackStart = (__idata unsigned char*) SP + 1;

    
    
    // track elapsed time for doing something periodically (e.g., every 10 seconds)
    unsigned long previousTimeSendRadio = 0;
    unsigned long previousTimeHeartbeat = 0;
    unsigned long elapsedTimeSendRadio;
    unsigned long elapsedTimeHeartbeat;
    unsigned long heartbeat = 0;
    
    
    // upper eight bits hold error or no data flags
 	unsigned int rxdata = UART_NO_DATA;
    
    // this is kind of a holdover from trying an rtos...
    //stackStart = (__idata unsigned char*) SP + 1;
    

	// hardware initialization
    set_clock_1t_mode();
	init_port_pins();
    init_uart();
    
    // provides one millisecond tick
    init_timer0();
    
    // FIXME: comment
    //init_timer1();
    
    //captures pulse lengths for received radio signals
    init_timer2_capture();

    // individual interrupts
    init_capture_interrupt();
    init_serial_interrupt();


	// set default pin levels
    led_off();
    buzzer_off();
    tdata_off();
    reset_pin_off();
    
    // FIXME: need to set startup reset pin state in case it is used?
    //reset_off();
    
    // DEBUG: disable radio (not working, does not set pin high?)
    //radio_on();
    
    //startup_beep();
    //startup_debug(stackStart);
    startup_blink();

	// enable interrupts
    enable_global_interrupts();
    

	while (true)
	{
		// FIXME: enable watch dog timer functionality
		//WDT0_feed();


        // try to get one byte from uart rx buffer
		rxdata = uart_getc();
        
#if 0
        // DEBUG: echo back received character
        //delay1ms(500);
        //if (rxdata != UART_NO_DATA)
        //{
        //    // if buffer is not empty, echo back byte by transmitting
        //    led_toggle();
        //    uart_putc(rxdata);
        //}
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

        

#if 1
        // process serial receive data
        if (rxdata != UART_NO_DATA)
        {
            uart_state_machine(rxdata);
        }
        
#endif


#if 1

    // have we received a succesfully decoded radio packet
    if (available())
    {
        // this is needed to avoid corrupting the currently received packet
        // a better way would be some type of queue probably
        //disable_capture_interrupt();
        disable_global_interrupts();
        
        // formatted for tasmota
        radio_decode_report();
        
        // formatted like rc-switch example
        //radio_decode_debug();


        reset_available();
        
        //enable_capture_interrupt();
        enable_global_interrupts();
    }
    
#endif
        
        
#if 0
        // DEBUG:
        // display serial message about every 10 seconds
        elapsedTimeHeartbeat = get_elapsed_time(previousTimeHeartbeat);

        if (elapsedTimeHeartbeat >= 10000)
        {
            //printf("elapsedTime (ms): %lu\r\n", elapsedTimeHeartbeat);
            //printf("heartbeat (count): %lu\r\n", heartbeat);
        
            
            previousTimeHeartbeat = get_current_time();
            
            heartbeat++;
        }
        
#endif
        
     
     
#if 1
        // FIXME: should we check to see if we are in the middle of receiving?
        
        // periodically send out a radio transmission for a sort of loopback test
        elapsedTimeSendRadio = get_elapsed_time(previousTimeSendRadio);

        if (elapsedTimeSendRadio >= 20000)
        {
            //disable_capture_interrupt();
            disable_global_interrupts();
            
            // FIXME: avoid magic numbers
            radio_tx_blocking(4, 0);
            
            //enable_capture_interrupt();
            enable_global_interrupts();
            
            previousTimeSendRadio = get_current_time();
        }
        
#endif 
      
        
        

	}
    
}
