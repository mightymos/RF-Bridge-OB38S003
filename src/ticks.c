#include "hal.h"
//#include "timer.h"

#include <limits.h>

unsigned long get_current_timer0(void)
{
    unsigned long currentTime;
    
    // FIXME: disable timer0 interrupt for atomic reading of variable
    //        consider check to see if interrupt was enabled in the first place
    disable_timer0_interrupt();
    
    // FIXME: compute the proper conversion from counts to milliseconds
    //currentTime = gTimeMilliseconds;
	currentTime = get_time_milliseconds();
    
    // re-enable timer0 interrupt
    enable_timer0_interrupt();
    
    return currentTime;
}


unsigned long get_elapsed_timer0(unsigned long previousTime)
{
    unsigned long currentTime;
    unsigned long elapsed;
    
    currentTime = get_current_timer0();
        
    // handle typical versus wraparound condition
    if (previousTime <= currentTime)
    {
        elapsed = currentTime - previousTime;
    } else {
		// FIXME: seems like a bad idea to hardcode unsigned long maximum because what if type changes
        elapsed = ULONG_MAX - previousTime + currentTime;
    }
    
    return elapsed;
}

unsigned long get_current_timer1(void)
{
    unsigned long currentTime;
    

    disable_timer1_interrupt();
    
    // FIXME: compute the proper conversion from counts to microseconds
    //currentTime = gTimeTenMicroseconds;
	currentTime = get_time_ten_microseconds();
    
    enable_timer1_interrupt();
    
    return currentTime;
}


unsigned long get_elapsed_timer1(unsigned long previousTime)
{
    unsigned long currentTime;
    unsigned long elapsed;
    
    currentTime = get_current_timer1();
    
    //printf("currentTime: %lu\r\n", currentTime);
    
    // handle typical versus wraparound condition
    if (previousTime <= currentTime)
    {
        elapsed = currentTime - previousTime;
    } else {
		// FIXME: seems like a bad idea to hardcode unsigned long maximum because what if type changes
        elapsed = ULONG_MAX - previousTime + currentTime;
    }
    
    return elapsed;
}
