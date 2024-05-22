#include <stdlib.h>
#include <string.h>


#include "delay.h"
#include "hal.h"
#include "rcswitch.h"
//#include "timer.h"


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


unsigned long long get_received_value(void)
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

unsigned int* getReceivedRawdata(void)
{
    return timings;
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