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


// SFRs for OB38S003
// FIXME: this seems like a bad place to stores these
__sfr __at (0x8E) CKCON;


__sfr __at (0xA8) IEN0;


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
__sfr __at (0x9A) IEN2;

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


//sfr16 CC1   = 0xC2;
//sfr16 CC2   = 0xC4;
//sfr16 CC3   = 0xC6;
//sfr16 CRC   = 0xCA;
//sfr16 T2    = 0xCC;    // TH2 | TL2

// constants

// option is not including the 0xB1 bucket sniffing as this is only needed to define new protocols
// FIXME: this seems to overflow ram, needs testing
//#define INCLUDE_BUCKET_SNIFFING 1
//#define INCLUDE_BUCKET_SNIFFING 0

// FIXME: define a similar macro in delay.h
//#define SYSCLK 16000000
//
#define FIRMWARE_VERSION 0x03


extern void buzzer_on(void);
extern void buzzer_off(void);
extern bool rdata_level(void);
extern void led_on(void);
extern void led_off(void);
extern void led_toggle(void);
extern void radio_receiver_off(void);
extern void radio_receiver_on(void);
extern bool get_radio_wake(void);
extern void reset_pin_on(void);
extern void reset_pin_off(void);
extern void reset_pin_toggle(void);
extern void tdata_on(void);
extern void tdata_off(void);
extern void uart_tx_pin_off(void);
extern void uart_tx_pin_on(void);
extern void uart_tx_pin_toggle(void);

#endif // INC_GLOBALS_H_
