/*
 * globals.h
 *
 *  Ported on: 02.16.2023
 *      Author: Jonathan Armstrong
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include <8052.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// SFRs
__sfr __at (0xD2) P0M0;
__sfr __at (0xD3) P0M1;
__sfr __at (0xD4) P1M0;
__sfr __at (0xD5) P1M1;
__sfr __at (0xD9) PFCON;
__sfr __at (0xDA) P3M0;
__sfr __at (0xDB) P3M1;
__sfr __at (0x91) AUX;
__sfr __at (0xAA) SRELL;
__sfr __at (0xBA) SRELH;

// Timer2 Compare/Capture control register
__sfr __at (0xC1) CCEN;
__sfr __at (0xD1) CCEN2;
__sfr __at (0xC2) CCL1;
__sfr __at (0xC3) CCH1;
__sfr __at (0xC4) CCL2;
__sfr __at (0xC5) CCH2;
__sfr __at (0xC6) CCL3;
__sfr __at (0xC7) CCH3;
__sfr __at (0xC8) T2CON;
__sfr __at (0xC9) CCCON;
__sfr __at (0xCA) CRCL;
__sfr __at (0xCB) CRCH;
__sfr __at (0xCC) TL2;
__sfr __at (0xCD) TH2;

__sfr __at (0xA8) IEN0;

//sfr16 CC1   = 0xC2;
//sfr16 CC2   = 0xC4;
//sfr16 CC3   = 0xC6;
//sfr16 CRC   = 0xCA;
//sfr16 T2    = 0xCC;    // TH2 | TL2

// constants

// option is not including the 0xB1 bucket sniffing as this is only needed to define new protocols
// FIXME: this seems to overflow ram, needs testing
//#define INCLUDE_BUCKET_SNIFFING 1
#define INCLUDE_BUCKET_SNIFFING 0

//
#define SYSCLK 16000000
//
#define FIRMWARE_VERSION 0x03

// pin defines
#define BUZZER    P0_0
#define TDATA     P0_7
#define RDATA     P1_6
#define RF_ENABLE P1_7
#define LED       P3_0


// macros
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

#define START_GET(x) (uint8_t)(((x).status >> 12) & 0x0F)
#define START_INC(x) ((x).status = ((START_GET(x) + 1) << 12) | ((x).status & 0x0FFF))
#define START_CLEAR(x) ((x).status = 0, (x).bit_count = 0, (x).actual_bit_of_byte = 0)

#define BIT0_GET(x) (uint8_t)(((x).status >> 8) & 0x0F)
#define BIT0_INC(x) ((x).status = ((BIT0_GET(x) + 1) << 8) | ((x).status & 0xF0FF))
#define BIT0_CLEAR(x) ((x).status &= 0xF0FF)

#define BIT1_GET(x) (uint8_t)(((x).status >> 4) & 0x0F)
#define BIT1_INC(x) ((x).status = ((BIT1_GET(x) + 1) << 4) | ((x).status & 0xFF0F))
#define BIT1_CLEAR(x) ((x).status &= 0xFF0F)

#define BITS_CLEAR(x) ((x).status &= 0xF00F)

#define END_GET(x) (uint8_t)((x).status & 0x0F)
#define END_INC(x) ((x).status = END_GET(x) | ((x).status & 0xFFF0))
#define END_CLEAR(x) ((x).status &= 0xFFF0)

#define BITS_GET(x) (uint8_t)((x).bit_count)
#define BITS_INC(x) ((x).bit_count += 1)

#define ABP_GET(x) (uint8_t)((x).actual_bit_of_byte)
#define ABP_DEC(x) ((x).actual_bit_of_byte -= 1)
#define ABP_RESET(x) ((x).actual_bit_of_byte = 8)

#define PROTOCOL_BUCKETS(X) X ## _buckets
#define PROTOCOL_START(X) X ## _start
#define PROTOCOL_BIT0(X) X ## _bit0
#define PROTOCOL_BIT1(X) X ## _bit1
#define PROTOCOL_END(X) X ## _end

#define HIGH(x) ((x) | 0x08)
#define LOW(x) ((x) & 0x07)
#define BUCKET_NR(x) ((x) & 0x07)
#define BUCKET_STATE(x) (bool)(((x) & 0x08) >> 3)


void InitTimer2_us(uint16_t interval, uint16_t timeout);
extern void InitTimer2_ms(uint16_t interval, uint16_t timeout);
extern void WaitTimer2Finished(void);
extern void StopTimer2(void);
extern bool IsTimer2Finished(void);
extern bool IsTimer3Finished(void);

extern void led_on(void);
extern void led_off(void);
extern void led_toggle(void);
extern void buzzer_on(void);
extern void buzzer_off(void);
extern void tdata_on(void);
extern void tdata_off(void);
extern bool isrdataHigh(void);

#endif // INC_GLOBALS_H_
