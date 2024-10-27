// based on test code posted by user PSLLSP
// https://github.com/grigorig/stcgal/issues/26


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
//-----------------------------------------
#if defined(TARGET_BOARD_OB38S003)
void tm0(void) __interrupt (d_T0_Vector)
#elif defined(TARGET_BOARD_EFM8BB1) || defined(TARGET_BOARD_EFM8BB1LCB)
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
                //save the data to RBUF
                RBUF = RDAT;
                //stop receive
                RING = 0;
                //set receive completed flag
                REND = 1;
            } else {
                RDAT >>= 1;
                
                //shift RX data to RX buffer
                // on some microcontrollers we do not have an extra receive pin available
                // so the hal will always return false and this will get optimized out by compiler
                if (get_soft_rx_pin())
                {
                    RDAT |= 0x80;
                }
            }
        }
    } else if (!get_soft_rx_pin()) {
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
                // send start bit
                //i.e., TXB = 0;
                soft_tx_pin_off();
                // load data from TBUF to TDAT
                TDAT = TBUF;
                // initial send bit number (8 data bits + 1 stop bit)
                TBIT = 9;
            } else {
                // shift data to CY
                TDAT >>= 1;

                if (--TBIT == 0)
                {
                    // i.e., TXB = 1;
                    soft_tx_pin_on();
                    // stop send
                    TING = 0;
                    // set send completed flag
                    TEND = 1;
                } else {
                    // write CY to TX port
                    // i.e., TXB = CY;
                    set_soft_tx_pin(CY);
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

#if 0
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

#endif