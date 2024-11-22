/*
 * hal.h
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#ifndef PORTISCH_MACROS_H
#define PORTISCH_MACROS_H

#include <stdbool.h>
#include <stdint.h>

// macros
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))


// used to help with defining stored protocol constants
#define PROTOCOL_BUCKETS(X) X ## _buckets
#define PROTOCOL_START(X) X ## _start
#define PROTOCOL_BIT0(X) X ## _bit0
#define PROTOCOL_BIT1(X) X ## _bit1
#define PROTOCOL_END(X) X ## _end

// set bit to high or low to indicate pin logic level
#define HIGH(x) ((x) | 0x08)
#define LOW(x) ((x) & 0x07)


#endif // PORTISCH_MACROS_H
