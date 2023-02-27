/*
 * timers.h
 *
 *  Ported on: 02.25.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

#define MAX_COUNT 50

extern volatile bool gCaptureDone;

extern __xdata volatile uint16_t gDuration[MAX_COUNT];
extern volatile uint8_t  gCount;

#endif