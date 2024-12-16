/*
 *
 *  Ported on: 02.16.2023
 *      Author: Jonathan Armstrong
 */

#include "hal.h"
#include "OB38S003.h"


// pg. 3 of OB38S003 datasheet
// high speed architecture of 1 clock/machine cycle runs up to 16MHz.
void set_clock_mode(void)
{
    // default is 2T mode
    // ITS = 000 or 1T mode
    CKCON &= ~0x70;
}

// pg. 44 - once the watchdog is started it cannot be stopped
void enable_watchdog(void)
{
    // changes WDTC from read only to enable write
    TAKEY = 0x55;
    TAKEY = 0xAA;
    TAKEY = 0x5A;
    
    // sets WDTE bit
    //WDTC |= 0x20;
    
    // enable WDT and select time-out reset period is 3.2768s
    WDTC = 0x28;
}

void disable_watchdog(void)
{
    // changes WDTC from read only to enable write
    TAKEY = 0x55;
    TAKEY = 0xAA;
    TAKEY = 0x5A;
    
    // clear WDTE bit
    WDTC &= ~0x20;
}

void refresh_watchdog(void)
{
    // pg. 46 - Clear WDT timer to 0.
    WDTK = 0x55;
}

void reset_mcu(void)
{
    // 1.5.6 Example of software reset
    TAKEY = 0x55;
    TAKEY = 0xAA;
    // enable SWRES write attribute
    TAKEY = 0x5A;
    // software reset MCU
    SWRES = 0xFF;
}

void init_port_pins(void)
{
    // buzzer push pull
    P0M1 &= ~0x01;
    P0M0 |=  0x01;
    
    // radio transmit push pull
    P0M1 &= ~0x80;
    P0M0 |=  0x80;
    
    // uart tx push pull
    //P1M1 &= ~0x01;
    //P1M0 |=  0x01;
    
    // radio incoming input only
    //P1M1 |=  0x40;
    //P1M0 &= ~0x40;
    
    // software uart (transmit by reset pin)
    // default should be quasi output
    //P1M1 &= ~0x20;
    //P1M0 &= ~0x20;
    
    // rf sleep/wake push pull
    // see sonoffr22_pins.h - pin attached to upper leg of resistor divider
    P1M1 &= ~0x80;
    P1M0 |=  0x80;
    
    // led push pull
    P3M1 &= ~0x01;
    P3M0 |=  0x01;
    
}


void init_uart(void)
{
    // BRGS = 1 so uart baud rate generator uses SREL (instead of timer 1)
    AUX |= 0x80;
    
    // mode 1, no parity bit SM0 = 0 and SM2 = 0 by default for 8-bit uart
    SM1 = 1;
    
    // see formula below with SMOD = 1
    PCON |= 0x80;
    
    // SRELPS0 = 1 so prescaller is Fosc / 32 for eq. 8.4.1.2
    PFCON |= 0x10;
    
    // pg. 43, sec. 8.4.1.2
    // baud rate = (2^SMOD x Fosc) / ((32 or 64) * (2^10 - SREL))
    // SRELPS[1:0] = 00 divisor is 64, 01 divisor is 32
    // (2^1 * 16000000)/(32*(2^10 - 920)) = 9615
    // FIXME: consider computing with macro dependent on clock frequency
    //SRELH = 0x03;
    //SRELL = 0x98;
    
    // 19200
    SRELH = 0x03;
    SRELL = 0xcc;
}

uint8_t get_capture_flags(void)
{
    return CCCON;
}

void set_capture_flags(const uint8_t flags)
{
    CCCON = flags;
}

void init_timer0_8bit_autoreload(void)
{
    // 16-bit mode
    //TMOD |= 0x01;
    //  8-bit auto reload mode
    TMOD |= 0x02;
    
    // FIXME: this would not clear bits if other than reset value, so bad HAL practice
    // FIXME: T0PS bits = 0b01 so prescale is Fosc
    //PFCON |= 0x01;
}

void init_timer1_8bit_autoreload(void)
{
    // 16-bit mode
    //TMOD |= 0x10;
    //  8-bit auto reload mode
    TMOD |= 0x20;
    
    // FIXME: this would not clear bits if other than reset value, so bad HAL practice
    // T1PS prescaler Fosc
    // b01 = FOCS
    //PFCON |= 0x04;
    // b10 = FOCS / 96
    //PFCON |= 0x08;
}


//================================================================================
// datasheet states that using timer2 in capture mode is similar to programmable counter array (PCA) on other microcontrollers
//================================================================================
void init_timer2_as_capture(void)
{
    // capture on both rising and falling edge at pin CC1
    //(i.e., P1.6 or RDATA which is input from D0 of radio)
    CCEN  = 0x60;
    
    // FIXME: rcswitch appears to work with auto reload disabled why did we ever enable it in the first place?
    // timer 2 is clocked with 1/8 of the oscillator frequency (prescaler select bits)
    // timer 2 reload is disabled
    // timer 2 takes input frequency from prescaler
    // see rcswitch for examples of converting counts to time
    // but basically we want to choose a prescale value that results in simple math like dividing by two as a single right shift
    T2CON = 0x81;
}

// FIXME: this is not really called the PCA on this microcontroller
//        but for now we use the same naming convention
void pca0_run(void)
{
    // change to timer2 stop
    T2CON &= ~0x03;
    // then to input frequency from prescaler
    T2CON |= 0x01;
}

void pca0_halt(void)
{
    // clears T2I bits (i.e. timer 2 stop)
    T2CON &= ~0x03;
}


uint16_t get_capture_mode(void)
{
    // FIXME: what is the difference between CRCH/CRCL and CCHx or CCHx where x = 0,1, or 2?
    return (CCH1 << 8) | CCL1;
}

void clear_capture_flag(void)
{
    // clear CCF1 bit
    CCCON &= ~0x02;
}

// the time constant is explained in the rcswitch.c file
uint16_t countsToTime(const uint16_t counts)
{
    uint16_t duration;
    
    
    //duration = counts / 2;
    
    // this only saves two code bytes apparently
    // equivalent to divide by two
    duration = counts >> 1;
    
    return duration;
}
