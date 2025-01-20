#include "delay.h"
#include "hal.h"

// we use the same format as portisch so okay to include
#include "portisch_command_format.h"

#include "rcswitch.h"
#include "state_machine.h"
#include "uart.h"

#include <stdio.h>
#include <stdint.h>

//#include "uart_software.h"

// using commands documented here:
// https://github.com/Portisch/RF-Bridge-EFM8BB1/wiki

// we try to borrow portisch uart protocol but in some cases it does not translate exactly
// standard decoding 0xA4 is 12 bytes always
// standard transmit 0xA5 is 12 bytes always
// advanced sniffing 0xA6 is unsupported
// (rcswitch is essentially doing a form of this anyway with 0xA4 because we compare measured timings to a table to look for match)
// advanced transmit 0xA8 can take a variable data length
// with sync, command, length, protocol index, and end, we will choose to support eight data bytes (64 bits) for a total of 13 bytes
#define PACKET_MAX_SIZE  13


// we place this in internal ram so that a larger external ram buffer can be allocated in uart.c for use in portisch
uint8_t uartPacket[PACKET_MAX_SIZE];

// includes protocol ID and actual radio data
uint8_t gLengthExpected = 0;
    

//-----------------------------------------------------------------------------
// Portisch generally used this type of state machine
//-----------------------------------------------------------------------------
RF_COMMAND_T uart_state_machine(const unsigned int rxdataWithFlags)
{
    // FIXME: eventually an appropriate initialization value might be RF_RFIN for decoding by default
    // but right now we decode outside of state machine
    __xdata static uart_state_t     state = IDLE;
    __xdata static uart_command_t command = NONE;

    // FIXME: need to know what initialization value is appropriate
    __xdata static uint8_t position = 0;
    
    // track count of entries into function
    static uint16_t idleResetCount = 0;
    
    
    // return this value when we need the radio state machine to do something
    __xdata RF_COMMAND_T rfCommand = NO_COMMAND;
    
    
    // strip uart flags to leave only potential data remaining
    // so that code is more readable below (do not need to keep remembering to AND with 0xff)
    const uint8_t rxdata = rxdataWithFlags & 0xFF;  
    
    // used with beep
    uint16_t delay;
    
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
                //putstring("idleReset\r\n");
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
                    //puthex2(rxdata);
                }
                
                break;

            // sync byte got received, so next read command
            case SYNC_INIT:
            
                // FIXME: add comment
                command = rxdata;
                
                // DEBUG
                //puthex2(command);

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
                        gLengthExpected = 9;
                        state = RECEIVING;
                        
                        break;
                    case RF_CODE_RFOUT_NEW:
                        position = 0;
                        // this command has a variable length which is provided by the sender
                        state = RECEIVE_LENGTH;
                        break;
                    case RF_DO_BEEP:
                        position = 0;
                        gLengthExpected = 2;
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
            case RECEIVE_LENGTH:

                gLengthExpected = rxdata & 0xFF;
                
                if (gLengthExpected > 0)
                {
                    state = RECEIVING;
                    
                } else {
                    state = SYNC_FINISH;
                }
                
                break;

            // receiving UART data
            case RECEIVING:
                // actual data
                uartPacket[position] = rxdata;
                position++;

                // DEBUG:
                //puthex2(rxdata);
                
                // look for expected end of packet and also avoid buffer overflow
                if (position == gLengthExpected)
                {
                    state = SYNC_FINISH;
                }
                else if (position >= PACKET_MAX_SIZE)
                {
                    // FIXME: review this logic
                    gLengthExpected = PACKET_MAX_SIZE;
                    state = SYNC_FINISH;
                }
                break;

            // wait and check for UART_SYNC_END
            case SYNC_FINISH:

                if (rxdata == RF_CODE_STOP)
                {
                    // DEBUG:
                    //puthex2(rxdata);
                    //putc('\r');
                    //putc('\n');
                    
                    // indicate this round of receiving uart packet is finished
                    state = IDLE;

                    // the last command received should still be saved from before we received any additional data bytes
                    switch(command)
                    {
                        case RF_CODE_RFOUT:
                            rfCommand = RF_RFOUT_START;
                            break;
                        case RF_CODE_RFOUT_NEW:
                            rfCommand = RF_RFOUT_NEW_START;
                            break;
                        case RF_DO_BEEP:
                        
                            // remember SDCC is little ending for 8051 while data is send as big endian
                            //delay = *(uint16_t *)&uartPacket[0];
                            delay = (uartPacket[0] << 8) | uartPacket[1];
                            
                            
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
    // persists between function calls
    __xdata static RF_STATE_T state = RF_IDLE;
    
    
    struct Protocol* protocolPtr;
    __xdata struct Pulse pulses;
    
    // only used when timings are provided
    uint16_t timing;

    
    // this should be occuring after an entire uart packet is received (i.e., after SYNC_FINISH)
    switch(state)
    {
        case RF_IDLE:
            switch(command)
            {
                case RF_RFOUT_START:
                    state = RF_TRANSMIT_BY_TIMING;
                    break;
                case RF_RFOUT_NEW_START:
                    state = RF_TRANSMIT_BY_PROTOCOL;
                    break;
            }

            break;

        case RF_TRANSMIT_BY_TIMING:
                
            disable_capture_interrupt();

            // DEBUG:
            //putstring("timing\r\n");

    
            // user provided pulse timings
            // sync
            timing = uartPacket[0] << 8 | uartPacket[1];
            pulses.syncLow  = timing;
            
            // FIXME: I think this low and high naming convention on the wiki is misleading
            // FIXME: and check into the convention on reedtripradio as well
            // low
            timing  = uartPacket[2] << 8 | uartPacket[3];
            pulses.syncHigh = timing;
            pulses.zeroHigh = timing;
            pulses.oneLow   = timing;
            
            // high
            timing = uartPacket[4] << 8 | uartPacket[5];
            pulses.zeroLow  = timing;
            pulses.oneHigh  = timing;

            
            // divide by ten to convert from microseconds to tens of microseconds
            pulses.oneHigh  = pulses.oneHigh  / 10;
            pulses.oneLow   = pulses.oneLow   / 10;
            pulses.zeroHigh = pulses.zeroHigh / 10;
            pulses.zeroLow  = pulses.zeroLow  / 10;
            pulses.syncHigh = pulses.syncHigh / 10;
            pulses.syncLow  = pulses.syncLow  / 10;
            
            // 
            pulses.invertedSignal = false;

            //
            send(&pulses, &uartPacket[6], 24);
            
            // DEBUG:
            //putstring("end\r\n");
            
            enable_capture_interrupt();
            
            state = RF_FINISHED;
            

            break;
            
        case RF_TRANSMIT_BY_PROTOCOL:
            // 
            disable_capture_interrupt();
            
            // DEBUG:
            //putstring("protocol\r\n");
    

            // bytes 0..1:  Tsyn
            // bytes 2..3:  Tlow
            // bytes 4..5:  Thigh
            // bytes 6..8:  24bit Data
            // FIXME: in portisch array index equal to protocol number, so 0x00 is the first protocol
            // however, receive_protocol() in rcswitch uses 0x00 to be protocol 1
            // so we may need to make these consistent
            protocolPtr = &protocols[uartPacket[0]];
            
            // calculate timing pulses in microseconds
            pulses.oneHigh  = protocolPtr->pulseLength * protocolPtr->one.high;
            pulses.oneLow   = protocolPtr->pulseLength * protocolPtr->one.low;
            pulses.zeroHigh = protocolPtr->pulseLength * protocolPtr->zero.high;
            pulses.zeroLow  = protocolPtr->pulseLength * protocolPtr->zero.low;
            pulses.syncHigh = protocolPtr->pulseLength * protocolPtr->syncFactor.high;
            pulses.syncLow  = protocolPtr->pulseLength * protocolPtr->syncFactor.low;
            
            // divide by ten to convert from microseconds to tens of microseconds
            pulses.oneHigh  = pulses.oneHigh  / 10;
            pulses.oneLow   = pulses.oneLow   / 10;
            pulses.zeroHigh = pulses.zeroHigh / 10;
            pulses.zeroLow  = pulses.zeroLow  / 10;
            pulses.syncHigh = pulses.syncHigh / 10;
            pulses.syncLow  = pulses.syncLow  / 10;
            
            
            pulses.invertedSignal = protocolPtr->invertedSignal;

            // use a known protocol for transmitting
            send(&pulses, &uartPacket[1], (gLengthExpected - 1) * 8);
            
            // DEBUG:
            //putstring("end\r\n");
            
            enable_capture_interrupt();
            
            state = RF_FINISHED;
        
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
    uint16_t buckets[3];
    
    // assumes standard PT226x timings
    // FIXME: how to handle otherwise?
    // FIXME: handle inverted signal?
    // FIXME: not sure if we should compute an average or something for repeats?
    buckets[0] = timings[0];
    buckets[1] = timings[0] / 31;
    buckets[2] = buckets[1] * 3;

                    
    // packet start sequence
    uart_putc(RF_CODE_START);
    // command
    uart_putc(RF_CODE_RFIN);
    
    // sync, low, high timings are the portisch 0xA4 order convention
    uart_putc((buckets[0] >> 8) & 0xFF);
    uart_putc(buckets[0] & 0xFF);


    uart_putc((buckets[1] >> 8) & 0xFF);
    uart_putc( buckets[1] & 0xFF);
    uart_putc((buckets[2] >> 8) & 0xFF);
    uart_putc( buckets[2] & 0xFF);
    
    // data
    // FIXME: strange that shifting by ZERO works but omitting the shift does not
    uart_putc((get_received_value() >> 16) & 0xFF);
    uart_putc((get_received_value() >>  8) & 0xFF);
    uart_putc((get_received_value() >>  0) & 0xFF);
    
    // packet stop
    uart_putc(RF_CODE_STOP);
}


#if 0

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

#if 0

// FIXME: adapt to send out over software uart or for sniffing mode
void radio_decode_timings(void)
{
    unsigned int index;
    
    for (index = 0; index < get_received_bitlength() * 2; index++)
    {
        // packet start sequence
        uart_putc(RF_CODE_START);
        uart_putc(0xAF);
        
        // sync, low, high timings
        uart_putc((timings[index] >> 8) & 0xFF);
        uart_putc(timings[index] & 0xFF);
    }
    
    uart_putc(RF_CODE_STOP);
}

#endif