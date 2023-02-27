//=========================================================
//=========================================================

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// SFR declarations
//#include <SI_EFM8BB1_Register_Enums.h>
#include <8052.h>
#include <stdint.h>
#include <stdio.h>

#include "..\inc\globals.h"
#include "..\inc\initdevice.h"
//#include "..\inc\pca.h"
#include "..\inc\timer.h"
#include "..\inc\uart.h"
//#include "wdt_0.h"

//#include "..\inc\rf_handling.h"
#include "..\inc\rf_protocols.h"

// FIXME: try to remove the need for these, or change to global naming convention
//bool gReadUartData = true;

// units of milliseconds
#define TOLERANCE 100

const static uint16_t timings [] = { 350, 1050, 10850 };

//RF_SNIFFING_MODE_T lastSniffingMode;
//uint8_t trRepeats;
//uint8_t gLength;


// sdccman sec. 3.8.1 indicates isr prototype must appear in the file containing main
extern void uart_isr(void) __interrupt (4);
extern void timer2_isr(void) __interrupt (5);
//extern void pca_isr(void) __interrupt (14);

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
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
    static bool oneShot = false;
    
    // upper eight bits hold error or no data flags
 	unsigned int rxdata = UART_NO_DATA;

	// hardware initialization
	init_port_pins();
    init_uart();
    init_timer2_capture();

	// set default state
    led_off();
    buzzer_off();
    tdata_off();
    
    // DEBUG: disable radio (not working, does not set pin high?)
    //radio_off();
    
    //RDATA = 1;


    // FIXME: does this contradict state machine where sniffing is advanced?
	// set desired sniffing type to PT2260
	//gSniffingMode = STANDARD;
	//PCA0_DoSniffing(RF_CODE_RFIN);
	//gLastSniffingCommand = RF_CODE_RFIN;

    // FIXME: comment on pca being a comparator?
	//pca_stop_sniffing();

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
        //delay_hacky(10);
        
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
        
        if (gCaptureDone)
        {
            //led_toggle();
            
            //uart_putc((gCaptureDiff >> 8) & 0xff);
            //uart_putc(gCaptureDiff & 0xff);
            
            // divide by four to get microseconds
            
            disable_interrupts();
            
            if ( (gDuration[0] > (timings[0] - TOLERANCE_MIN)) && (gDuration[0] < (timings[0] + TOLERANCE_MIN)) )
            {
                led_toggle();

                if ( (gDuration[1] > (timings[2] - TOLERANCE_MAX)) && (gDuration[1] < (timings[2] + TOLERANCE_MAX)) )
                {
                    printf("SYNC:\r\n");
                    printf("%u\r\n", gDuration[0]);
                    printf("%u\r\n", gDuration[1]);
                    printf("\r\n");
                }
            }
            
            enable_interrupts();
            
            gCaptureDone = false;
        }

	}
    
}
