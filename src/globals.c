/*
 *
 *  Ported on: 02.16.2023
 *      Author: Jonathan Armstrong
 */
#include "..\inc\globals.h"

inline void led_on(void)
{
    LED = 1;
}

inline void led_off(void)
{
    LED = 0;
}

inline void led_toggle(void)
{
    LED = !LED;
}

inline void buzzer_on(void)
{
    BUZZER = 1;
}

inline void buzzer_off(void)
{
    BUZZER = 0;
}

inline void tdata_on(void)
{
    TDATA = 1;
}


inline void tdata_off(void)
{
    TDATA = 0;
}

inline void radio_off(void)
{
    RF_ENABLE = 1;
}


inline void radio_on(void)
{
    RF_ENABLE = 0;
}

inline bool is_rdata_low(void)
{
    return RDATA;
}