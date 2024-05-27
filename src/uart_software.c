// based on test code posted by user PSLLSP
// https://github.com/grigorig/stcgal/issues/26


#if defined(TARGET_BOARD_OB38S003)
#include "OB38S003.h"
#elif defined(TARGET_BOARD_EFM8BB1)
#include <stdint.h>
#include "EFM8BB1.h"
#endif


#include "hal.h"
#include "uart_software.h"

#define RCNT_RELOAD 3
#define TCNT_RELOAD 1

unsigned char TBUF,RBUF;

__bit TING,RING;
__bit TEND,REND;
unsigned char t, r;
unsigned char buf[8];

// internal
unsigned char TDAT,RDAT;
unsigned char TCNT,RCNT;
unsigned char TBIT,RBIT;

//-----------------------------------------
// timer interrupt routine for software UART
// FIXME: ob38s003 uses timer 0 interrupt
// efm8bb1  uses timer 3 interrupt
#if defined(TARGET_BOARD_OB38S003)
void tm0(void) __interrupt (d_T0_Vector)
#elif defined(TARGET_BOARD_EFM8BB1)
void tm0(void) __interrupt (TIMER0_VECTOR)
#endif
{    

    // for timer 0/1 no autoreload is available in 16 bit mode
	// however, timer 2/3 would have autoreload available
	load_timer0(SOFT_BAUD);

    
  if (RING)
  {
    if (--RCNT == 0)
	{
      //reset send baudrate counter
      RCNT = 3;
      if (--RBIT == 0)
	  {
        RBUF = RDAT; //save the data to RBUF
        RING = 0; //stop receive
        REND = 1; //set receive completed flag
      } else {
        RDAT >>= 1;
        if (RXB) RDAT |= 0x80; //shift RX data to RX buffer
      }
    }
  } else if (!RXB) {
    //set start receive flag
    RING = 1;
    //initial receive baudrate counter
    RCNT = 4;
    //initial receive bit number (8 data bits + 1 stop bit)
    RBIT = 9;
  }

  if (--TCNT == 0)
  {
    //reset send baudrate counter
    TCNT = TCNT_RELOAD;
    if (TING)
	{  //judge whether sending
      if (TBIT == 0)
	  {
        TXB = 0; //send start bit
        TDAT = TBUF; //load data from TBUF to TDAT
        TBIT = 9; //initial send bit number (8 data bits + 1 stop bit)
      } else {
		  TDAT >>= 1; //shift data to CY
		  
		  if (--TBIT == 0)
		  {
			TXB = 1;
			TING = 0; //stop send
			TEND = 1; //set send completed flag
		  } else {
			TXB = CY; //write CY to TX port
		  }
      }
    }
  }

	// timer 0/1 flags are automatically cleared on both controllers
	// if using timer 3 high overflow flag must be cleared
	//TMR3CN0 &= ~TF3H__SET;
}

//-----------------------------------------
//initial UART module variable
void init_software_uart(void)
{
    TING = 0;
    RING = 0;
    TEND = 1;
    REND = 0;
    TCNT = 0;
    RCNT = 0;
}

//-----------------------------------------
void putc(const char c)
{
    while (TEND==0);
    TBUF = c;
    TEND = 0;
    TING = 1;
}

//-----------------------------------------
// avoid puts() naming to avoid conflict with printf/stdio/putchar
void putstring(const char *s)
{
   while (*s) putc(*s++);
}

//-----------------------------------------
void puthex(unsigned char v)
{
   unsigned char c;
   v &= 0x0f;
   if (v<10) c = '0'+v;
   else c = 'A'-10+v;
   putc(c);
}

//-----------------------------------------
void puthex2(const unsigned char x)
{
   puthex(x >> 4);
   puthex(x);
}

//-----------------------------------------
unsigned char uart_rx(bool* result)
{
    volatile unsigned char rxByte = 0;
    *result = false;
    
    // provided in software UART example
    if (REND)
    {
        // this is shown in the original example
        // i think we need to access buffer, so index does not get confused
        // even though we basically just use rxByte to store a single byte
        buf[r++ & 0x08] = RBUF;
        REND = 0;
        rxByte = buf[(r-1) & 0x08];
        *result = true;
    }
    
    return rxByte;
}

//-----------------------------------------
void uart_loop_test(void)
{
    volatile unsigned char rxByte = 0;
    bool result;
    
    while(true)
    {
        rxByte = uart_rx(&result);
        
        if (result)
        {  
            // echo back character
            putc(rxByte);
        }
    }
}