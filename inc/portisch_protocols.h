/*
 * RF_Protocols.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_RF_PROTOCOLS_H_
#define INC_RF_PROTOCOLS_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stddef.h>
#include <stdint.h>

#include "portisch_macros.h"
#include "portisch_config.h"

/*
 * bucket sniffing constants
 */
#define MIN_FOOTER_LENGTH	300
#define MIN_BUCKET_LENGTH	100

#define PT226x_SYNC_MIN		4500

/*
 * sync constants
 */
#define TOLERANCE_MAX		500
#define TOLERANCE_MIN		100

/*
 * number of repeating by default
 */
#define RF_TRANSMIT_REPEATS		8

typedef struct PROTOCOL_STATUS
{
	//uint16_t status;
	uint8_t sync_status;
	uint8_t bit0_status;
	uint8_t bit1_status;
	uint8_t end_status;
	uint8_t bit_count;
	uint8_t actual_bit_of_byte;
} PROTOCOL_STATUS;

typedef struct PROTOCOL_DATA_UINT8_T
{
	// pointer to array of uint8_t elements
	uint8_t* dat;
	// size of the array
	uint8_t size;
} PROTOCOL_DATA_UINT8_T;

typedef struct PROTOCOL_DATA_UINT16_T
{
	// pointer to array of uint16_t elements
	uint16_t* dat;
	// size of the array
	uint8_t size;
} PROTOCOL_DATA_UINT16_T;

typedef struct BUCKET_PROTOCOL_DATA
{
	// array and array size of buckets
	PROTOCOL_DATA_UINT16_T buckets;
	// array and array size of start buckets
	PROTOCOL_DATA_UINT8_T start;
	// array and array size of bit 0 buckets
	PROTOCOL_DATA_UINT8_T bit0;
	// array and array size of bit 1 buckets
	PROTOCOL_DATA_UINT8_T bit1;
	// array and array size of end buckets
	PROTOCOL_DATA_UINT8_T end;
	// bit count for this protocol
	uint8_t bit_count;
} BUCKET_PROTOCOL_DATA;

/*
 * PT2260, EV1527,... original RF bridge protocol
 * http://www.princeton.com.tw/Portals/0/Product/PT2260_4.pdf
 */
#if EFM8BB1_SUPPORT_PT226X_PROTOCOL == 1
#define PT226X
__code static uint16_t PROTOCOL_BUCKETS(PT226X)[] = { 350, 1050, 10850 };
__code static uint8_t  PROTOCOL_START(PT226X)[]   = { HIGH(0), LOW(2) };
__code static uint8_t  PROTOCOL_BIT0(PT226X)[]    = { HIGH(0), LOW(1) };
__code static uint8_t  PROTOCOL_BIT1(PT226X)[]    = { HIGH(1), LOW(0) };
#endif

/*
 * Rohrmotor24
 * https://github.com/bjwelker/Raspi-Rollo/tree/master/Arduino/Rollo_Code_Receiver
 */
#if EFM8BB1_SUPPORT_Rohrmotor24_PROTOCOL == 1
#define Rohrmotor24
__code static uint16_t PROTOCOL_BUCKETS(Rohrmotor24)[] = { 370, 740, 4800, 1500, 8400};
__code static uint8_t PROTOCOL_START(Rohrmotor24)[] = { HIGH(2), LOW(3) };
__code static uint8_t PROTOCOL_BIT0(Rohrmotor24)[]  = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(Rohrmotor24)[]  = { HIGH(1), LOW(0) };
__code static uint8_t PROTOCOL_END(Rohrmotor24)[]   = { LOW(4) };
#endif

/*
 * UNDERWATER PAR56 LED LAMP, 502266
 * http://www.seamaid-lighting.com/de/produit/lampe-par56/
 */
#if EFM8BB1_SUPPORT_PAR56_PROTOCOL == 1
#define PAR56
__code static uint16_t PROTOCOL_BUCKETS(PAR56)[] = { 380, 1100, 3000, 9000};
__code static uint8_t PROTOCOL_START(PAR56)[] = { HIGH(2), LOW(3) };
__code static uint8_t PROTOCOL_BIT0(PAR56)[]  = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(PAR56)[]  = { HIGH(1), LOW(0) };
#endif

/*
 * Alecto WS-1200 Series Wireless Weather Station
 */
#if EFM8BB1_SUPPORT_WS_1200_PROTOCOL == 1
#define WS_1200
__code static uint16_t PROTOCOL_BUCKETS(WS_1200)[] = { 500, 1000, 1500, 29500 };
__code static uint8_t PROTOCOL_START(WS_1200)[] = { LOW(3) };
__code static uint8_t PROTOCOL_BIT0(WS_1200)[]  = { HIGH(2), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(WS_1200)[]  = { HIGH(0), LOW(1) };
#endif

/*
 * ALDI Remote controlled wall sockets, 4x
  */
#if EFM8BB1_SUPPORT_ALDI_4x_PROTOCOL == 1
#define ALDI_4x
__code static uint16_t PROTOCOL_BUCKETS(ALDI_4x)[] = { 400, 1200, 3000, 7250};
__code static uint8_t PROTOCOL_START(ALDI_4x)[] = { HIGH(2), LOW(3) };
__code static uint8_t PROTOCOL_BIT0(ALDI_4x)[]  = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(ALDI_4x)[]  = { HIGH(1), LOW(0) };
#endif

/*
 * HT6P20X chips
 * http://www.holtek.com.tw/documents/10179/11842/6p20v170.pdf
 */
#if EFM8BB1_SUPPORT_HT6P20X_PROTOCOL == 1
#define HT6P20X
__code static uint16_t PROTOCOL_BUCKETS(HT6P20X)[] = { 450, 900, 10350};
__code static uint8_t PROTOCOL_START(HT6P20X)[]    = { LOW(2), HIGH(0) };
__code static uint8_t PROTOCOL_BIT0(HT6P20X)[]     = { LOW(0), HIGH(1) };
__code static uint8_t PROTOCOL_BIT1(HT6P20X)[]     = { LOW(1), HIGH(0) };
#endif

/*
 * HT12A/HT12E chips
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12_PROTOCOL == 1
#define HT12
__code static uint16_t PROTOCOL_BUCKETS(HT12)[] = { 330, 630, 10830};
__code static uint8_t PROTOCOL_START(HT12)[]    = { LOW(2), HIGH(0) };
__code static uint8_t PROTOCOL_BIT0(HT12)[]     = { LOW(0), HIGH(1) };
__code static uint8_t PROTOCOL_BIT1(HT12)[]     = { LOW(1), HIGH(0) };
#endif

/*
 * HT12A/HT12E chips - Generic Doorbell
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12a_PROTOCOL == 1
#define HT12a
__code static uint16_t PROTOCOL_BUCKETS(HT12a)[] = { 200, 380, 6950};
__code static uint8_t PROTOCOL_START(HT12a)[] = { LOW(2), HIGH(0) };
__code static uint8_t PROTOCOL_BIT0(HT12a)[]  = { LOW(0), HIGH(1) };
__code static uint8_t PROTOCOL_BIT1(HT12a)[]  = { LOW(1), HIGH(0) };
#endif

/*
 * HT12A/HT12E chips - Atag Extractor - Plus/Minus
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
#define HT12b
__code static uint16_t PROTOCOL_BUCKETS(HT12b)[] = { 350, 650, 13000};
__code static uint8_t PROTOCOL_START(HT12b)[] = { LOW(2), HIGH(0) };
__code static uint8_t PROTOCOL_BIT0(HT12b)[]  = { LOW(0), HIGH(1) };
__code static uint8_t PROTOCOL_BIT1(HT12b)[]  = { LOW(1), HIGH(0) };
#endif

/*
 * HT12A/HT12E chips - Atag Extractor - Lights/Timer
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
#define HT12c
__code static uint16_t PROTOCOL_BUCKETS(HT12c)[] = { 350, 700, 15650};
__code static uint8_t PROTOCOL_START(HT12c)[] = { LOW(2), HIGH(0) };
__code static uint8_tPROTOCOL_BIT0(HT12c)[]   = { LOW(0), HIGH(1) };
__code static uint8_t PROTOCOL_BIT1(HT12c)[]  = { LOW(1), HIGH(0) };
#endif

/*
 * Meteo SPxx -  Weather station (PHU Metrex)
 * https://gist.github.com/klaper/ce3ba02501516d9a6d294367d2c300a6
 */
#if EFM8BB1_SUPPORT_SP45_PROTOCOL == 1
#define SP45
__code static uint16_t PROTOCOL_BUCKETS(SP45)[] = { 650, 7810, 1820, 3980 };
__code static uint8_t PROTOCOL_START(SP45)[] = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT0(SP45)[]  = { HIGH(0), LOW(2) };
__code static uint8_t PROTOCOL_BIT1(SP45)[]  = { HIGH(0), LOW(3) };
#endif

/*
 * Dooya DC90 remote
 */
#if EFM8BB1_SUPPORT_DC90_PROTOCOL == 1
#define DC90
__code static uint16_t PROTOCOL_BUCKETS(DC90)[] = { 360, 720, 4800, 1500 };
__code static uint8_t PROTOCOL_START(DC90)[] = { HIGH(2), LOW(3) };
__code static uint8_t PROTOCOL_BIT0(DC90)[]  = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(DC90)[]  = { HIGH(1), LOW(0) };
#endif

/*
 * Digoo DG-HOSA Smart 433MHz Wireless Household Carbon Monoxide Sensor
 */
#if EFM8BB1_SUPPORT_DG_HOSA_PROTOCOL == 1
#define DG_HOSA
__code static uint16_t PROTOCOL_BUCKETS(DG_HOSA)[] = { 590, 1500, 430, 13450 };
__code static uint8_t PROTOCOL_START(DG_HOSA)[] = { HIGH(2), LOW(3) };
__code static uint8_t PROTOCOL_BIT0(DG_HOSA)[]  = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(DG_HOSA)[]  = { HIGH(1), LOW(0) };
#endif

/*
 * KaKu wall sockets
 */
#if EFM8BB1_SUPPORT_Kaku_PROTOCOL == 1
#define KaKu
__code static uint16_t PROTOCOL_BUCKETS(KaKu)[] = { 300, 2560, 140, 1150, 10230 };
__code static uint8_t PROTOCOL_START(KaKu)[] = {  HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT0(KaKu)[]  = {  HIGH(0), LOW(3),  HIGH(0), LOW(2) };
__code static uint8_t PROTOCOL_BIT1(KaKu)[]  = {  HIGH(0), LOW(2),  HIGH(0), LOW(3) };
__code static uint8_t PROTOCOL_END(KaKu)[]   = {  HIGH(0), LOW(4) };
#endif

/*
 * DIO Chacon RF 433Mhz #95
 */
#if EFM8BB1_SUPPORT_DIO_PROTOCOL == 1
#define DIO_emg
__code static uint16_t PROTOCOL_BUCKETS(DIO_emg)[] = { 260, 2714, 1300, 10400 };
__code static uint8_t PROTOCOL_START(DIO_emg)[] = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT0(DIO_emg)[]  = { HIGH(0), LOW(0), HIGH(0), LOW(2) };
__code static uint8_t PROTOCOL_BIT1(DIO_emg)[]  = { HIGH(0), LOW(2), HIGH(0), LOW(0) };
__code static uint8_t PROTOCOL_END(DIO_emg)[]   = { HIGH(0), LOW(3) };
#endif

/*
 * 1ByOne Doorbell, PR #97
 */
#if EFM8BB1_SUPPORT_1BYONE_PROTOCOL == 1
#define OneByOne
__code static uint16_t PROTOCOL_BUCKETS(OneByOne)[] = { 370, 1080, 6530 };
__code static uint8_t PROTOCOL_START(OneByOne)[] = { LOW(2), HIGH(0) };
__code static uint8_t PROTOCOL_BIT0(OneByOne)[]  = { LOW(1), HIGH(0) };
__code static uint8_t PROTOCOL_BIT1(OneByOne)[]  = { LOW(0), HIGH(1) };
#endif

/*
 * Prologue Sensor #96
 */
#if EFM8BB1_SUPPORT_Prologue_PROTOCOL == 1
#define Prologue
__code static uint16_t PROTOCOL_BUCKETS(Prologue)[] = { 660, 2000, 4000, 9000 };
__code static uint8_t PROTOCOL_START(Prologue)[] = { HIGH(0), LOW(3) };
__code static uint8_t PROTOCOL_BIT0(Prologue)[]  = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(Prologue)[]  = { HIGH(0), LOW(2) };
__code static uint8_t PROTOCOL_END(Prologue)[]   = { HIGH(0), LOW(1) };
#endif

/*
 * T-187-N (TX)-1 Generic Dog Training Collar Remote Control
 */
#if EFM8BB1_SUPPORT_DOG_COLLAR_PROTOCOL == 1
#define DogCollar
__code static uint16_t PROTOCOL_BUCKETS(DogCollar)[] = { 1560, 720, 210 };
__code static uint8_t PROTOCOL_START(DogCollar)[] = { HIGH(0), LOW(1) };
__code static uint8_t PROTOCOL_BIT0(DogCollar)[]  = { HIGH(2), LOW(1) };
__code static uint8_t PROTOCOL_BIT1(DogCollar)[]  = { HIGH(1), LOW(2) };
__code static uint8_t PROTOCOL_END(DogCollar)[]   = { HIGH(2), LOW(1) };
#endif

/*
 * Byron BY302 Doorbell
 */
#if EFM8BB1_SUPPORT_BY302_PROTOCOL == 1
#define BY302
__code static uint16_t PROTOCOL_BUCKETS(BY302)[] = { 470, 1020, 3070 };
__code static uint8_t PROTOCOL_START(BY302)[] = { LOW(2), HIGH(0) };
__code static uint8_t PROTOCOL_BIT0(BY302)[]  = { LOW(1), HIGH(0) };
__code static uint8_t PROTOCOL_BIT1(BY302)[]  = { LOW(0), HIGH(1) };
#endif

/*
 * 5514 SILENT Dual Tech
 */
#if EFM8BB1_SUPPORT_DT_5514_PROTOCOL == 1
#define DT_5514
__code static uint16_t PROTOCOL_BUCKETS(DT_5514)[] = { 400, 720, 4910 };
__code static uint8_t PROTOCOL_START(DT_5514)[] = { LOW(1), HIGH(2) };
__code static uint8_t PROTOCOL_BIT0(DT_5514)[]  = { LOW(0), HIGH(1) };
__code static uint8_t PROTOCOL_BIT1(DT_5514)[]  = { LOW(1), HIGH(0) };
#endif

/*
 * Auriol H13726 Weather Station
 */
#if EFM8BB1_SUPPORT_H13726_PROTOCOL == 1
#define H13726
__code static uint16_t PROTOCOL_BUCKETS(H13726)[] = { 560, 1910, 3890, 8820 };
__code static uint8_t PROTOCOL_START(H13726)[] = { LOW(3), HIGH(0) };
__code static uint8_t PROTOCOL_BIT0(H13726)[]  = { LOW(1), HIGH(0) };
__code static uint8_t PROTOCOL_BIT1(H13726)[]  = { LOW(2), HIGH(0) };
#endif

__code static struct BUCKET_PROTOCOL_DATA PROTOCOL_DATA[] =
{
#if EFM8BB1_SUPPORT_PT226X_PROTOCOL == 1
		/*
		 * PT2260, EV1527,... original RF bridge protocol
		 */
		{
			{ &PROTOCOL_BUCKETS(PT226X)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(PT226X)) },
			{ &PROTOCOL_START(PT226X)[0], ARRAY_LENGTH(PROTOCOL_START(PT226X)) },
			{ &PROTOCOL_BIT0(PT226X)[0], ARRAY_LENGTH(PROTOCOL_BIT0(PT226X)) },
			{ &PROTOCOL_BIT1(PT226X)[0], ARRAY_LENGTH(PROTOCOL_BIT1(PT226X)) },
			{ NULL, 0 },
			24
		},
#endif
#if EFM8BB1_SUPPORT_Rohrmotor24_PROTOCOL == 1
		/*
		 * Rohrmotor24
		 */
		{
			{ &PROTOCOL_BUCKETS(Rohrmotor24)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(Rohrmotor24)) },
			{ &PROTOCOL_START(Rohrmotor24)[0], ARRAY_LENGTH(PROTOCOL_START(Rohrmotor24)) },
			{ &PROTOCOL_BIT0(Rohrmotor24)[0], ARRAY_LENGTH(PROTOCOL_BIT0(Rohrmotor24)) },
			{ &PROTOCOL_BIT1(Rohrmotor24)[0], ARRAY_LENGTH(PROTOCOL_BIT1(Rohrmotor24)) },
			{ &PROTOCOL_END(Rohrmotor24)[0], ARRAY_LENGTH(PROTOCOL_END(Rohrmotor24)) },
			40
		},
#endif
#if EFM8BB1_SUPPORT_PAR56_PROTOCOL == 1
		/*
		 * UNDERWATER PAR56 LED LAMP, 502266
		 */
		{
			{ &PROTOCOL_BUCKETS(PAR56)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(PAR56)) },
			{ &PROTOCOL_START(PAR56)[0], ARRAY_LENGTH(PROTOCOL_START(PAR56)) },
			{ &PROTOCOL_BIT0(PAR56)[0], ARRAY_LENGTH(PROTOCOL_BIT0(PAR56)) },
			{ &PROTOCOL_BIT1(PAR56)[0], ARRAY_LENGTH(PROTOCOL_BIT1(PAR56)) },
			{ NULL, 0 },
			24
		},
#endif
#if EFM8BB1_SUPPORT_WS_1200_PROTOCOL == 1
		/*
		 * Alecto WS-1200 Series Wireless Weather Station
		 */
		{
			{ &PROTOCOL_BUCKETS(WS_1200)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(WS_1200)) },
			{ &PROTOCOL_START(WS_1200)[0], ARRAY_LENGTH(PROTOCOL_START(WS_1200)) },
			{ &PROTOCOL_BIT0(WS_1200)[0], ARRAY_LENGTH(PROTOCOL_BIT0(WS_1200)) },
			{ &PROTOCOL_BIT1(WS_1200)[0], ARRAY_LENGTH(PROTOCOL_BIT1(WS_1200)) },
			{ NULL, 0 },
			71
		},
#endif
#if EFM8BB1_SUPPORT_ALDI_4x_PROTOCOL == 1
		/*
		 * ALDI Remote controlled wall sockets, 4x
		 */
		{
			{ &PROTOCOL_BUCKETS(ALDI_4x)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(ALDI_4x)) },
			{ &PROTOCOL_START(ALDI_4x)[0], ARRAY_LENGTH(PROTOCOL_START(ALDI_4x)) },
			{ &PROTOCOL_BIT0(ALDI_4x)[0], ARRAY_LENGTH(PROTOCOL_BIT0(ALDI_4x)) },
			{ &PROTOCOL_BIT1(ALDI_4x)[0], ARRAY_LENGTH(PROTOCOL_BIT1(ALDI_4x)) },
			{ NULL, 0 },
			24
		},
#endif
#if EFM8BB1_SUPPORT_HT6P20X_PROTOCOL == 1
		/*
		 * HT6P20X chips
		 */
		{
			{ &PROTOCOL_BUCKETS(HT6P20X)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(HT6P20X)) },
			{ &PROTOCOL_START(HT6P20X)[0], ARRAY_LENGTH(PROTOCOL_START(HT6P20X)) },
			{ &PROTOCOL_BIT0(HT6P20X)[0], ARRAY_LENGTH(PROTOCOL_BIT0(HT6P20X)) },
			{ &PROTOCOL_BIT1(HT6P20X)[0], ARRAY_LENGTH(PROTOCOL_BIT1(HT6P20X)) },
			{ NULL, 0 },
			24
		},
#endif
#if EFM8BB1_SUPPORT_HT12_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips
		 */
		{
			{ &PROTOCOL_BUCKETS(HT12)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(HT12)) },
			{ &PROTOCOL_START(HT12)[0], ARRAY_LENGTH(PROTOCOL_START(HT12)) },
			{ &PROTOCOL_BIT0(HT12)[0], ARRAY_LENGTH(PROTOCOL_BIT0(HT12)) },
			{ &PROTOCOL_BIT1(HT12)[0], ARRAY_LENGTH(PROTOCOL_BIT1(HT12)) },
			{ NULL, 0 },
			12
		},
#endif
#if EFM8BB1_SUPPORT_HT12a_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips - A
		 */
		{
			{ &PROTOCOL_BUCKETS(HT12a)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(HT12a)) },
			{ &PROTOCOL_START(HT12a)[0], ARRAY_LENGTH(PROTOCOL_START(HT12a)) },
			{ &PROTOCOL_BIT0(HT12a)[0], ARRAY_LENGTH(PROTOCOL_BIT0(HT12a)) },
			{ &PROTOCOL_BIT1(HT12a)[0], ARRAY_LENGTH(PROTOCOL_BIT1(HT12a)) },
			{ NULL, 0 },
			12
		},
#endif
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips - B
		 */
		{
			{ &PROTOCOL_BUCKETS(HT12b)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(HT12b)) },
			{ &PROTOCOL_START(HT12b)[0], ARRAY_LENGTH(PROTOCOL_START(HT12b)) },
			{ &PROTOCOL_BIT0(HT12b)[0], ARRAY_LENGTH(PROTOCOL_BIT0(HT12b)) },
			{ &PROTOCOL_BIT1(HT12b)[0], ARRAY_LENGTH(PROTOCOL_BIT1(HT12b)) },
			{ NULL, 0 },
			12
		},
#endif
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips - C
		 */
		{
			{ &PROTOCOL_BUCKETS(HT12c)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(HT12c)) },
			{ &PROTOCOL_START(HT12c)[0], ARRAY_LENGTH(PROTOCOL_START(HT12c)) },
			{ &PROTOCOL_BIT0(HT12c)[0], ARRAY_LENGTH(PROTOCOL_BIT0(HT12c)) },
			{ &PROTOCOL_BIT1(HT12c)[0], ARRAY_LENGTH(PROTOCOL_BIT1(HT12c)) },
			{ NULL, 0 },
			18
		},
#endif
#if EFM8BB1_SUPPORT_SP45_PROTOCOL == 1
		/*
		 * Meteo SPxx -  Weather station (PHU Metrex)
		 */
		{
			{ &PROTOCOL_BUCKETS(SP45)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(SP45)) },
			{ &PROTOCOL_START(SP45)[0], ARRAY_LENGTH(PROTOCOL_START(SP45)) },
			{ &PROTOCOL_BIT0(SP45)[0], ARRAY_LENGTH(PROTOCOL_BIT0(SP45)) },
			{ &PROTOCOL_BIT1(SP45)[0], ARRAY_LENGTH(PROTOCOL_BIT1(SP45)) },
			{ NULL, 0 },
			40
		},
#endif
#if EFM8BB1_SUPPORT_DC90_PROTOCOL == 1
		/*
		 * Dooya DC90 remote
		 */
		{
			{ &PROTOCOL_BUCKETS(DC90)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(DC90)) },
			{ &PROTOCOL_START(DC90)[0], ARRAY_LENGTH(PROTOCOL_START(DC90)) },
			{ &PROTOCOL_BIT0(DC90)[0], ARRAY_LENGTH(PROTOCOL_BIT0(DC90)) },
			{ &PROTOCOL_BIT1(DC90)[0], ARRAY_LENGTH(PROTOCOL_BIT1(DC90)) },
			{ NULL, 0 },
			40
		},
#endif
#if EFM8BB1_SUPPORT_DG_HOSA_PROTOCOL == 1
		/*
		 * Digoo DG-HOSA Smart 433MHz Wireless Household Carbon Monoxide Sensor
		 */
		{
			{ &PROTOCOL_BUCKETS(DG_HOSA)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(DG_HOSA)) },
			{ &PROTOCOL_START(DG_HOSA)[0], ARRAY_LENGTH(PROTOCOL_START(DG_HOSA)) },
			{ &PROTOCOL_BIT0(DG_HOSA)[0], ARRAY_LENGTH(PROTOCOL_BIT0(DG_HOSA)) },
			{ &PROTOCOL_BIT1(DG_HOSA)[0], ARRAY_LENGTH(PROTOCOL_BIT1(DG_HOSA)) },
			{ NULL, 0 },
			24
		},
#endif
#if EFM8BB1_SUPPORT_Kaku_PROTOCOL == 1
		/*
		 * KaKu wall sockets
		 */
		{
			{ &PROTOCOL_BUCKETS(KaKu)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(KaKu)) },
			{ &PROTOCOL_START(KaKu)[0], ARRAY_LENGTH(PROTOCOL_START(KaKu)) },
			{ &PROTOCOL_BIT0(KaKu)[0], ARRAY_LENGTH(PROTOCOL_BIT0(KaKu)) },
			{ &PROTOCOL_BIT1(KaKu)[0], ARRAY_LENGTH(PROTOCOL_BIT1(KaKu)) },
			{ &PROTOCOL_END(KaKu)[0], ARRAY_LENGTH(PROTOCOL_END(KaKu)) },
			32
		},
#endif
#if EFM8BB1_SUPPORT_DIO_PROTOCOL == 1
		/*
		 * DIO CHACON
		 */
		{
			{ &PROTOCOL_BUCKETS(DIO_emg)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(DIO_emg)) },
			{ &PROTOCOL_START(DIO_emg)[0], ARRAY_LENGTH(PROTOCOL_START(DIO_emg)) },
			{ &PROTOCOL_BIT0(DIO_emg)[0], ARRAY_LENGTH(PROTOCOL_BIT0(DIO_emg)) },
			{ &PROTOCOL_BIT1(DIO_emg)[0], ARRAY_LENGTH(PROTOCOL_BIT1(DIO_emg)) },
			{ &PROTOCOL_END(DIO_emg)[0], ARRAY_LENGTH(PROTOCOL_END(DIO_emg)) },
			32
		},
#endif
#if EFM8BB1_SUPPORT_1BYONE_PROTOCOL == 1
		/*
		 * 1ByOne Doorbell
		 */
		{
			{ &PROTOCOL_BUCKETS(OneByOne)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(OneByOne)) },
			{ &PROTOCOL_START(OneByOne)[0], ARRAY_LENGTH(PROTOCOL_START(OneByOne)) },
			{ &PROTOCOL_BIT0(OneByOne)[0], ARRAY_LENGTH(PROTOCOL_BIT0(OneByOne)) },
			{ &PROTOCOL_BIT1(OneByOne)[0], ARRAY_LENGTH(PROTOCOL_BIT1(OneByOne)) },
			{ NULL, 0 },
			17
		},
#endif
#if EFM8BB1_SUPPORT_Prologue_PROTOCOL == 1
		/*
		 * Prologue Sensor
		 */
		{
			{ &PROTOCOL_BUCKETS(Prologue)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(Prologue)) },
			{ &PROTOCOL_START(Prologue)[0], ARRAY_LENGTH(PROTOCOL_START(Prologue)) },
			{ &PROTOCOL_BIT0(Prologue)[0], ARRAY_LENGTH(PROTOCOL_BIT0(Prologue)) },
			{ &PROTOCOL_BIT1(Prologue)[0], ARRAY_LENGTH(PROTOCOL_BIT1(Prologue)) },
			{ &PROTOCOL_END(Prologue)[0], ARRAY_LENGTH(PROTOCOL_END(Prologue)) },
			36
		},
#endif
#if EFM8BB1_SUPPORT_DOG_COLLAR_PROTOCOL == 1
		/*
		 * T-187-N (TX)-1 Generic Dog Training Collar Remote Control
		 */
		{
			{ &PROTOCOL_BUCKETS(DogCollar)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(DogCollar)) },
			{ &PROTOCOL_START(DogCollar)[0], ARRAY_LENGTH(PROTOCOL_START(DogCollar)) },
			{ &PROTOCOL_BIT0(DogCollar)[0], ARRAY_LENGTH(PROTOCOL_BIT0(DogCollar)) },
			{ &PROTOCOL_BIT1(DogCollar)[0], ARRAY_LENGTH(PROTOCOL_BIT1(DogCollar)) },
			{ &PROTOCOL_END(DogCollar)[0], ARRAY_LENGTH(PROTOCOL_END(DogCollar)) },
			40
		},
#endif
#if EFM8BB1_SUPPORT_BY302_PROTOCOL == 1
		/*
		 * Byron BY302 Doorbell
		 */
		{
			{ &PROTOCOL_BUCKETS(BY302)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(BY302)) },
			{ &PROTOCOL_START(BY302)[0], ARRAY_LENGTH(PROTOCOL_START(BY302)) },
			{ &PROTOCOL_BIT0(BY302)[0], ARRAY_LENGTH(PROTOCOL_BIT0(BY302)) },
			{ &PROTOCOL_BIT1(BY302)[0], ARRAY_LENGTH(PROTOCOL_BIT1(BY302)) },
			{ NULL, 0 },
			20
		},
#endif
#if EFM8BB1_SUPPORT_DT_5514_PROTOCOL == 1
		/*
		 * 5514 SILENT Dual Tech
		 */
		{
			{ &PROTOCOL_BUCKETS(DT_5514)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(DT_5514)) },
			{ &PROTOCOL_START(DT_5514)[0], ARRAY_LENGTH(PROTOCOL_START(DT_5514)) },
			{ &PROTOCOL_BIT0(DT_5514)[0], ARRAY_LENGTH(PROTOCOL_BIT0(DT_5514)) },
			{ &PROTOCOL_BIT1(DT_5514)[0], ARRAY_LENGTH(PROTOCOL_BIT1(DT_5514)) },
			{ NULL, 0 },
			39
		},
#endif
#if EFM8BB1_SUPPORT_H13726_PROTOCOL == 1
		/*
		 * Auriol H13726 Weather Station
		 */
		{
			{ &PROTOCOL_BUCKETS(H13726)[0], ARRAY_LENGTH(PROTOCOL_BUCKETS(H13726)) },
			{ &PROTOCOL_START(H13726)[0], ARRAY_LENGTH(PROTOCOL_START(H13726)) },
			{ &PROTOCOL_BIT0(H13726)[0], ARRAY_LENGTH(PROTOCOL_BIT0(H13726)) },
			{ &PROTOCOL_BIT1(H13726)[0], ARRAY_LENGTH(PROTOCOL_BIT1(H13726)) },
			{ NULL, 0 },
			36
		},
#endif
};

// https://www.ashn.dev/blog/2020-01-06-c-array-length.html
#define NUM_OF_PROTOCOLS (sizeof(PROTOCOL_DATA) / sizeof(PROTOCOL_DATA[0]))
#endif // INC_RF_PROTOCOLS_H_
