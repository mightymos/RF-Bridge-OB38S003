#include <limits.h>
#include <stdint.h>
#include <stdlib.h>


#include "hal.h"
//#include "rcswitch.h"
#include "rcswitch_tasmota.h"
#include "timer.h"


// track count of timer 1 which is decremented
//uint16_t gTimerOneCount = 0;

// finally when timer reaches zero, set flag to false
//bool gIsTimerOneFinished = true;

// track time since startup in one millisecond increments
static unsigned long gTimeMilliseconds = 0;
static unsigned long gTimeTenMicroseconds = 0;


unsigned long get_current_timer0(void)
{
    unsigned long currentTime;
    
    // FIXME: disable timer0 interrupt for atomic reading of variable
    //        consider check to see if interrupt was enabled in the first place
    disable_timer0_interrupt();
    
    // FIXME: compute the proper conversion from counts to milliseconds
    currentTime = gTimeMilliseconds;
    
    // re-enable timer0 interrupt
    enable_timer0_interrupt();
    
    return currentTime;
}


unsigned long get_elapsed_timer0(unsigned long previousTime)
{
    unsigned long currentTime;
    unsigned long elapsed;
    
    currentTime = get_current_timer0();
    
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

unsigned long get_current_timer1(void)
{
    unsigned long currentTime;
    

    disable_timer1_interrupt();
    
    // FIXME: compute the proper conversion from counts to microseconds
    currentTime = gTimeTenMicroseconds;
    
    enable_timer1_interrupt();
    
    return currentTime;
}


unsigned long get_elapsed_timer1(unsigned long previousTime)
{
    unsigned long currentTime;
    unsigned long elapsed;
    
    currentTime = get_current_timer1();
    
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




//void timer0_isr(void) __interrupt (1)
//{
//    gTimeMilliseconds++;
//
//    // one millisecond to overflow
//    load_timer0(0xc17f);
//    
//    // ten microseconds to overflow
//    //TH0 = 0xff;
//    //TL0 = 0x5f;
//}


// timer 1 interrupt
void timer1_isr(void) __interrupt (3)
{
    // tracks time since timer enabled, used to track long periods of time
    gTimeTenMicroseconds++;
    
    // ten microseconds to overflow
    load_timer1(0xff5f);
}

//-----------------------------------------------------------------------------
// timer 2 should previously be set in capture mode 0 - pg. 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (5)
{
    const uint8_t gapMagicNumber  = 200;
    const uint8_t repeatThreshold   = 2;
    
    // track previous and new timer values so we can compute dfiference
    uint8_t lowByteOld;
    uint8_t highByteOld;
    static uint8_t lowByteNew  = 0;
    static uint8_t highByteNew = 0;
    
    // for converting 8-bit timer values to 16-bits to allow subtraction
    uint16_t        previous;
    static uint16_t current = 0;
    
    // this eventually represents the level duration in microseconds (difference between edge transitions)
    unsigned long duration;
    
    // store previous level values so we can hook up oscilloscope channel on radio transmitter device
    //  and another channel on reset pin to observe that edges are being captured on radio receiver
    bool        levelOld;
    static bool levelNew = false;
    

    // rc-switch variables
    static unsigned int repeatCount = 0;
    static unsigned int changeCount = 0;

    // FIXME: move to rcswitch.h
    const unsigned int separationLimit = RCSWITCH_SEPARATION_LIMIT;
    
    const uint8_t numProto = get_num_protos();
    
    // FIXME: function name is confusing
    levelOld = levelNew;
    levelNew = rdata_level();
    
    
#if 0

    // DEBUG: on oscilloscope
    // we use edge transitions rather than absolute level for no particular reason
    if (!levelOld && levelNew)
    {
        // rising edge
        reset_pin_on();
    } else if (levelOld && !levelNew) {
        // falling edge
        reset_pin_off();
    }
    
#endif

    
    // we are always looking for pulse duration (i.e., difference), so need to store previous and new values
    lowByteOld  = lowByteNew;
    highByteOld = highByteNew;
    
    // this stores timer 2 value without stopping timer 2
    lowByteNew  = get_timer2_low();
    highByteNew = get_timer2_high();
    
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
    
    buftimings[3] = buftimings[2];
    buftimings[2] = buftimings[1];
    buftimings[1] = buftimings[0];
    buftimings[0] = duration;
    
    if (duration > separationLimit ||
      changeCount == 156 ||
      (abs(buftimings[3] - buftimings[2]) < 50 &&
        abs(buftimings[2] - buftimings[1]) < 50 &&
        changeCount > 25)) {
    // принят длинный импульс продолжительностью более nSeparationLimit (4300)
    // A long stretch without signal level change occurred. This could
    // be the gap between two transmission.
    if (abs(duration - timings[0]) < 400 ||
        changeCount == 156 ||
        (abs(buftimings[3] - timings[1]) < 50 &&
          abs(buftimings[2] - timings[2]) < 50 &&
          abs(buftimings[1] - timings[3]) < 50 &&
          changeCount > 25)) {
      // если его длительность отличается от первого импульса,
      // который приняли раньше, менее чем на +-200 (исходно 200)
      // то считаем это повторным пакетом и игнорируем его
      // This long signal is close in length to the long signal which
      // started the previously recorded timings; this suggests that
      // it may indeed by a a gap between two transmissions (we assume
      // here that a sender will send the signal multiple times,
      // with roughly the same gap between them).

      // количество повторных пакетов
      repeatCount++;
      // при приеме второго повторного начинаем анализ принятого первым
      if (repeatCount == 1) {
        unsigned long long thismask = 1;
        for(unsigned int i = 1; i <= numProto; i++) {
            if (((1ULL << numProto) - 1) & thismask) {
          //if (nReceiveProtocolMask & thismask) {
            if (receive_protocol(i, changeCount)) {
              // receive succeeded for protocol i
              break;
            }
          }
          thismask <<= 1;
        }
        // очищаем количество повторных пакетов
        repeatCount = 0;
      }
    }
    // дительность отличается более чем на +-200 от первого
    // принятого ранее, очищаем счетчик для приема нового пакета
    changeCount = 0;
    if (abs(buftimings[3] - buftimings[2]) < 50 &&
        abs(buftimings[2] - buftimings[1]) < 50) {
      timings[1] = buftimings[3];
      timings[2] = buftimings[2];
      timings[3] = buftimings[1];
      changeCount = 4;
    }
    }

    // detect overflow
    if (changeCount >= RCSWITCH_MAX_CHANGES) {
        changeCount = 0;
        repeatCount = 0;
    }

    // заносим в массив длительность очередного принятого импульса
    // игнорируем шумовые всплески менее 100 мкс
    if (changeCount > 0 && duration < 100) {
        timings[changeCount-1] += duration;
    } else {
        timings[changeCount++] = duration;
    }


    
        
    //clear compare/capture 1 flag
    clear_ccp1_flag();
}