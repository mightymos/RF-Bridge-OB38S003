#include <stdlib.h>


#include "rcswitch.h"

// we are basically following the same decoding approach used by the rc-switch project
// https://github.com/sui77/rc-switch
// https://github.com/1technophile/rc-switch

volatile __xdata struct RC_SWITCH_T gRCSwitch = {0, 0, 0, 0, 60, 4300};
volatile __xdata uint16_t timings[RCSWITCH_MAX_CHANGES];



// rc-switch protocol 1
//const uint16_t gTimings [] = { 350, 1050, 10850 };
//const uint16_t gTimings [] = { 1000, 1000, 1000 };


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




bool available()
{
    return gRCSwitch.nReceivedValue != 0;
}

void resetAvailable()
{
    gRCSwitch.nReceivedValue = 0;
}


unsigned long getReceivedValue()
{
    return gRCSwitch.nReceivedValue;
}

unsigned int getReceivedBitlength()
{
    return gRCSwitch.nReceivedBitlength;
}

unsigned int getReceivedDelay()
{
    return gRCSwitch.nReceivedDelay;
}

unsigned int getReceivedProtocol()
{
    return gRCSwitch.nReceivedProtocol;
}


bool receiveProtocol(const int p, unsigned int changeCount)
{

//    Protocol pro;
//    memcpy_P(&pro, &proto[p-1], sizeof(Protocol));

    struct Protocol* pro = protocols;
    
    // the first procotol is considered p=1 but is at index 0
    unsigned int index = p - 1;
    
    unsigned long code = 0;
    
    // assuming the longer pulse length is the pulse captured in timings[0]
    const unsigned int syncLengthInPulses = ((pro[index].syncFactor.low) > (pro[index].syncFactor.high)) ? (pro[index].syncFactor.low) : (pro[index].syncFactor.high);    
    const unsigned int delay = timings[0] / syncLengthInPulses;
    const unsigned int delayTolerance = delay * gRCSwitch.nReceiveTolerance / 100;
    
    
    
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
    const unsigned int firstDataTiming = (pro[index].invertedSignal) ? (2) : (1);

    for (unsigned int i = firstDataTiming; i < changeCount - 1; i += 2)
    {
        code <<= 1;
        
        if (abs(timings[i] - delay * pro[index].zero.high) < delayTolerance &&
            abs(timings[i + 1] - delay * pro[index].zero.low) < delayTolerance) {
            // zero
        } else if (abs(timings[i] - delay * pro[index].one.high) < delayTolerance &&
            abs(timings[i + 1] - delay * pro[index].one.low) < delayTolerance) {
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