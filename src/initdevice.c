//=========================================================
//=========================================================

// USER INCLUDES
//#include <SI_EFM8BB1_Register_Enums.h>
#include <8052.h>
#include <stdint.h>
#include "..\inc\globals.h"
#include "..\inc\initdevice.h"


void init_port_pins(void)
{
    // buzzer push pull
    P0M1 &= ~0x01;
    P0M0 |=  0x01;
    
    // uart tx push pull
    //P1M1 &= ~0x01;
    //P1M0 |=  0x01;
    
    // radio incoming input only
    P1M1 |=  0x40;
    P1M1 &= ~0x40;
    
    // rf sleep/wake push pull
    P1M1 &= ~0x10;
    P1M1 |=  0x10;
    
    // led push pull
    P3M1 &= ~0x01;
    P3M0 |=  0x01;
    
}


void init_uart(void)
{
    // BRGS = 1 so uart baud rate generator uses SREL (instead of timer 1)
    AUX |= 0x80;
    
    // mode 1, no parity bit SM0 = 0 and SM2 = 0 by default
    SM1 = 1;
    
	// uart reception enabled
    REN = 1;
    
    // see formula below with SMOD = 1
    PCON |= SMOD;
    
    // SRELPS0 = 1;
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


//================================================================================
//================================================================================
void init_interrupts(void)
{
    // enable serial interrupt
    ES = 1;
    
    // enable comparator interrupt
    //IEN2 |= 0x04;
    
    // enable timer 2 reload interrupt enable
    EXEN2 = 1;
    
    // global enable interrupts
    EA = 1;

}


//================================================================================
//================================================================================
void init_timer2_capture(void)
{
    // capture on both rising and falling edge at pin CC1
    //(i.e., P1.6 or RDATA which is input from D0 of radio)
    CCEN  = 0x60;
    
    // time 2 input frequency from prescaler
    // timer 2 mode 0 auto reload/
    // timer 2 is clocked with 1/4 of the oscillator frequency (prescaler select bit)
    // (e.g., 0.25 microseconds per count)
    T2CON = 0x51;
    
    // compare/capture 1 interrupt control bit
    CCCON = 0x20;
}

void enable_interrupts(void)
{
    EA = 1;
}

void disable_interrupts(void)
{
    EA = 0;
}