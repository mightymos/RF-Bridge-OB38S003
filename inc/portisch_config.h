/*
 * RF_Config.h
 *
 *  Created on: 19.02.2019
 *      Author: Portisch
 *
 *  Enable/Disable the protocols you want to include when compiling
 *  The memory of the used EFM8BB1 is limited to 8KB
 *
 */


// include functions and set buffer sizes to support sniffing of unknown protocol timings (comment to exclude)
// we would often exclude to allow room for more protocols in flash memory

#if defined(BUCKET_SNIFFING_INCLUDED)
    // if bucket sniffing is included, we cannot fit many protocols in memory so we just allow the standard one
    // PT2260, EV1527,... original RF bridge protocol
    #define PORTISCH_SUPPORT_PT226X_PROTOCOL

#elif defined(MULTIPLE_PROTOCOLS_INCLUDED)
    
    // typical protocols, comment to disable
    // PT2260, EV1527,... original RF bridge protocol
    #define PORTISCH_SUPPORT_PT226X_PROTOCOL
    // HT6P20X chips
    #define PORTISCH_SUPPORT_HT6P20X_PROTOCOL
    // HT12A/HT12E chips
    #define PORTISCH_SUPPORT_HT12_PROTOCOL

    // more protocols, uncomment to enable
    // Rohrmotor24
    //#define PORTISCH_SUPPORT_Rohrmotor24_PROTOCOL
    // UNDERWATER PAR56 LED LAMP, 502266
    //#define PORTISCH_SUPPORT_PAR56_PROTOCOL
    // Alecto WS-1200 Series Wireless Weather Station	
    //#define PORTISCH_SUPPORT_WS_1200_PROTOCOL
    // ALDI Remote controlled wall sockets, 4x
    #define PORTISCH_SUPPORT_ALDI_4x_PROTOCOL
    // Meteo SPxx -  Weather station (PHU Metrex)	
    #define PORTISCH_SUPPORT_SP45_PROTOCOL
    // Dooya DC90 remote    
    #define PORTISCH_SUPPORT_DC90_PROTOCOL
    // Digoo DG-HOSA Smart 433MHz Wireless Household Carbon Monoxide Sensor
    #define PORTISCH_SUPPORT_DG_HOSA_PROTOCOL
    // HT12A/HT12E chips - Generic Doorbell
    //#define PORTISCH_SUPPORT_HT12a_PROTOCOL
    // HT12A/HT12E chips - Atag Extractor - Plus/Minus/Lights/Timer
    //#define PORTISCH_SUPPORT_HT12_Atag_PROTOCOL
    // KaKu wall sockets
    //#define PORTISCH_SUPPORT_Kaku_PROTOCOL
    // DIO Chacon RF 433Mhz, Issue #95
    //#define PORTISCH_SUPPORT_DIO_PROTOCOL
    // 1ByOne Doorbell, PR #97
    //#define PORTISCH_SUPPORT_1BYONE_PROTOCOL
    // Prologue Sensor, Issue #96
    //#define PORTISCH_SUPPORT_Prologue_PROTOCOL
    // Generic dog training collar - board label T-187-n (TX)-1, PR #100
    //#define PORTISCH_SUPPORT_DOG_COLLAR_PROTOCOL
    // Byron BY302 Doorbell, Issue #102
    //#define PORTISCH_SUPPORT_BY302_PROTOCOL
    // 5514 SILENT Dual Tech, Issue #104
    //#define PORTISCH_SUPPORT_DT_5514_PROTOCOL
    // Auriol H13726 Weather Station, Issue #106
    //#define PORTISCH_SUPPORT_H13726_PROTOCOL
    
#else
    // error check
    #error Please defined either BUCKET_SNIFFING_INCLUDED or MULTIPLE_PROTOCOLS_INCLUDED in Makefile.

#endif