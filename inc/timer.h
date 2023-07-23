/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

#define MAX_PERIOD_COUNT 48
#define BUFFER_SIZE 24

//extern volatile bool gSyncFirst;
//extern volatile bool gSyncSecond;
//extern volatile bool gCaptureDone;
extern uint16_t gTimerOneCount;
extern bool     gIsTimerOneFinished;

struct RADIO_PACKET_T
{
    volatile bool syncFirstFlag;
    volatile bool syncSecondFlag;
    volatile bool captureDone;
    
    volatile uint8_t  count;
    
    volatile uint16_t syncFirstDuration;
    volatile uint16_t syncSecondDuration;
    //volatile uint16_t duration[MAX_PERIOD_COUNT];
    
    volatile bool lastLevel;
    
    unsigned long diff[BUFFER_SIZE];
    bool radioBits[24];
    
    uint16_t errors;
    
    uint8_t writePosition;
    uint8_t readPosition;
    uint8_t length;
};


struct RC_SWITCH_T
{
    unsigned long nReceivedValue;
    unsigned int nReceivedBitlength;
    unsigned int nReceivedDelay;
    unsigned int nReceivedProtocol;
};

extern __xdata struct RADIO_PACKET_T gPacket;

extern __xdata struct RC_SWITCH_T gRCSwitch;

extern const uint16_t gTimings [];

//extern volatile bool gSingleStep;

extern unsigned long get_current_time(void);
extern unsigned long get_elapsed_time(unsigned long previousTime);
//extern void reload_timer1(unsigned int reload);
extern void reload_timer1(unsigned char high, unsigned char low);

#endif