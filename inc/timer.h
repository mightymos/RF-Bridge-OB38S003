/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

#define MAX_COUNT 48

//extern volatile bool gSyncFirst;
//extern volatile bool gSyncSecond;
//extern volatile bool gCaptureDone;

struct RADIO_PACKET_T
{
    volatile bool syncFirst;
    volatile bool syncSecond;
    volatile bool captureDone;
    
    volatile uint8_t  count;
    
    volatile uint16_t syncFirstDuration;
    volatile uint16_t syncSecondDuration;
    volatile uint16_t duration[MAX_COUNT];
    volatile unsigned long diff;
};

extern __xdata struct RADIO_PACKET_T gPacket;
extern const uint16_t timings [];

extern volatile bool gSingleStep;

#endif