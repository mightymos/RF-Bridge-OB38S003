#ifndef INC_OB38S003_H_
#define INC_OB38S003_H_

// SFRs for On-Bright OB38S003 8051 microcontroller

// FIXME: add notes from documentation
__sfr __at (0x8E) CKCON;

__sfr __at (0x9A) IEN2;

__sfr __at (0xA8) IEN0;

__sfr __at (0xD2) P0M0;
__sfr __at (0xD3) P0M1;
__sfr __at (0xD4) P1M0;
__sfr __at (0xD5) P1M1;
__sfr __at (0xD9) PFCON;
__sfr __at (0xDA) P3M0;
__sfr __at (0xDB) P3M1;
__sfr __at (0x91) AUX;
__sfr __at (0xAA) SRELL;
__sfr __at (0xBA) SRELH;
__sfr __at (0xB6) WDTC;
__sfr __at (0xB7) WDTK;


// Timer2 Compare/Capture control register
__sfr __at (0xC1) CCEN;
__sfr __at (0xD1) CCEN2;
__sfr __at (0xC2) CCL1;
__sfr __at (0xC3) CCH1;
__sfr __at (0xC4) CCL2;
__sfr __at (0xC5) CCH2;
__sfr __at (0xC6) CCL3;
__sfr __at (0xC7) CCH3;
__sfr __at (0xC8) T2CON;
__sfr __at (0xC9) CCCON;
__sfr __at (0xCA) CRCL;
__sfr __at (0xCB) CRCH;
__sfr __at (0xCC) TL2;
__sfr __at (0xCD) TH2;

__sfr __at (0xF7) TAKEY;

//sfr16 CC1   = 0xC2;
//sfr16 CC2   = 0xC4;
//sfr16 CC3   = 0xC6;
//sfr16 CRC   = 0xCA;
//sfr16 T2    = 0xCC;    // TH2 | TL2

#endif // INC_OB38S003_H_