#include <limits.h>
#include <stdlib.h>
#include <string.h>


#include "delay.h"
#include "hal.h"
#include "rcswitch.h"

#include "uart_software.h"

#include "timer_interrupts.h"

//#include "ticks.h"


// FIXME: explain constants
volatile __xdata struct RC_SWITCH_T gRCSwitch = {0, 0, 0, 0, 60, 4300};
volatile __xdata uint16_t timings[RCSWITCH_MAX_CHANGES];


const uint8_t nRepeatTransmit = 8;

// pulse length is units of 10 microseconds, modified from the microsecond version because it is unreasonable to generate microsecond timing on these microcontrollers
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

uint16_t* getReceivedRawdata(void)
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
        gRCSwitch.nReceivedValue     = code;
        gRCSwitch.nReceivedBitlength = (changeCount - 1) / 2;
        gRCSwitch.nReceivedDelay     = delay;
        gRCSwitch.nReceivedProtocol  = p;
        
        return true;
    }

    return false;
}

void capture_handler(const uint16_t currentCapture)
{
    const uint8_t gapMagicNumber  = 200;
    const uint8_t repeatThreshold   = 2;
    
    // for converting 8-bit timer values to 16-bits to allow subtraction
    uint16_t        previous;
    static uint16_t current = 0;
    
    // this eventually represents the level duration in microseconds (difference between edge transitions)
    // FIXME: we should probably comment on why this is long type once I remember
    unsigned long duration;
    

    // rc-switch variables
    // changed from int to uint8_t to save memory
    static uint8_t repeatCount = 0;
    static uint8_t changeCount = 0;


    const unsigned int separationLimit = gRCSwitch.nSeparationLimit;

    // FIXME: rcswitch library originally relied on computing time difference
    //        but I do not think they handled the cause when timer overflowed
    //        an alternative is to just reset counter to zero each edge detection
    //previous = current;
    //current = currentCapture;
    //duration = current - previous;
    
    // e.g., EFM8BB1
    // e.g. (1/(24500000))*(49/2) = 1      microsec
    // e.g. (1/(24500000/12))*2   = 0.9796 microsec
    // so need to do the inverse to go from counts to time (i.e., counts * 1/2 = time)
    // (1/(24500000/12))*dec(0xFFFF) = 32.0987755 millisecs max
    
    // e.g., OBS38S003
    // e.g. prescale at (1/4) at 16 MHz, four counts are needed to get one microsecond
    // e.g. prescale at (1/24) at 16 MHz, 2/3 counts are need to get one microsecond
    // so inverse is counts * 3/2 = time
    // e.g., (1/(16000000/24)) * dec(0xFFFF) = 98.30 milliseconds maximum can be counted
    //duration = countsToTime(duration);
    duration = countsToTime(currentCapture);
    
    // reset counter on each edge detection so we avoid need to computer time difference
    // and hopefully avoid situation where counter overflows and wraps around
    clear_pca_counter();
    
    // from oscillscope readings it appears that first sync pulse of first radio packet is frequently not output properly by receiver
    // this could be because radio receiver needs to "warm up" (despite already being enabled?)
    // and it is known that radio packet transmissions are often repeated (between about four and twenty times) perhaps in part for this reason
    if (duration > separationLimit)
    {
        // A long stretch without signal level change occurred. This could
        // be the gap between two transmission.
        if (abs(duration - timings[0]) < gapMagicNumber)
        {
          // This long signal is close in length to the long signal which
          // started the previously recorded timings; this suggests that
          // it may indeed by a a gap between two transmissions (we assume
          // here that a sender will send the signal multiple times,
          // with roughly the same gap between them).
          repeatCount++;
          
          if (repeatCount == repeatThreshold)
          {
            for(unsigned int i = 1; i <= numProto; i++)
            {
              if (receive_protocol(i, changeCount))
              {
                // receive succeeded for protocol i
                break;
              }
            }
            
            repeatCount = 0;
          }
        }
        
        changeCount = 0;
    }

    // detect overflow
    if (changeCount >= RCSWITCH_MAX_CHANGES)
    {
        changeCount = 0;
        repeatCount = 0;
    }

    timings[changeCount++] = duration;
    
    // DEBUG: this is a decent way of seeing the actual timing on an oscilloscope
    //debug_pin01_toggle();
}

/**
 * Sets Repeat Transmits
 */
//void setRepeatTransmit(const int repeat)
//{
//  nRepeatTransmit = repeat;
//}

/**
  * Sets the protocol to send.
  */
//void setProtocol(const struct Protocol pro)
//{
//  protocol = pro;
//}

/**
  * Sets the protocol to send, from a list of predefined protocols
  */
//void setProtocol(const int nProtocol)
//{
//    if (nProtocol < 1 || nProtocol > numProto)
//    {
//        // TODO: trigger an error, e.g. "bad protocol" ?
//        nProtocol = 1;
//    }
//
//  memcpy(&protocol, &protocols[nProtocol-1], sizeof(struct Protocol));
//}

/**
 * Transmit a single high-low pulse.
 */
void transmit(const bool invertedSignal, uint16_t delayHigh, uint16_t delayLow)
{
    __xdata uint8_t firstLogicLevel  = invertedSignal ? 0 : 1;
    __xdata uint8_t secondLogicLevel = invertedSignal ? 1 : 0;

    //
    set_tdata(firstLogicLevel);


    init_delay_timer_us(1, delayHigh);
    wait_delay_timer_finished();


    //
    set_tdata(secondLogicLevel);


    init_delay_timer_us(1, delayLow);
    wait_delay_timer_finished();
}


/**
 * Transmit the first 'length' bits of the integer 'code'. The
 * bits are sent from MSB to LSB, i.e., first the bit at position length-1,
 * then the bit at position length-2, and so on, till finally the bit at position 0.
 * e.g., for Tasmota:
 * with timings (sync, low, high)
 * (timings identical to protocol 1)
 * RfRaw AA A5 2A 62 04 1A 01 5E D0 03 58 55
 * by protocol (protocol 1)
 * RfRaw AA A8 04 01 D0 03 58 55
 */
//void sendByProtocol(const int nProtocol, const unsigned int length)
void send(struct Pulse* pulses, unsigned char* packetStart, const unsigned char bitsInPacket)
{
    // this allows us to send an abitrary amount of bits from a byte array
    uint8_t bitIndex;
    uint8_t currentBit;
    uint8_t currentByte;
    
    // track packet repeat count
    uint8_t nRepeat;
    
    // pointer to byte intended to be sent currently
    unsigned char* packetPtr;



    // must repeat sent packet so that receiver interprets it as valid
    for (nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++)
    {
        currentBit = 0;
        
        // reset to first byte of data
        packetPtr = packetStart;
        
        // make a copy of current byte in order to shift that copy
        currentByte = *packetPtr;
        
        // moved sync pulse sending here to match manchester encoding style shown in application notes
        transmit(pulses->invertedSignal, pulses->syncHigh, pulses->syncLow);
        
        // we must send repeat transmission for decoder to accept radio packet
        for (bitIndex = 0; bitIndex < bitsInPacket; bitIndex++)
        {
            
            if (currentBit == 8)
            {
                // FIXME:
                packetPtr++;
                
                // FIXME: why is this done, looking for wrap around?
                //if(packetPtr == &packet[0]){bitIndex = bitsInPacket + 1;}
                
                currentBit = 0;
                currentByte = *packetPtr;
            }
            
            // mask out all but left most bit value, and if byte is not equal to zero (i.e. left most bit must be one) then send one level
            if ((currentByte & 0x80) == 0x80)
            {
                transmit(pulses->invertedSignal, pulses->oneHigh, pulses->oneLow);
            }
            else
            {
                transmit(pulses->invertedSignal, pulses->zeroHigh, pulses->zeroLow);
            }
            
            //
            currentByte = currentByte << 1;
            
            //
            currentBit++;
        }
        
        // FIXME: sync is actually supposed to be transmitted before data
        //        even if rcswitch ignores the first sync pulse and just looks for gaps (the sync) between repeat transmissions
        //transmit(pulses->invertedSignal, pulses->syncHigh, pulses->syncLow);
    }

    // disable transmit after sending (i.e., for inverted protocols)
    tdata_off();

    // we do this outside of the function
    //radio_receiver_on();
}