/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

#define MAX_PERIOD_COUNT 48

//extern volatile bool gSyncFirst;
//extern volatile bool gSyncSecond;
//extern volatile bool gCaptureDone;

struct RADIO_PACKET_T
{
    volatile bool syncFirstFlag;
    volatile bool syncSecondFlag;
    volatile bool captureDone;
    
    volatile uint8_t  count;
    
    volatile uint16_t syncFirstDuration;
    volatile uint16_t syncSecondDuration;
    volatile uint16_t duration[MAX_PERIOD_COUNT];
    volatile bool lastLevel;
};

extern __xdata struct RADIO_PACKET_T gPacket;
extern const uint16_t gTimings [];

extern volatile bool gSingleStep;

#endif