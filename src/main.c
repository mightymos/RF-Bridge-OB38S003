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
#include "..\inc\timer.h"
#include "..\inc\uart.h"
//#include "wdt_0.h"

//#include "..\inc\rf_handling.h"
#include "..\inc\rf_protocols.h"


// DEBUG: bench sensor
//15:12:45.080 MQT: tele/tasmota_4339CA/RESULT = {"Time":"2023-02-27T15:12:45","RfReceived":{"Sync":10250,"Low":340,"High":1010,"Data":"4AF10A","RfKey":"None"}}
//15:12:49.186 MQT: tele/tasmota_4339CA/RESULT = {"Time":"2023-02-27T15:12:49","RfReceived":{"Sync":10250,"Low":340,"High":1000,"Data":"4AF10E","RfKey":"None"}}
#define RADIO_STARTUP_TIME 500
#define REPEAT_TRANSMISSIONS 8

// sdccman sec. 3.8.1 indicates isr prototype must appear in the file containing main
extern void uart_isr(void) __interrupt (4);
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
    static uart_state_t state = IDLE;
    uart_command_t command = NONE;
    
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
                    gSingleStep = true;
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


//
// borrowed from ReedTripRadio
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
            tdata_on();
            delay10us(105);
            
            tdata_off();
            delay10us(35);
        }
        else
        {
            tdata_on();
            delay10us(35);
            
            tdata_off();
            delay10us(105);
        }
        
        toSend = toSend << 1;
    }
}

void send_radio_packet(const unsigned char rfcode)
{
    unsigned char index;
    //unsigned char byteToSend;
    
    
    //enable_radio_vdd();
    delay1ms(RADIO_STARTUP_TIME);
    
    // many receivers require repeatedly sending identical transmissions to accept data
    for (index = 0; index < REPEAT_TRANSMISSIONS; index++)
    {
        // rf sync pulse
        tdata_on();
        delay10us(35);
        
        // this should be the only really long delay required
        tdata_off();
        delay1ms(11);

        // send rf key with unique id and code
        send(0x4a);
        send(0xf1);
        send(rfcode);
    }
    
    //disable_radio_vdd();
    
    // FIXME: we need to force ask low/high just in case correct?
}


//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
    // FIXME: avoid magic numbers
    __xdata bool rfData[24];

    uint8_t index;
    uint8_t bitIndex;
    
    // upper eight bits hold error or no data flags
 	unsigned int rxdata = UART_NO_DATA;

	// hardware initialization
    set_clock_1t_mode();
	init_port_pins();
    init_uart();
    init_timer2_capture();

	// set default state
    led_off();
    buzzer_off();
    tdata_off();
    
    // DEBUG: disable radio (not working, does not set pin high?)
    //radio_off();
    

	// enable interrupts
    init_serial_interrupt();
    init_capture_interrupt();
    enable_global_interrupts();


    // FIXME: startup beep helpful or annoying?
    //buzzer_on();
    //delay1ms(20);
    //buzzer_off();
    
    // startup blink
    led_on();
    delay1ms(500);
    led_off();
    
    printf("Startup...\r\n");

    // DEBUG:
    //while (true)
    //{
    //    send_radio_packet(0x0a);
    //    //uart_tx_pin_toggle();
    //    delay1ms(10000);
    //    //delay10us(35);
    //}



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
        
        // check if transmit buffer is empty
        if(!is_uart_tx_buffer_empty())
        {
            if (gTXFinished)
            {
                // if not empty, set TI, which triggers interrupt to actually transmit
                uart_init_tx_polling();
            }
        }

        
        if (rxdata != UART_NO_DATA)
        {
            uart_state_machine(rxdata);
        }
        
        // DEBUG: 
        // if (gPacket.count >= 2)
        // {
            // printf("count:%u\r\n",       gPacket.count);
            // printf("syncFirst: %u\r\n", gPacket.syncFirstDuration);
            // printf("syncSecond: %u\r\n", gPacket.syncSecondDuration);
            // printf("diff: %lu\r\n", gPacket.diff);
            
            // gPacket.count = 0;
        // }
        
        // similar to rc-switch duration based decoder
        if (gPacket.captureDone)
        {
            //led_toggle();
            
            index = 0;
            bitIndex = 0;
            while (index < TOTAL_RF_DATA_BITS)
            {
                // FIXME: support inverted protocols (or does it matter since it is duration only?)
                if (gPacket.duration[bitIndex] && !gPacket.duration[bitIndex + 1])
                {
                    rfData[index] = 1;
                } else {
                    if (!gPacket.duration[bitIndex] && gPacket.duration[bitIndex + 1])
                    {
                        rfData[index] = 0;
                    } else {
                        // error condition
                        printf("decoding error\r\n");
                    }
                }
                
                index++;
                bitIndex += 2;
            }
            
            index = 0;
            while (index < TOTAL_RF_DATA_BITS)
            {
                printf("%d", rfData[index]);
                index++;
            }
            
            printf("\r\n");
            
            //enable_interrupts();
            //gSyncFirst   = false;
            //gSyncSecond  = false;
            gPacket.captureDone = false;
        }

	}
    
}
