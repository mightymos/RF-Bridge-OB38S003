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


#ifndef __OB38S003_H__
#define __OB38S003_H__

// macros allow definitions with different compilers
#include <compiler.h>

/*  BYTE Registers  */

SFR(  P0, 0x80);
SFR(  P1, 0x90);
SFR(  P3, 0xB0);
SFR( PSW, 0xD0);
SFR( ACC, 0xE0);
SFR(   B, 0xF0);
SFR(  SP, 0x81);
SFR( DPL, 0x82);
SFR( DPH, 0x83);

SFR(  PCON, 0x87);    /*  SMOD,X,X,X,X,X,STOP,IDLE  */
SFR(  TCON, 0x88);
SFR(  TMOD, 0x89);
SFR(   TL0, 0x8A);
SFR(   TL1, 0x8B);
SFR(   TH0, 0x8C);
SFR(   TH1, 0x8D);
SFR( CKCON, 0x8E);
SFR(  SCON, 0x98);
SFR(  SBUF, 0x99);
SFR(    IE, 0xA8);    /*  Separate IEN0&IEN1 elsewhere  */
SFR(    IP, 0xA9);    /*  Separate IP0&IP1 elsewhere  */

/*  Dual DPTR control register  */
SFR( DPL1, 0x84);
SFR( DPH1, 0x85);
SFR(  AUX, 0x91);    /*  BRS,x,x,x,x,DPS  */

/*  ISP __sfr __at  */
SFR(  IFCON, 0x8F);    /*  X,CDPR,X,X,X,X,X,ISPE  */
SFR(  TAKEY, 0xF7);
SFR( ISPFAH, 0xE1);
SFR( ISPFAL, 0xE2);
SFR( ISPFDL, 0xE3);
SFR( ISPFDH, 0xEB);
SFR(  ISPFC, 0xE4);

/*  Enhance UART control register  */
SFR(  S0CON, 0x98);
SFR(  S0BUF, 0x99);
SFR( S0RELL, 0xAA);
SFR( S0RELH, 0xBA);
SFR(  SRELL, 0xAA);
SFR(  SRELH, 0xBA);
SFR(  PFCON, 0xD9);

/*  Enhance Interrupt control register  */
SFR(   IEN2, 0x9A);
SFR(   IEN0, 0xA8);
SFR(   IEN1, 0xB8);
SFR(    IP0, 0xA9);
SFR(    IP1, 0xB9);
SFR(  IRCON, 0xC0);
SFR( INTDEG, 0xEE);
SFR(  ENHIT, 0xE5);

/*  KBI  */
SFR( KBLS, 0x93);
SFR(  KBE, 0x94);
SFR(  KBF, 0x95);
SFR(  KBD, 0x96);

/*  IRCON2  */
SFR( IRCON2, 0x97);

/*  Reset rule  */
SFR(  RSTS, 0xA1);
SFR(   LVC, 0xE6);    /*  LVI_EN,X,LVRXE,X,X,X,X,X  */
SFR( SWRES, 0xE7);

/*  ADC  */
SFR( ADCC1, 0xAB);
SFR( ADCC2, 0xAC);
SFR( ADCDH, 0xAD);
SFR( ADCDL, 0xAE);
SFR( ADCCS, 0xAF);
SFR( ADCSH, 0xEF);
#define ADCCAL (*((unsigned char volatile xdata*)0xFFFF))

/*  PWM  */
SFR(   PWMC, 0xB5);
SFR( PWMD0H, 0xBC);
SFR( PWMD0L, 0xBD);
SFR( PWMD1H, 0xBE);
SFR( PWMD1L, 0xBF);
SFR( PWMD2H, 0xB1);
SFR( PWMD2L, 0xB2);
SFR( PWMD3H, 0xB3);
SFR( PWMD3L, 0xB4);
SFR( PWMMDH, 0xCE);
SFR( PWMMDL, 0xCF);

/*  WDT  */
SFR( WDTC, 0xB6);
SFR( WDTK, 0xB7);

/*  Timer2 Compare/Capture control register  */
SFR(  CCEN, 0xC1);
SFR( CCEN2, 0xD1);
SFR(  CCL1, 0xC2);
SFR(  CCH1, 0xC3);
SFR(  CCL2, 0xC4);
SFR(  CCH2, 0xC5);
SFR(  CCL3, 0xC6);
SFR(  CCH3, 0xC7);
SFR( T2CON, 0XC8);
SFR( CCCON, 0xC9);
SFR(  CRCL, 0xCA);
SFR(  CRCH, 0xCB);
SFR(   TL2, 0xCC);
SFR(   TH2, 0xCD);

/*  GPIO mode control register  */
SFR( P0M0, 0xD2);
SFR( P0M1, 0xD3);
SFR( P1M0, 0xD4);
SFR( P1M1, 0xD5);
SFR( P3M0, 0xDA);
SFR( P3M1, 0xDB);

/*  SPI  */
SFR(  SPIC1, 0xF1);
SFR(  SPIC2, 0xF2);
SFR( SPITXD, 0xF3);
SFR( SPIRXD, 0xF4);
SFR(  SPIS, 0xF5);

/*  IIC  */
SFR(   IICS, 0xF8);
SFR( IICCTL, 0xF9);
SFR(  IICA1, 0xFA);
SFR(  IICA2, 0xFB);
SFR( IICRWD, 0xFC);
SFR( IICEBT, 0xFD);

/*  Comparator  */
SFR(   OpPin, 0xF6);
SFR( Cmp0CON, 0xFE);
SFR( Cmp1CON, 0xFF);

/*  WORD/DWORD Registers  */

SFR16E ( CC1, 0xC3C2);
SFR16E ( CC2, 0xC5C4);
SFR16E ( CC3, 0xC7C6);
SFR16E ( CRC, 0xCBCA);
SFR16E (  T2, 0xCDCC);    // TH2 | TL2

/*  BIT Registers  */
SBIT( P0_0, 0x80, 0 ); /* Port 0 bit 0 */
SBIT( P0_1, 0x80, 1 ); /* Port 0 bit 1 */
SBIT( P0_2, 0x80, 2 ); /* Port 0 bit 2 */
SBIT( P0_3, 0x80, 3 ); /* Port 0 bit 3 */
SBIT( P0_4, 0x80, 4 ); /* Port 0 bit 4 */
SBIT( P0_5, 0x80, 5 ); /* Port 0 bit 5 */
SBIT( P0_6, 0x80, 6 ); /* Port 0 bit 6 */
SBIT( P0_7, 0x80, 7 ); /* Port 0 bit 7 */
SBIT( P1_0, 0x90, 0 ); /* Port 1 bit 0 */
SBIT( P1_1, 0x90, 1 ); /* Port 1 bit 1 */
SBIT( P1_2, 0x90, 2 ); /* Port 1 bit 2 */
SBIT( P1_3, 0x90, 3 ); /* Port 1 bit 3 */
SBIT( P1_4, 0x90, 4 ); /* Port 1 bit 4 */
SBIT( P1_5, 0x90, 5 ); /* Port 1 bit 5 */
SBIT( P1_6, 0x90, 6 ); /* Port 1 bit 6 */
SBIT( P1_7, 0x90, 7 ); /* Port 1 bit 7 */
SBIT( P3_0, 0xB0, 0 ); /* Port 3 bit 0 */
SBIT( P3_1, 0xB1, 1 ); /* Port 3 bit 1 */

/*  PSW  */
SBIT(   P, 0xD0, 0 ); /* PSW.0 - ACCUMULATOR PARITY FLAG */
SBIT(  F1, 0xD0, 1 ); /* PSW.1 - FLAG 1 */
SBIT(  OV, 0xD0, 2 ); /* PSW.2 - OVERFLOW FLAG */
SBIT( RS0, 0xD0, 3 ); /* PSW.3 - REGISTER BANK SELECT 0 */
SBIT( RS1, 0xD0, 4 ); /* PSW.4 - REGISTER BANK SELECT 1 */
SBIT(  F0, 0xD0, 5 ); /* PSW.5 - FLAG 0 */
SBIT(  AC, 0xD0, 6 ); /* PSW.6 - AUXILIARY CARRY FLAG */
SBIT(  CY, 0xD0, 7 ); /* PSW.7 - CARRY FLAG */

/*  TCON  */

SBIT( IT0, 0x88, 0 ); /* TCON.0 - EXT. INTERRUPT 0 TYPE */
SBIT( IE0, 0x88, 1 ); /* TCON.1 - EXT. INTERRUPT 0 EDGE FLAG */
SBIT( IT1, 0x88, 2 ); /* TCON.2 - EXT. INTERRUPT 1 TYPE */
SBIT( IE1, 0x88, 3 ); /* TCON.3 - EXT. INTERRUPT 1 EDGE FLAG */
SBIT( TR0, 0x88, 4 ); /* TCON.4 - TIMER 0 ON/OFF CONTROL */
SBIT( TF0, 0x88, 5 ); /* TCON.5 - TIMER 0 OVERFLOW FLAG */
SBIT( TR1, 0x88, 6 ); /* TCON.6 - TIMER 1 ON/OFF CONTROL */
SBIT( TF1, 0x88, 7 ); /* TCON.7 - TIMER 1 OVERFLOW FLAG */

/*  IE (IEN0 in datasheet)  */
SBIT( EX0, 0xA8, 0 );
SBIT( ET0, 0xA8, 1 );
SBIT( EX1, 0xA8, 2 );
SBIT( ET1, 0xA8, 3 );
SBIT(  ES, 0xA8, 4 );
SBIT( ET2, 0xA8, 5 );
SBIT(  EA, 0xA8, 7 );

/*  S0CON  */
SBIT(  RI, 0x98, 0 ); /* SCON.0 - RECEIVE INTERRUPT FLAG */
SBIT(  TI, 0x98, 1 ); /* SCON.1 - TRANSMIT INTERRUPT FLAG */
SBIT( RB8, 0x98, 2 ); /* SCON.2 - RECEIVE BIT 8 */
SBIT( TB8, 0x98, 3 ); /* SCON.3 - TRANSMIT BIT 8 */
SBIT( REN, 0x98, 4 ); /* SCON.4 - RECEIVE ENABLE */
SBIT( SM2, 0x98, 5 ); /* SCON.5 - MULTIPROCESSOR COMMUNICATION ENABLE */
SBIT( SM1, 0x98, 6 ); /* SCON.7 - SERIAL MODE CONTROL BIT 0 */
SBIT( SM0, 0x98, 7 ); /* SCON.7 - SERIAL MODE CONTROL BIT 0 */


/*  IICS  */
SBIT(   RW, 0xF8, 0);
SBIT(   BB, 0xF8, 0);
SBIT( TXAK, 0xF8, 1);
SBIT( RXAK, 0xF8, 2);
SBIT( TXIF, 0xF8, 3);
SBIT( RXIF, 0xF8, 4);
SBIT( LAIF, 0xF8, 5);
SBIT( MPIF, 0xF8, 6);


/*  IRCON  */
SBIT( PWMIF, 0xC0, 0);
SBIT( SPIIF, 0xC0, 1);
SBIT( ADCIF, 0xC0, 2);
SBIT( KBIIF, 0xC0, 3);
SBIT( LVIIF, 0xC0, 4);
SBIT( IICIF, 0xC0, 5);
SBIT(   TF2, 0xC0, 6);
SBIT(  EXF2, 0xC0, 7);


/*  IEN1  */
SBIT( IEPWM, 0xB8, 0);
SBIT( IESPI, 0xB8, 1);
SBIT( IEADC, 0xB8, 2);
SBIT( IEKBI, 0xB8, 3);
SBIT( IELVI, 0xB8, 4);
SBIT( IEIIC, 0xB8, 5);
SBIT( EXEN2, 0xB8, 7);


/*  Interrupt Define  */
#define d_INT0_Vector       0   // 0003h
#define d_T0_Vector         1   // 000Bh
#define d_INT1_Vector       2   // 0013h
#define d_T1_Vector         3   // 001Bh
#define d_UART0_Vector      4   // 0023h
#define d_T2_Vector         5   // 002Bh
#define d_PWM_Vector        8   // 0043h
#define d_SPI_Vector        9   // 004Bh
#define d_ADC_Vector        10  // 0053h
#define d_KBI_Vector        11  // 005Bh
#define d_LVI_Vector        12  // 0063h
#define d_IIC_Vector        13  // 006Bh
#define d_WDT_Vector        17  // 008Bh
#define d_Comparator_Vector 18  // 0093h

#endif
      