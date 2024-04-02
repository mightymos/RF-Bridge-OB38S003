// based on test code posted by user PSLLSP
// https://github.com/grigorig/stcgal/issues/26


#include "ob38s003.h"
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
//Timer interrupt routine for UART
void tm0(void) __interrupt (1)
{    

    // reload since no auto reload available on timer 0
    TH0 = (BAUD >> 8) & 0xff;
    TL0 = BAUD & 0xff;
    
  if (RING) {
    if (--RCNT == 0) {
      //reset send baudrate counter
      RCNT = 3;
      if (--RBIT == 0) {
        RBUF = RDAT; //save the data to RBUF
        RING = 0; //stop receive
        REND = 1; //set receive completed flag
      }
      else {
        RDAT >>= 1;
        if (RXB) RDAT |= 0x80; //shift RX data to RX buffer
      }
    }
  }
  else if (!RXB) {
    //set start receive flag
    RING = 1;
    //initial receive baudrate counter
    RCNT = 4;
    //initial receive bit number (8 data bits + 1 stop bit)
    RBIT = 9;
  }

  if (--TCNT == 0) {
    //reset send baudrate counter
    TCNT = TCNT_RELOAD;
    if (TING) {  //judge whether sending
      if (TBIT == 0) {
        TXB = 0; //send start bit
        TDAT = TBUF; //load data from TBUF to TDAT
        TBIT = 9; //initial send bit number (8 data bits + 1 stop bit)
      }
    else {
      TDAT >>= 1; //shift data to CY
      if (--TBIT == 0) {
        TXB = 1;
        TING = 0; //stop send
        TEND = 1; //set send completed flag
      }
      else {
        TXB = CY; //write CY to TX port
      }
    }
  }
}
}

// FIXME: these names and functionality could be confusing, if we switch over to using timer0 as tick function
void enable_timer0(void)
{
    //timer0 in 16-bit mode
    //(ob38s003 only supports autoreload on timer2, so must reload manually within interrupt)
    TMOD |= T0_M0;
    
    // FIXME: we set this elsewhere in main()
    // but need to confirm this is okay I guess
    //timer0 working at 1T mode    
    //AUXR = 0x80;
    //set_clock_1t_mode();
    //CKCON = 0x00;
    
    // prescaler is FOSC so prescaler is 1
    // FIXME: this would not necessarily clear upper bit so bad HAL practice
    PFCON |= 0x01;
    
    //initial timer0
    TH0 = (BAUD >> 8) & 0xff;
    TL0 = BAUD & 0xff;

    //enable timer0 interrupt
    //IE1  |= M_ET0;
    ET0 = true;

    //timer0 start running
    TR0 = true;
    
    // FIXME: does processor have priorities?
    //improve timer0 interrupt priority
    //IP1H |= M_PT0;
}

void disable_timer0(void)
{
    //timer0 in 16-bit auto reload mode
    //TMOD = 0x00;
    
    // FIXME: is it necessary or make sense to undo clock setting mode
    //timer0 working at 12T mode    
    //AUXR &= ~0x80;
    
    //timer0 stop running
    TR0 = 0;
    
    // clear overflow flag
    TF0 = 0;
    
    //initialize timer0 and set reload value
    TL0 = 0x00;
    TH0 = 0x00;
    
    //disable timer0 interrupt
    //IE1  &= ~M_ET0;
    ET0 = 0;
    
    //clear timer0 interrupt priority
    //IP1H &= ~M_PT0;
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

//void puts(const char *s)
//{
//   while (*s) putc(*s++);
//}

void puthex(unsigned char v)
{
   unsigned char c;
   v &= 0x0f;
   if (v<10) c = '0'+v;
   else c = 'A'-10+v;
   putc(c);
}

void puthex2(const unsigned char x)
{
   puthex(x >> 4);
   puthex(x);
}

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