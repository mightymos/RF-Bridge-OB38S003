#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "globals.h"
#include "rf_protocols.h"
#include "timer.h"

// store attempts to decode radio bits
__xdata struct RADIO_PACKET_T gPacket;

__xdata struct RC_SWITCH_T gRCSwitch;


// track count of timer 1 which is decremented, and finally flag reaching zero
uint16_t gTimerOneCount = 0;
bool     gIsTimerOneFinished = true;

// track time since startup in one millisecond increments
static unsigned long gTimeMilliseconds = 0;


// rc-switch protocol 1
const uint16_t gTimings [] = { 350, 1050, 10850 };
//const uint16_t gTimings [] = { 1000, 1000, 1000 };

#define RCSWITCH_MAX_CHANGES 67

uint16_t timings[RCSWITCH_MAX_CHANGES];



enum PULSE_DETECT_T {
    IDLE,
    FIRST_PULSE_LEVEL,
    SECOND_PULSE_LEVEL,
    DATA_BITS
};


unsigned long get_current_time(void)
{
    unsigned long currentTime;
    
    // FIXME: disable timer0 interrupt for atomic reading of variable
    //        consider check to see if interrupt was enabled in the first place
    ET0 = 0;
    
    // FIXME: compute the proper conversion from counts to milliseconds
    currentTime = gTimeMilliseconds;
    
    // re-enable timer0 interrupt
    ET0 = 1;
    
    return currentTime;
}


unsigned long get_elapsed_time(unsigned long previousTime)
{
    unsigned long currentTime;
    unsigned long elapsed;
    
    currentTime = get_current_time();
    
    //printf("currentTime: %lu\r\n", currentTime);
    
    // handle typical versus wraparound condition
    if (previousTime <= currentTime)
    {
        elapsed = currentTime - previousTime;
    } else {
        elapsed = ULONG_MAX - previousTime + currentTime;
    }
    
    return elapsed;
}

void write_diff(unsigned long diff)
{
    // FIXME: add comment
    gPacket.diff[gPacket.writePosition] = diff;

    // FIXME: add comment
    gPacket.writePosition++;

    // handle wrap around
    if (gPacket.writePosition == BUFFER_SIZE)
    {
        gPacket.writePosition = 0;
    }        

    gPacket.length++;
}

// FIXME: change function name because this is not directly reloading timer
//void reload_timer1(unsigned int reload)
//{
//    gIsTimerOneFinished = false;
//    gTimerOneCount = reload;
//}

void reload_timer1(unsigned char high, unsigned char low)
{
    gIsTimerOneFinished = false;
    
    TH1 = high;
    TL1 = low;
}

bool receiveProtocol(const int p, unsigned int changeCount) {

//    Protocol pro;
//    memcpy_P(&pro, &proto[p-1], sizeof(Protocol));

    const int nReceiveTolerance = 60;
    const bool invertedSignal = false;
    
    unsigned long code = 0;
    
    // assuming the longer pulse length is the pulse captured in timings[0]
    //const unsigned int syncLengthInPulses =  ((pro.syncFactor.low) > (pro.syncFactor.high)) ? (pro.syncFactor.low) : (pro.syncFactor.high);
    const unsigned int syncLengthInPulses = 31;
    
    //const unsigned int delay = RCSwitch::timings[0] / syncLengthInPulses;
    const unsigned int delay = 350;
    
    //const unsigned int delayTolerance = delay * RCSwitch::nReceiveTolerance / 100;
    const unsigned int delayTolerance = delay * nReceiveTolerance / 100;
    
    const unsigned int zeroHigh = 1;
    const unsigned int zeroLow  = 3;
    const unsigned int oneHigh  = 3;
    const unsigned int oneLow   = 1;
    
    
    
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
    const unsigned int firstDataTiming = (invertedSignal) ? (2) : (1);

    for (unsigned int i = firstDataTiming; i < changeCount - 1; i += 2)
    {
        code <<= 1;
        
        //if (diff(RCSwitch::timings[i], delay * pro.zero.high) < delayTolerance &&
        //    diff(RCSwitch::timings[i + 1], delay * pro.zero.low) < delayTolerance) {
        //    // zero
        //} else if (diff(RCSwitch::timings[i], delay * pro.one.high) < delayTolerance &&
        //           diff(RCSwitch::timings[i + 1], delay * pro.one.low) < delayTolerance) {
        //    // one
        //    code |= 1;
        //} else {
        //    // Failed
        //    return false;
        //}
        
        if (abs(timings[i] - delay * zeroHigh) < delayTolerance &&
            abs(timings[i + 1] - delay * zeroLow) < delayTolerance) {
            // zero
        } else if (abs(timings[i] - delay * oneHigh) < delayTolerance &&
            abs(timings[i + 1] - delay * oneLow) < delayTolerance) {
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
        //gRCSwitch.nReceivedProtocol = p;
        gRCSwitch.nReceivedProtocol = 1;
        
        return true;
    }

    return false;
}

void timer0_isr(void) __interrupt (1)
{
    gTimeMilliseconds++;

    // one millisecond to overflow
    TH0 = 0xc1;
    TL0 = 0x7f;
    
    // ten microseconds to overflow
    //TH0 = 0xff;
    //TL0 = 0x5f;
}


// timer 1 interrupt
void timer1_isr(void) __interrupt (3)
{
    // one shot type count essentially used to implement non-blocking delays
    //if (gTimerOneCount > 0)
    //{
    //    gTimerOneCount--;
    //    
    //    if (gTimerOneCount == 0)
    //    {
    //        gIsTimerOneFinished = true;
    //    }
    //}
    
    gIsTimerOneFinished = true;
    
    // tracks time since timer enabled, used to track long periods of time
    //gTimerOneElapsed++;
    
    // ten microseconds to overflow
    //TH1 = 0xff;
    //TL1 = 0x5f;
}

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (5)
{
    //uint8_t lowByteSaved;
    //uint8_t highByteSaved;
    uint8_t lowByteOld;
    uint8_t highByteOld;
    static uint8_t lowByteNew  = 0;
    static uint8_t highByteNew = 0;
    
    
    bool        levelOld;
    static bool levelNew = false;
    
    static uint8_t dataCount;
    
    const uint8_t maxDataCount = 24;
    
    static unsigned int repeatCount = 0;
    static unsigned int changeCount = 0;
    
    const unsigned int separationLimit = 4300;
    const unsigned int numProto = 1;
    
   
    uint16_t        previous;
    static uint16_t current = 0;
    
    unsigned long duration;

    static enum PULSE_DETECT_T state = FIRST_PULSE_LEVEL;
    
    
    // FIXME: function name is confusing
    levelOld = levelNew;
    levelNew = rdata_level();
    
    // DEBUG: output pulses to compare to radio DO pin on oscilloscope (should match)
    //if (levelNew)
    //{
    //    reset_pin_on();
    //} else {
    //    reset_pin_off();
    //}
    
    
#if 1

    if (!levelOld && levelNew)
    {
        // rising edge
        reset_pin_on();
    } else if (levelOld && !levelNew) {
        // falling edge
        reset_pin_off();
    }
    
#endif

    // save in case check at end of interrupts reverts value
    //lowByteSaved  = lowByteNew;
    //highByteSaved = highByteNew;
    
    // we are always looking for pulse duration (i.e., difference), so need to store previous and new values
    lowByteOld  = lowByteNew;
    highByteOld = highByteNew;
    
    // this stores timer 2 value without stopping timer 2
    lowByteNew  = CCL1;
    highByteNew = CCH1;
    
    // go from 8-bit to 16-bit variables
    previous = current;
    current = (highByteNew << 8) | lowByteNew;
    
    //printf("P: %u\r\n", previous);
    //printf("C: %u\r\n", current);
    
    // check for overflow condition
    if (current < previous)
    {
        // DEBUG:
        //printf("Over:\r\n");
        
        // FIXME: no magic numbers
        // if overflow, we must compute difference by taking into account wrap around at maximum variable size
        duration = USHRT_MAX  - previous + current;
    } else {
        duration = current - previous;
    }
    
    // FIXME: no magic numbers
    // with timer clocked at (16 MHz / 4), four counts are needed to get one microsecond
    //duration = duration / 4;
    // 16 MHz / 12
    // e.g., (1/(16000000/24)) * dec(0xFFFF) = 98.30 milliseconds maximum can be counted
    duration = (duration * 3) / 2;
    
    // from oscillscope readings it appears that first sync pulse of first radio packet is frequently not output properly by receiver
    // this could be because radio receiver needs to "warm up" (despite already being enabled?)
    // and it is known that radio packet transmissions are often repeated (between about four and twenty times) perhaps in part for this reason
    // so detection assumes or relies on this behavior in algorithm to determine timing
    if (duration > separationLimit)
    {
        // A long stretch without signal level change occurred. This could
        // be the gap between two transmission.
        if ((repeatCount == 0) || (abs(duration - timings[0]) < 200))
        {
          // This long signal is close in length to the long signal which
          // started the previously recorded timings; this suggests that
          // it may indeed by a a gap between two transmissions (we assume
          // here that a sender will send the signal multiple times,
          // with roughly the same gap between them).
          repeatCount++;
          
          if (repeatCount == 2)
          {
            for(unsigned int i = 1; i <= numProto; i++)
            {
              if (receiveProtocol(i, changeCount))
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

    //RCSwitch::timings[changeCount++] = duration;
    timings[changeCount++] = duration;
    
    //lastTime = time;
        
    //clear compare/capture 1 flag
    CCCON &= ~0x02;
}


    // In this approach we will capture the time between each transition coming from the demodulation circuit. The Input Capture
    // function on a micro-controller is very useful for this because it will generate an interrupt, precise time measurements, and
    // allow decision processing based on the elapsed counter value.
    // 1. Set up timer to interrupt on every edge (may require changing edge trigger in the ISR)
    // FIXME: done externally to interrupt so that's okay right?
    
    // 2. ISR routine should flag the edge occurred and store count value
    
    
    // 3. Start timer, capture first edge and discard this.
    // FIXME: we do not discard the first edge but should
    
    // 4. Capture next edge and check if stored count value equal 2T (T = ½ data rate)
    
    
    // 5. Repeat step 4 until count value = 2T (This is now synchronized with the data clock)
    // 6. Read current logic level of the incoming pin and save as current bit value (1 or 0)
    // 7. Capture next edge
    // a. Compare stored count value with T
    // b. If value = T
    // ● Capture next edge and make sure this value also = T (else error)
    // ● Next bit = current bit
    // ● Return next bit
    // c. Else if value = 2T
    // ● Next bit = opposite of current bit
    // ● Return next bit
    // d. Else
    // ● Return error
    // 8. Store next bit in buffer
    // 9. If desired number of bits are decoded; exit to continue further processing
    // 10. Else set current bit to next bit and loop to step 7
