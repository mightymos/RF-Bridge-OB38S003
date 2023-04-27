//=========================================================
//=========================================================

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// SFR declarations
//#include <SI_EFM8BB1_Register_Enums.h>
#include <8052.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// borrowed from area-8051 uni-stc HAL...
#include "..\inc\delay.h"

#include "..\inc\globals.h"
#include "..\inc\initdevice.h"
//#include "..\inc\pca.h"
#include "..\inc\rtos.h"
#include "..\inc\timer.h"
#include "..\inc\uart.h"
//#include "wdt_0.h"

//#include "..\inc\rf_handling.h"
#include "..\inc\rf_protocols.h"


// DEBUG: bench sensor
//15:12:45.080 MQT: tele/tasmota_4339CA/RESULT = {"Time":"2023-02-27T15:12:45","RfReceived":{"Sync":10250,"Low":340,"High":1010,"Data":"4AF10A","RfKey":"None"}}
//15:12:49.186 MQT: tele/tasmota_4339CA/RESULT = {"Time":"2023-02-27T15:12:49","RfReceived":{"Sync":10250,"Low":340,"High":1000,"Data":"4AF10E","RfKey":"None"}}
#define RADIO_STARTUP_TIME 500
#define TX_REPEAT_TRANSMISSIONS 1

// removed SYN470R radio receiver on one of my boards and tied reset pin to rdata input pin for testing purposes
// so basically enabling loopback uses reset pin to apply radio transmission signals, as opposed to tdata pin


// TODO:
typedef enum {
    START_FIRST_LEVEL,
    START_SECOND_LEVEL,
    RF_BITS,
} RADIO_DECODE_STATE_T;

typedef enum {
    STARTUP,
    FIRST_PULSE_LEVEL,
    SECOND_PULSE_LEVEL,
    INITIALIZE_SEND,
    SEND_DATA,
    ONE_FIRST_LEVEL,
    ONE_SECOND_LEVEL,
    ZERO_FIRST_LEVEL,
    ZERO_SECOND_LEVEL,
    FINISHED
} RADIO_TX_STATE_T;

// sdccman sec. 3.8.1 indicates isr prototype must appear in the file containing main
//extern void timer0_isr(void) __interrupt (1);
extern void rtos_timer(void) __interrupt (1);
extern void uart_isr(void)   __interrupt (4);
extern void timer2_isr(void) __interrupt (5);

//-----------------------------------------------------------------------------
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
    
    // track time since we last received any data
    uint16_t idleResetCount = 0;

    // FIXME: this seems to reset state machine if we do not receive data after some time
/*
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
*/
    
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
            //}
            //else
            //    state = SYNC_FINISH;
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
//-----------------------------------------------------------------------------
void radio_decode_state_machine(unsigned long diff)
{
    static RADIO_DECODE_STATE_T currentState = START_FIRST_LEVEL;
    
    static uint8_t bitIndex    = 0;
    static uint8_t periodIndex = 0;
    
    // FIXME: add comment
    static uint16_t firstPeriod;
    static uint16_t secondPeriod;

    // state machine begins by looking for sync pulse and then stores duration of data bits
    switch (currentState)
    {
        // only begin saving durations if expected start pulse timings are observed
        //   as an initial attempt to ignore noise pulses
        case START_FIRST_LEVEL:
            if ( abs(diff - gTimings[0]) < TOLERANCE_MIN )
            {
                gPacket.syncFirstDuration = diff;
                gPacket.syncFirstFlag = true;
                currentState = START_SECOND_LEVEL;
                
                periodIndex = 0;
                bitIndex    = 0;
                
                //led_toggle();
            } else {
                //DEBUG:
                //reset_pin_off();
            }
            break;
            
        case START_SECOND_LEVEL:
            
            if ( abs(diff - gTimings[2]) < TOLERANCE_MAX )
            {
                gPacket.syncSecondDuration = diff;
                gPacket.syncSecondFlag = true;

                currentState = RF_BITS;
            } else {
                currentState = START_FIRST_LEVEL;
            }
            break;
            
        case RF_BITS:
        
            // check if index is even or odd
            if ((periodIndex % 2) == 0)
            {
                //firstPeriod  = gPacket.duration[periodIndex - 1];
                //secondPeriod = gPacket.duration[periodIndex];
                firstPeriod = diff;
            } else {
                secondPeriod = diff;
                
                // FIXME: support inverted protocols (or does it matter since it is duration only?)
                if (abs(firstPeriod - gTimings[1]) < TOLERANCE_MIN && abs(secondPeriod - gTimings[0]) < TOLERANCE_MIN)
                {
                    gPacket.radioBits[bitIndex++] = 1;
                } else {
                    if (abs(firstPeriod - gTimings[0]) < TOLERANCE_MIN && abs(secondPeriod - gTimings[1]) < TOLERANCE_MIN)
                    {
                        gPacket.radioBits[bitIndex++] = 0;
                    } else {
                        // error condition
                        printf("error: %u, %u, %u\r\n", periodIndex);
                        printf("%u\r\n", firstPeriod);
                        printf("%u\r\n", secondPeriod);

                        // reset state to beginning
                        currentState = START_FIRST_LEVEL;
                    }
                }
            }
            
            periodIndex++;

            
            if (periodIndex >= MAX_PERIOD_COUNT)
            {
                //firstPeriod  = gPacket.duration[periodIndex - 1];
                
                if (abs(firstPeriod - gTimings[1]) < TOLERANCE_MIN)
                {
                    gPacket.radioBits[TOTAL_RF_DATA_BITS - 1] = 1;
                } else {
                    if (abs(firstPeriod - gTimings[0]) < TOLERANCE_MIN)
                    {
                        gPacket.radioBits[TOTAL_RF_DATA_BITS - 1] = 0;
                    }
                }
            
                gPacket.captureDone = true;
                
                currentState = START_FIRST_LEVEL;
                
                // FIXME:
                // we may never get another transition if level remains the same
                // e.g. last bit is low level and default signal level is also low
                // so save last level here
                //gPacket.lastLevel = !is_rdata_low();
            }
            
            break;
            
        default:
            break;
    }
}

/*
//
// toggle pins for radio transmission
//  
void send(const unsigned char byte)
{
    const unsigned char numBits = 8;
    
    // mask out highest bit
    const unsigned char mask = 1 << (numBits - 1);
    
    unsigned char i = 0;

    // byte for shifting
    unsigned char toSend = byte;
    
    // Repeat until all bits sent
    for(i = 0; i < numBits; i++)
    {
        // mask out all but left most bit value, and if byte is not equal to zero (i.e. left most bit must be one) then send one level
        if((toSend & mask) > 0)
        {
            
            reset_pin_on();
            //tdata_on();
            delay10us(105);
            
            reset_pin_off();
            //tdata_off();
            delay10us(35);
        }
        else
        {
            reset_pin_on();
            //tdata_on();
            delay10us(35);
            
            reset_pin_off();
            //tdata_off();
            delay10us(105);
        }
        
        toSend = toSend << 1;
    }
}
*/

/*
void send_radio_packet(const unsigned char rfcode)
{
    unsigned char index;
    //unsigned char byteToSend;
    
    
    //enable_radio_vdd();
    delay1ms(RADIO_STARTUP_TIME);
    
    // many receivers require repeatedly sending identical transmissions to accept data
    for (index = 0; index < TX_REPEAT_TRANSMISSIONS; index++)
    {
        // rf sync pulse
        reset_pin_on();
        //tdata_on();
        delay10us(35);
        
        reset_pin_off();
        //tdata_off();
    
        // this should be the only really long delay required
        // FIXME: needs to be programmable
        delay1ms(10);
        delay10us(85);

        // send rf key with unique id and code
        send(0x4a);
        send(0xf1);
        send(rfcode);
    }
    
    //disable_radio_vdd();
    
    // FIXME: we need to force ask low/high just in case correct?
}
*/

/*
void send_1khz_square_wave(void)
{
    unsigned char index;
    
    delay1ms(RADIO_STARTUP_TIME);
    
    for (index = 0; index < 25; index++)
    {
        reset_pin_on();
        //tdata_on();
        delay10us(100);

        reset_pin_off();
        //tdata_off();
        delay10us(100);
    }
}
*/


//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
    // FIXME: avoid magic numbers
    __xdata uint8_t radioBytes[3];
    

    // FIXME: add comment
    uint8_t index;
    uint8_t arrayIndex;
    uint8_t bitIndex;
    
    
    // track elapsed time for doing something periodically (e.g., every 10 seconds)
    unsigned long previousTime = 0;
    unsigned long elapsedTime;
    
    // upper eight bits hold error or no data flags
 	unsigned int rxdata = UART_NO_DATA;
    
    bool doATransmit = false;
    
    _fn fn;
    int ok;
    // init hardware and rtos
    //init_hardware();

	// hardware initialization
    set_clock_1t_mode();
	init_port_pins();
    init_uart();
    init_timer0();
    init_timer2_capture();

    // individual interrupts
    init_capture_interrupt();
    //init_serial_interrupt();


	// set default pin levels
    led_off();
    buzzer_off();
    tdata_off();
    
    // FIXME: need to set startup reset pin state in case it is used?
    //reset_off();
    
    // DEBUG: disable radio (not working, does not set pin high?)
    //radio_off();
    

    // FIXME: startup beep helpful or annoying?
    //buzzer_on();
    //delay1ms(20);
    //buzzer_off();
    
    // startup blink
    led_on();
    delay1ms(500);
    led_off();
    
    printf("Startup...\r\n");

    // init semaphores
    //init(&key_pressed, 0);
    //init(&key_released, 1);
    //init(&flash_req, 0);
    //init(&ext_int,0);	

    // init rtos
    rtos_init();
    
    // add required idle process
    fn = idle;
    // lowest priority
    ok = create_process(fn, 0) >= 0;
    
    //fn = send_radio_packet_task;
    //fn = flash_led;
    //ok |= create_process(fn, 1) >= 0;

	// enable interrupts
    enable_global_interrupts();
    
    if (ok)
    {
        rtos_run();
    }

    while (true);
    return 0;

	while (true)
	{
		// reset watch dog timer
		//WDT0_feed();


        // try to get one byte from uart rx buffer
		rxdata = uart_getc();
        
        // FIXME: figure out why this allows state machine echoes to work properly
        //delay_hacky(10);
        
        // DEBUG: echo back received character
        //delay1ms(500);
        //if (rxdata != UART_NO_DATA)
        //{
        //    // if buffer is not empty, echo back byte by transmitting
        //    led_toggle();
        //    uart_putc(rxdata);
        //}
        
        // check if serial transmit buffer is empty
        if(!is_uart_tx_buffer_empty())
        {
            if (gTXFinished)
            {
                // if not empty, set TI, which triggers interrupt to actually transmit
                uart_init_tx_polling();
            }
        }

        // process serial receive data
        if (rxdata != UART_NO_DATA)
        {
            uart_state_machine(rxdata);
        }
        
        
        // DEBUG:
        // this executes about every 10 seconds
        //elapsedTime = get_elapsed_time(previousTime);

        //if (elapsedTime >= 1000000)
        //{
        //    printf("elapsedTime: %lu\r\n", elapsedTime);
        //    printf("sending radio packet...\r\n");
        //    //printf("heartbeat...\r\n");
        //
        //    //send_radio_packet(0x0f);
        //    //send_1khz_square_wave();
        //    
        //    doATransmit = true;
        //    
        //    previousTime = get_current_time();
        //}
        
        //if (doATransmit)
        //{
        //    doATransmit = send_radio_packet_state_machine();
        //}
        
        if (gPacket.length > 0)
        {
            //printf("debug: radio decode...\r\n");
            led_toggle();
            
            radio_decode_state_machine(gPacket.diff[gPacket.readPosition]);
            
            gPacket.readPosition++;
            
            // handle wrap around
            if (gPacket.readPosition == BUFFER_SIZE)
            {
                gPacket.readPosition = 0;
            }
            
            gPacket.length--;
        }
        
        
        // look for flag set in capture mode timer interrupt
        // similar to rc-switch duration based decoder
        if (gPacket.captureDone)
        {
            //led_toggle();
            disable_capture_interrupt();
                        
            
            // clear array
            index = 0;
            while(index < 3)
            {
                radioBytes[index] = 0;
                index++;
            }
            
            index = 0;
            bitIndex = 0;
            arrayIndex = 0;
            
            // convert bits to byte array
            while (arrayIndex < 3)
            {
                while (index < 8)
                {
                    if (gPacket.radioBits[bitIndex])
                    {
                        radioBytes[arrayIndex] |=  (1 << (7 - index));
                        //printf("%u: %u: %u: %u\r\n", bitIndex, index, arrayIndex, radioBytes[arrayIndex]);
                    }
                    
                    
                    index++;
                    bitIndex++;
                }
                
                index = 0;
                arrayIndex++;
            }
            
            
            //printf("\r\n");
            
            // display bits
            //bitIndex = 0;
            //while (bitIndex < 24)
            //{
            //    printf("%d", radioBits[bitIndex]);
            //    bitIndex++;
            //}

            //printf("\r\n");
            
            // display bytes in hex format
            index = 0;
            while (index < 3)
            {
                printf("0x%x\r\n", radioBytes[index] >> 4);
                printf("0x%x\r\n", radioBytes[index] & 0x0f);
                index++;
            }
            
            
            //enable_interrupts();
            //gSyncFirst   = false;
            //gSyncSecond  = false;
            gPacket.captureDone = false;
            
            enable_capture_interrupt();
        }

	}
    
}
