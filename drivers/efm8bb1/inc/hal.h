/*
 * Globals.h
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include <stdbool.h>
#include <stdint.h>
#include <EFM8BB1.h>



// this seems to start sniffing by default as opposed to waiting for command over uart
#define SNIFFING_ON_AT_STARTUP 1

#define SYSCLK	24500000

#define FIRMWARE_VERSION		0x03

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

// USER PROTOTYPES
//SI_SBIT(LED,    SFR_P1, 0);		// LED
//SI_SBIT(T_DATA, SFR_P0, 0);		// T_DATA
//SI_SBIT(R_DATA, SFR_P1, 3);		// R_DATA
//SI_SBIT(BUZZER, SFR_P1, 6);		// BUZZER
//__sbit __at (0x90) LED;
//__sbit __at (0x80) T_DATA;
//__sbit __at (0x93) R_DATA;
//__sbit __at (0x96) BUZZER;


#define TDATA  P0_0
// Sonoff Bridge
// EFM8BB1LCK board (uncomment only one LED line)
//#define LED    P1_0
#define LED    P1_4
#define RDATA  P1_3
#define BUZZER P1_6

#define DEBUG_PIN0 P1_5
#define DEBUG_PIN1 P1_6

inline void buzzer_on(void)
{
    BUZZER = 1;
}

inline void buzzer_off(void)
{
    BUZZER = 0;
}

inline bool rdata_level(void)
{
    return RDATA;
}

// setter prototypes
inline void led_on(void)
{
	// sonoff bridge
    //LED = 1;
	// EFM8BB1LCK board
	LED = 0;
}

inline void led_off(void)
{
	// sonoff bridge
    //LED = 0;
	// EFM8BB1LCK board
	LED = 1;
}

inline void led_toggle(void)
{
    LED = !LED;
}


inline void tdata_on(void)
{
    TDATA = 1;
}


inline void tdata_off(void)
{
    TDATA = 0;
}

inline void debug_pin0_on(void)
{
	DEBUG_PIN0 = 1;
}

inline void debug_pin0_off(void)
{
	DEBUG_PIN0 = 0;
}

inline void debug_pin0_toggle(void)
{
	DEBUG_PIN0 = !DEBUG_PIN0;
}

inline void debug_pin1_on(void)
{
	DEBUG_PIN1 = 1;
}

inline void debug_pin1_off(void)
{
	DEBUG_PIN1 = 0;
}

inline void debug_pin1_toggle(void)
{
	DEBUG_PIN1 = !DEBUG_PIN1;
}


inline void enable_global_interrupts(void)
{
    EA = 1;
}

inline void disable_global_interrupts(void)
{
    EA = 0;
}

extern void InitTimer2_us(uint16_t interval, uint16_t timeout);
extern void InitTimer3_us(uint16_t interval, uint16_t timeout);
extern void InitTimer2_ms(uint16_t interval, uint16_t timeout);
extern void InitTimer3_ms(uint16_t interval, uint16_t timeout);
extern void WaitTimer2Finished(void);
extern void WaitTimer3Finished(void);
extern void StopTimer2(void);
extern void StopTimer3(void);
extern bool IsTimer2Finished(void);
extern bool IsTimer3Finished(void);

#endif // INC_GLOBALS_H_
