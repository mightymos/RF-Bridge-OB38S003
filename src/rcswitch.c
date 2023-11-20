#include <stdlib.h>
#include <string.h>


#include "delay.h"
#include "hal.h"
#include "rcswitch.h"
#include "timer.h"


// FIXME: explain constants
volatile __xdata struct RC_SWITCH_T gRCSwitch = {0, 0, 0, 0, 60, 4300};
volatile __xdata uint16_t timings[RCSWITCH_MAX_CHANGES];

static __xdata struct TRANSMIT_PACKET_T gTxPacket;


//
const struct Protocol protocols[] = {
  { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false },    // protocol 1
  { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false },    // protocol 2
  { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false },    // protocol 3
  { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false },    // protocol 4
  { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false },    // protocol 5
  { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 6 (HT6P20B)
  { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false },    // protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
  { 200, {  3, 130}, {  7, 16 }, {  3,  16}, false},     // protocol 8 Conrad RS-200 RX
  { 200, { 130, 7 }, {  16, 7 }, { 16,  3 }, true},      // protocol 9 Conrad RS-200 TX
  { 365, { 18,  1 }, {  3,  1 }, {  1,  3 }, true },     // protocol 10 (1ByOne Doorbell)
  { 270, { 36,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 11 (HT12E)
  { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true }      // protocol 12 (SM5212)
};


// count of number of protocol entries
const unsigned int numProto = sizeof(protocols) / sizeof(protocols[0]);




bool available(void)
{
    return gRCSwitch.nReceivedValue != 0;
}

void reset_available(void)
{
    gRCSwitch.nReceivedValue = 0;
}


unsigned long get_received_value(void)
{
    return gRCSwitch.nReceivedValue;
}

unsigned char get_received_bitlength(void)
{
    return gRCSwitch.nReceivedBitlength;
}

unsigned int get_received_delay(void)
{
    return gRCSwitch.nReceivedDelay;
}

unsigned char get_received_protocol(void)
{
    return gRCSwitch.nReceivedProtocol;
}

int get_received_tolerance(void)
{
    return gRCSwitch.nReceiveTolerance;
}


bool receive_protocol(const int p, unsigned int changeCount)
{
    // FIXME: do we copy to ram from flash so check is faster in loops below?
    struct Protocol pro;

    // FIXME: we should probably check for out of bound index e.g. p = 0
    memcpy(&pro, &protocols[p-1], sizeof(struct Protocol));
    
    unsigned long code = 0;
    
    // assuming the longer pulse length is the pulse captured in timings[0]
    const unsigned int syncLengthInPulses = ((pro.syncFactor.low) > (pro.syncFactor.high)) ? (pro.syncFactor.low) : (pro.syncFactor.high);
    const unsigned int delay = timings[0] / syncLengthInPulses;
    const unsigned int delayTolerance = delay * get_received_tolerance() / 100;
    
    
    /* For protocols that start low, the sync period looks like
     *               _________
     * _____________|         |XXXXXXXXXXXX|
     *
     * |--1st dur--|-2nd dur-|-Start data-|
     *
     * The 3rd saved duration starts the data.
     *
     * For protocols that start high, the sync period looks like
     *
     *  ______________
     * |              |____________|XXXXXXXXXXXXX|
     *
     * |-filtered out-|--1st dur--|--Start data--|
     *
     * The 2nd saved duration starts the data
     */
    const unsigned int firstDataTiming = (pro.invertedSignal) ? (2) : (1);

    for (unsigned int i = firstDataTiming; i < changeCount - 1; i += 2)
    {
        code <<= 1;
        
        if (abs(timings[i] - delay * pro.zero.high) < delayTolerance &&
            abs(timings[i + 1] - delay * pro.zero.low) < delayTolerance) {
            // zero
        } else if (abs(timings[i] - delay * pro.one.high) < delayTolerance &&
            abs(timings[i + 1] - delay * pro.one.low) < delayTolerance) {
            // one
            code |= 1;
        } else {
            // failed
            return false;
        }
    }
    
    // ignore very short transmissions: no device sends them, so this must be noise
    if (changeCount > 7)
    {
        gRCSwitch.nReceivedValue = code;
        gRCSwitch.nReceivedBitlength = (changeCount - 1) / 2;
        gRCSwitch.nReceivedDelay = delay;
        gRCSwitch.nReceivedProtocol = p;
        
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
// transmits or simulates transmitting a radio packet by toggling the reset pin (which we bridge to radio receive pin)
// use timings from ReedTripRadio project
// use blocking send (instead of task style) because we disable receiving during sending anyway to avoid self feedback
// ----------------------------------------------------------------------------
bool radio_tx_blocking(const uint8_t totalRepeats, const int ident)
{
    unsigned int pulseLength;
    
    const int id = ident - 1;
    
    // FIXME: it seems like possibly if the first bit is not one, transmission is not received
    // 0x4a, 0xf1, 0x08
    //__code const bool bitLevels[] = {0,1,0,0, 1,0,1,0, 1,1,1,1, 0,0,0,1, 0,0,0,0, 1,0,0,0};
    //__code const bool bitLevels[] = {0,1,0,1, 1,0,1,0, 0,1,0,1, 1,0,1,0, 0,1,0,1, 1,0,1,0};
    __code const bool bitLevels[] = {1,0,1,0, 0,1,0,1, 1,0,1,0, 0,1,0,1, 1,0,1,0, 1,0,1,0};
    
    
    //05:49:32.304 RSL: RESULT = {"Time":"2023-08-17T05:49:32","RfReceived":{"Sync":20428,"Low":644,"High":1952,"Data":"80650A","RfKey":"None"}}
    //05:49:38.903 RSL: RESULT = {"Time":"2023-08-17T05:49:38","RfReceived":{"Sync":20474,"Low":644,"High":1952,"Data":"80650E","RfKey":"None"}}
    //__code const bool bitLevels[] = {1,0,0,0, 0,0,0,0, 0,1,1,0, 0,1,0,1, 0,0,0,0, 1,1,1,0};
    
    // time in milliseconds to delay between transmissions
    const unsigned int repetitionDelay = 30;

    // since sync pulse is two bits, we need 24 more for data bits
    const uint8_t indexEnd = 24;
    
    
    // FIXME: avoid indexing outside of array
    //if (ident >= numProto)
    //{
    //    return 0;
    //}
    
    // 10 microsecond increments (e.g., 350 microseconds delay is invoked with delay10us(35) below)
    pulseLength = protocols[id].pulseLength / 10;
    
    // FIXME: probably need to split up really long pulse times between calls to delay1ms() followed by delay10us()
    //        because too many loops using only delay10us() surely accumulates error
    gTxPacket.syncHigh = pulseLength * protocols[id].syncFactor.high;
    gTxPacket.syncLow  = pulseLength * protocols[id].syncFactor.low;
    
    gTxPacket.zeroHigh  = pulseLength * protocols[id].zero.high;
    gTxPacket.zeroLow   = pulseLength * protocols[id].zero.low;
    gTxPacket.oneHigh   = pulseLength * protocols[id].one.high;
    gTxPacket.oneLow    = pulseLength * protocols[id].one.low;
    
    // store millisecond and microsecond portions of delays separately
    gTxPacket.syncHighMS = (pulseLength * protocols[id].syncFactor.high) / 100;
    gTxPacket.syncHighUS = (pulseLength * protocols[id].syncFactor.high) % 100;
    
    gTxPacket.syncLowMS = (pulseLength * protocols[id].syncFactor.low) / 100;
    gTxPacket.syncLowUS = (pulseLength * protocols[id].syncFactor.low) % 100;
    
    gTxPacket.zeroHighMS = (pulseLength * protocols[id].zero.high) / 100;
    gTxPacket.zeroHighUS = (pulseLength * protocols[id].zero.high) % 100;
    
    gTxPacket.zeroLowMS = (pulseLength * protocols[id].zero.low) / 100;
    gTxPacket.zeroLowUS = (pulseLength * protocols[id].zero.low) % 100;
    
    gTxPacket.oneHighMS = (pulseLength * protocols[id].one.high) / 100;
    gTxPacket.oneHighUS = (pulseLength * protocols[id].one.high) % 100;
    
    gTxPacket.oneLowMS = (pulseLength * protocols[id].one.low) / 100;
    gTxPacket.oneLowUS = (pulseLength * protocols[id].one.low) % 100;
    
    // bit index
    uint8_t index;
    // track packet repetition
    uint8_t repeatIndex;    
    
    // FIXME: do not know if the transmit IC is constantly powered or not
    //enable_radio_vdd();
    //delay(RADIO_STARTUP_TIME);
    //delay(500);
    
    
    // FIXME: we need to handle inverted protocols
    for (repeatIndex = 0; repeatIndex < totalRepeats; repeatIndex++)
    {
    
        // sync pulse
        // allow attaching oscilloscope probe to a convenient location (reset) and also actually driving radio transmit pin
        //reset_pin_on();
        tdata_on();
        

        delay1ms(gTxPacket.syncHighMS);
        delay10us(gTxPacket.syncHighUS);

        
        // sync pulse
        //reset_pin_off();
        tdata_off();
        
        
        delay1ms(gTxPacket.syncLowMS);
        delay10us(gTxPacket.syncLowUS);


        // cycle through all the bits
        for (index = 0; index < indexEnd; index++)
        {
            
            // bit is one
            if (bitLevels[index])
            {
                //reset_pin_on();
                tdata_on();
                

                delay1ms(gTxPacket.oneHighMS);
                delay10us(gTxPacket.oneHighUS);


                //reset_pin_off();
                tdata_off();
                

                delay1ms(gTxPacket.oneLowMS);
                delay10us(gTxPacket.oneLowUS);

            } else {

                //reset_pin_on();
                tdata_on();
                
                delay1ms(gTxPacket.zeroHighMS);
                delay10us(gTxPacket.zeroHighUS);


                //reset_pin_off();
                tdata_off();
                
                delay1ms(gTxPacket.zeroLowMS);
                delay10us(gTxPacket.zeroLowUS);

            }
            
        }

        // transmission gap
        //reset_pin_off();
        tdata_off();
        
        // 30 millisecond gap between repeat transmissions
        // this was essentially just picked in ReedTripRadio but reason is not remembered...
        delay1ms(repetitionDelay);
        


    }
    
    // we are always finished because this is blocking (different than task version of the same function)
    return false;
}