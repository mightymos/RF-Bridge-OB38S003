#include <stdint.h>

#include "..\inc\globals.h"
#include "..\inc\timer.h"

uint16_t gTimer2Timeout;
uint16_t gTimer2Interval;
//uint16_t gTimer3Timeout;
//uint16_t gTimer3Interval;

volatile bool gCaptureDone = false;

__xdata volatile uint16_t gDuration[MAX_COUNT];
volatile uint8_t gCount;

//-----------------------------------------------------------------------------
// time 2 is associated with the programmable counter array (PCA) - pg. 9 and 32 of OB38S003 datasheet
//-----------------------------------------------------------------------------
void timer2_isr(void) __interrupt (5)
{
    static uint8_t lowByteOld  = 0;
    static uint8_t highByteOld = 0;
    static uint8_t lowByteNew  = 0;
    static uint8_t highByteNew = 0;
    
    static volatile uint16_t previous = 0;
    static volatile uint16_t current = 0;
    uint16_t diff;

    
    lowByteOld  = lowByteNew;
    highByteOld = highByteNew;
    
    lowByteNew  = CCL1;
    highByteNew = CCH1;
    
    previous = current;
    current = (highByteNew << 8) | lowByteNew;
    
    diff = current - previous;
    
    // with timer clocked at (16 MHz / 4), four counts are needed to get one microsecond
    diff = diff >> 2;
    
    if (diff > 20000)
    {
        gCount = 0;
    }
    
    if (gCount >= MAX_COUNT)
    {
        gCount = 0;
    }
    
    gDuration[gCount++] = diff;
    
    // check in main loop
    gCaptureDone = true;
    
    // clear timer 2 external reload flag
    EXF2 = 0;
    
    //clear compare/capture 1 flag
    CCCON &= ~0x02;
}