// Copyright 2018 jem@seethis.link
// Licensed under the MIT license (http://opensource.org/licenses/MIT)

#include "delay.h"

#define F_CPU 24500000ULL

#if !defined(F_CPU)
	#error Please define F_CPU
#endif

// do not care too much if this isn't accurate
// FIXME: how inaccurate are these?
void delay_us(uint16_t us) {
    while (us--) {
		__asm
#if F_CPU >=  8000000ULL
			nop
			nop
#endif
#if F_CPU >= 16000000ULL
			nop
			nop
#endif
#if F_CPU >= 24000000ULL
			nop
			nop
			nop
			nop
#endif
#if F_CPU >= 32000000ULL
			nop
			nop
			nop
			nop
#endif
#if F_CPU >= 40000000ULL
			nop
			nop
			nop
			nop
#endif
#if F_CPU >= 48000000ULL
			nop
			nop
			nop
			nop
#endif
		__endasm;
    }
}

void delay1ms(uint16_t ms) {
    while (ms--) {
        delay_us(1000);
    }
}
