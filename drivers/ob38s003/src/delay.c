/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2024 Jonathan Armstrong. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "delay.h"


//#define F_CPU 16000000ULL

// if we use delay style of efm8bb1
//#if !defined(F_CPU)
//  #error Please define F_CPU
//#endif

void delay_us(uint8_t us)
{
    // these assume 1T mode @ 16 MHz on OB38S003 microcontroller
    // in other words, they were tuned by observing oscilloscope traces
    // and are therefore useless with other clock modes or with another microcontroller
    
    // tuned with oscilloscope for 10 microseconds
    //unsigned int i,j;
    //for(i = 0; i < us; i++)
    //{
    //    for(j = 0; j < 11; j++);
    //}
    
    //   1us (observed  10us)
    //  10us (observed  15us)
    //  50us (observed  70us)
    // 100us (observed 150us)
    while (us--) {
        //__asm
        //  nop
        //__endasm;
    }
}

void delay1ms(const uint16_t msCount)
{
    uint16_t i, j;
    for(i = 0; i < msCount; i++)
    {
        for(j = 0; j < 1429; j++);
    }
}



//-----------------------------------------------------------
//-- END OF FILE --------------------------------------------