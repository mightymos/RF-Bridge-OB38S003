//=========================================================
//=========================================================

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// SFR declarations
//#include <SI_EFM8BB1_Register_Enums.h>
#include <8052.h>

#include "..\inc\globals.h"
#include "..\inc\initdevice.h"
//#include "uart_0.h"
//#include "pca_0.h"
//#include "wdt_0.h"
#include "..\inc\uart.h"
#include "..\inc\rf_handling.h"
#include "..\inc\rf_protocols.h"

// FIXME: try to remove the need for these, or change to global naming convention
//bool gReadUartData = true;

RF_SNIFFING_MODE_T lastSniffingMode;
uint8_t trRepeats;
uint8_t gLength;


// sdccman sec. 3.8.1 indicates isr prototype must appear in the file containing main
extern void uart_isr(void) __interrupt (4);
extern void timer2_isr(void) __interrupt (5);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void _sdcc_external_startup(void)
{

}

void delay_hacky(unsigned int msCount)
{
    unsigned int i,j;
    for(i = 0; i < msCount; i++)
    {
        for(j = 0; j < 1000; j++);
    }
}


// FIXME: this seems to be mixing multiple functions in one, consider changes
//void finish_command(uint8_t command)
//{
//	// send uart command
//	uart_put_command(command);
//
//    // FIXME: do not understand why control of uart is being spread around
//	// enable UART again
//	//gReadUartData = true;
//
//	// restart sniffing in its previous mode
//	//PCA0_DoSniffing(gLastSniffingCommand);
//}


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
                    //InitTimer3_ms(1, 50);
                    //buzzer_on();
                    // wait until timer has finished
                    //WaitTimer3Finished();
                    //buzzer_off();

                    // set desired RF protocol PT2260
                    //gSniffingMode = STANDARD;
                    //gLastSniffingCommand = PCA0_DoSniffing(RF_CODE_LEARN);

                    // start timeout timer
                    //InitTimer3_ms(1, 30000);
                    break;
                // do original sniffing
                case RF_CODE_RFIN:
                    // check if a RF signal got decoded
                    if ((gRFDataStatus & RF_DATA_RECEIVED_MASK) != 0)
                    {
                        uart_put_RF_Data_Standard(RF_CODE_RFIN);

                        // clear RF status
                        gRFDataStatus = 0;

                        // enable interrupt for RF receiving
                        //PCA0CPM0 |= PCA0CPM0_ECCF__ENABLED;
                    }
                    else
                    {
                        // handle new received buckets
                        if (buffer_out(&bucket))
                        {
                            handle_rf_bucket(bucket & 0x7FFF, (bool)((bucket & 0x8000) >> 15));
                        }
                    }
                    break;
                case RF_CODE_RFOUT:
                    // stop sniffing while handling received data
                    //PCA0_StopSniffing();
                    //state = RECEIVING;
                    //trRepeats = RF_TRANSMIT_REPEATS;
                    //position = 0;
                    //gLength = 9;
                    
                    trRepeats--;
                    PCA0_StopSniffing();

                    // byte 0..1:	Tsyn
                    // byte 2..3:	Tlow
                    // byte 4..5:	Thigh
                    // byte 6..7:	24bit Data

                    buckets[0] = *(uint16_t *)&gRFData[2];
                    buckets[1] = *(uint16_t *)&gRFData[4];
                    buckets[2] = *(uint16_t *)&gRFData[0];

                    SendBuckets(buckets,
                            protocols[0].start.dat, protocols[0].start.size,
                            protocols[0].bit0.dat, protocols[0].bit0.size, protocols[0].bit1.dat, protocols[0].bit1.size, protocols[0].end.dat, protocols[0].end.size,
                            protocols[0].bit_count,
                            gRFData + 6
                            );
                    break;
                case RF_DO_BEEP:
                    // FIXME: replace with timer rather than delay(), although appears original code was blocking too
                    buzzer_on();
                    delay_hacky(50);
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
                    gSniffingMode = STANDARD;
                    // re-enable default RF_CODE_RFIN sniffing
                    pca_start_sniffing(RF_CODE_RFIN);
                    gLastSniffingCommand = RF_CODE_RFIN;
                    break;
                case RF_CODE_ACK:
                    // re-enable default RF_CODE_RFIN sniffing
                    //gLastSniffingCommand = PCA0_DoSniffing(gLastSniffingCommand);
                    //state = IDLE;
                    break;
                    
                // wait until data got transfered
                case RF_FINISHED:
                    if (trRepeats == 0)
                    {
                        // disable RF transmit
                        tdata_off();

                        uart_put_command(RF_CODE_ACK);
                    } else {
                        gRFState = RF_IDLE;
                    }
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
            position = 0;
            gLength = rxdata & 0xFF;
            if (gLength > 0)
            {
                // stop sniffing while handling received data
                PCA0_StopSniffing();
                state = RECEIVING;
            }
            else
                state = SYNC_FINISH;
            break;

        // receiving UART data
        case RECEIVING:
            gRFData[position] = rxdata & 0xFF;
            position++;

            if (position == gLength)
            {
                state = SYNC_FINISH;
            }
            else if (position >= RF_DATA_BUFFERSIZE)
            {
                gLength = RF_DATA_BUFFERSIZE;
                state = SYNC_FINISH;
            }
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
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
    // upper eight bits hold error or no data flags
 	unsigned int rxdata = UART_NO_DATA;

	// hardware initialization
	init_port_pins();
    init_uart();

	// set default state
    led_off();
    buzzer_off();
    tdata_off();


    // FIXME: does this contradict state machine where sniffing is advanced?
	// set desired sniffing type to PT2260
	//gSniffingMode = STANDARD;
	//PCA0_DoSniffing(RF_CODE_RFIN);
	//gLastSniffingCommand = RF_CODE_RFIN;

    // FIXME: comment on pca being a comparator?
	pca_stop_sniffing();

	// enable global interrupts
    init_interrupts();


    // FIXME: startup beep helpful or annoying?
    //buzzer_on();
    //delay_hacky(50);
    //buzzer_off();
    
    led_on();
    delay_hacky(500);
    led_off();


	while (true)
	{
		// reset watch dog timer
		//WDT0_feed();


        // try to get one byte from uart rx buffer
		rxdata = uart_getc();
        
        // FIXME: figure out why this allows state machine echoes to work properly
        delay_hacky(10);
        
        // DEBUG: echo back received character
        //delay_hacky(500);
        //if (rxdata != UART_NO_DATA)
        //{
        //    // if buffer is not empty, echo back byte by transmitting
        //    led_toggle();
        //    uart_putc(rxdata);
        //}
        
        // check if transmit buffer is empty
        if(!is_uart_tx_buffer_empty())
        {
            // if not empty, set TI, which triggers interrupt to actually transmit
            uart_init_tx_polling();
        }

        
        if (rxdata != UART_NO_DATA)
        {
            uart_state_machine(rxdata);
        }

	}
    
}
