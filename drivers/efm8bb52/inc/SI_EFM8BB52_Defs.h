//------------------------------------------------------------------------------
// Copyright 2014 Silicon Laboratories, Inc.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------
//Supported Devices:
//  EFM8BB52F16G_QFN20
//  EFM8BB52F16G_QFN32
//  EFM8BB52F16G_TSSOP20
//  EFM8BB52F16G_TSSOP28
//  EFM8BB52F16I_QFN20
//  EFM8BB52F16I_QFN32
//  EFM8BB52F16I_TSSOP20
//  EFM8BB52F16I_TSSOP28
//  EFM8BB52F32G_QFN20
//  EFM8BB52F32G_QFN32
//  EFM8BB52F32G_TSSOP20
//  EFM8BB52F32G_TSSOP28
//  EFM8BB52F32I_QFN20
//  EFM8BB52F32I_QFN32
//  EFM8BB52F32I_TSSOP20
//  EFM8BB52F32I_TSSOP28

#ifndef SI_EFM8BB52_DEFS_H
#define SI_EFM8BB52_DEFS_H

//#include <si_toolchain.h>
#include <compiler.h>

//-----------------------------------------------------------------------------
// Register Definitions
//-----------------------------------------------------------------------------
SFR (ACC,        0xE0); ///< Accumulator                              
SFR (ADC0ASAH,   0xB6); ///< ADC0 Autoscan Start Address High Byte    
SFR (ADC0ASAL,   0xB5); ///< ADC0 Autoscan Start Address Low Byte     
SFR (ADC0ASCF,   0xA1); ///< ADC0 Autoscan Configuration              
SFR (ADC0ASCT,   0xC7); ///< ADC0 Autoscan Output Count               
SFR (ADC0CF1,    0xB9); ///< ADC0 Configuration                       
SFR (ADC0CF2,    0xDF); ///< ADC0 Power Control                       
SFR (ADC0CN0,    0xE8); ///< ADC0 Control 0                           
SFR (ADC0CN1,    0xB2); ///< ADC0 Control 1                           
SFR (ADC0CN2,    0xB3); ///< ADC0 Control 2                           
SFR (ADC0GTH,    0xC4); ///< ADC0 Greater-Than High Byte              
SFR (ADC0GTL,    0xC3); ///< ADC0 Greater-Than Low Byte               
SFR (ADC0H,      0xBE); ///< ADC0 Data Word High Byte                 
SFR (ADC0L,      0xBD); ///< ADC0 Data Word Low Byte                  
SFR (ADC0LTH,    0xC6); ///< ADC0 Less-Than High Byte                 
SFR (ADC0LTL,    0xC5); ///< ADC0 Less-Than Low Byte                  
SFR (ADC0MX,     0xBB); ///< ADC0 Multiplexer Selection               
SFR (B,          0xF0); ///< B Register                               
SFR (CKCON0,     0x8E); ///< Clock Control 0                          
SFR (CKCON1,     0xA6); ///< Clock Control 1                          
SFR (CLEN0,      0xC6); ///< Configurable Logic Enable 0              
SFR (CLIE0,      0xC7); ///< Configurable Logic Interrupt Enable 0    
SFR (CLIF0,      0xE8); ///< Configurable Logic Interrupt Flag 0      
SFR (CLKGRP0,    0xAF); ///< Clock Group Control                      
SFR (CLKSEL,     0xA9); ///< Clock Select                             
SFR (CLOUT0,     0xD1); ///< Configurable Logic Output 0              
SFR (CLU0CF,     0xB1); ///< Configurable Logic Unit 0 Configuration  
SFR (CLU0FN,     0xAF); ///< Configurable Logic Unit 0 Function Select
SFR (CLU0MX,     0x84); ///< Configurable Logic Unit 0 Multiplexer    
SFR (CLU1CF,     0xB3); ///< Configurable Logic Unit 1 Configuration  
SFR (CLU1FN,     0xB2); ///< Configurable Logic Unit 1 Function Select
SFR (CLU1MX,     0x85); ///< Configurable Logic Unit 1 Multiplexer    
SFR (CLU2CF,     0xB6); ///< Configurable Logic Unit 2 Configuration  
SFR (CLU2FN,     0xB5); ///< Configurable Logic Unit 2 Function Select
SFR (CLU2MX,     0x91); ///< Configurable Logic Unit 2 Multiplexer    
SFR (CLU3CF,     0xBF); ///< Configurable Logic Unit 3 Configuration  
SFR (CLU3FN,     0xBE); ///< Configurable Logic Unit 3 Function Select
SFR (CLU3MX,     0xAE); ///< Configurable Logic Unit 3 Multiplexer    
SFR (CMP0CN0,    0x9B); ///< Comparator 0 Control 0                   
SFR (CMP0CN1,    0x99); ///< Comparator 0 Control 1                   
SFR (CMP0MD,     0x9D); ///< Comparator 0 Mode                        
SFR (CMP0MX,     0x9F); ///< Comparator 0 Multiplexer Selection       
SFR (CMP1CN0,    0xBF); ///< Comparator 1 Control 0                   
SFR (CMP1CN1,    0xAC); ///< Comparator 1 Control 1                   
SFR (CMP1MD,     0xAB); ///< Comparator 1 Mode                        
SFR (CMP1MX,     0xAA); ///< Comparator 1 Multiplexer Selection       
SFR (CP0CN,      0xA4); ///< Charge Pump Configuration                
SFR (CRC0CN0,    0xCE); ///< CRC0 Control 0                           
SFR (CRC0CN1,    0x86); ///< CRC0 Control 1                           
SFR (CRC0CNT,    0xD3); ///< CRC0 Automatic Flash Sector Count        
SFR (CRC0DAT,    0xCB); ///< CRC0 Data Output                         
SFR (CRC0FLIP,   0xCF); ///< CRC0 Bit Flip                            
SFR (CRC0IN,     0xCA); ///< CRC0 Data Input                          
SFR (CRC0ST,     0xD2); ///< CRC0 Automatic Flash Sector Start        
SFR (DAC0ALT,    0x8A); ///< Alternate Mode Low Byte                  
SFR (DAC0CF0,    0x91); ///< DAC0 Configuration 0                     
SFR (DAC0CF1,    0x92); ///< DAC0 Configuration 1                     
SFR (DAC0H,      0x85); ///< DAC0 Data Word High Byte                 
SFR (DAC0L,      0x84); ///< DAC0 Data Word Low Byte                  
SFR (DERIVID,    0xAD); ///< Derivative Identification                
SFR (DEVICEID,   0xB5); ///< Device Identification                    
SFR (DPH,        0x83); ///< Data Pointer High                        
SFR (DPL,        0x82); ///< Data Pointer Low                         
SFR (EIE1,       0xE6); ///< Extended Interrupt Enable 1              
SFR (EIE2,       0xF3); ///< Extended Interrupt Enable 2              
SFR (EIP1,       0xBB); ///< Extended Interrupt Priority 1 Low        
SFR (EIP1H,      0xEE); ///< Extended Interrupt Priority 1 High       
SFR (EIP2,       0xED); ///< Extended Interrupt Priority 2            
SFR (EIP2H,      0xF6); ///< Extended Interrupt Priority 2 High       
SFR (EMI0CN,     0xE7); ///< External Memory Interface Control        
SFR (FLKEY,      0xB7); ///< Flash Lock and Key                       
SFR (HFO0CAL,    0xD6); ///< High Frequency Oscillator 0 Calibration  
SFR (HFO0CN,     0xEF); ///< High Frequency Oscillator Control        
SFR (HFO0TRIM0,  0xCC); ///< High Frequency Oscillator Trim           
SFR (IE,         0xA8); ///< Interrupt Enable                         
SFR (IP,         0xB8); ///< Interrupt Priority                       
SFR (IPH,        0xF2); ///< Interrupt Priority High                  
SFR (IT01CF,     0xE4); ///< INT0/INT1 Configuration                  
SFR (LFO0CN,     0xB1); ///< Low Frequency Oscillator Control         
SFR (P0,         0x80); ///< Port 0 Pin Latch                         
SFR (P0MASK,     0xFE); ///< Port 0 Mask                              
SFR (P0MAT,      0xFD); ///< Port 0 Match                             
SFR (P0MDIN,     0xF1); ///< Port 0 Input Mode                        
SFR (P0MDOUT,    0xA4); ///< Port 0 Output Mode                       
SFR (P0SKIP,     0xD4); ///< Port 0 Skip                              
SFR (P1,         0x90); ///< Port 1 Pin Latch                         
SFR (P1MASK,     0xEE); ///< Port 1 Mask                              
SFR (P1MAT,      0xED); ///< Port 1 Match                             
SFR (P1MDIN,     0xF2); ///< Port 1 Input Mode                        
SFR (P1MDOUT,    0xA5); ///< Port 1 Output Mode                       
SFR (P1SKIP,     0xD5); ///< Port 1 Skip                              
SFR (P2,         0xA0); ///< Port 2 Pin Latch                         
SFR (P2MASK,     0xFC); ///< Port 2 Mask                              
SFR (P2MAT,      0xFB); ///< Port 2 Match                             
SFR (P2MDIN,     0xF3); ///< Port 2 Input Mode                        
SFR (P2MDOUT,    0xA6); ///< Port 2 Output Mode                       
SFR (P2SKIP,     0xCC); ///< Port 2 Skip                              
SFR (P3,         0xB0); ///< Port  Pin Latch                          
SFR (P3MDIN,     0xF4); ///< Port 3 Input Mode                        
SFR (P3MDOUT,    0x9C); ///< Port 3 Output Mode                       
SFR (PCA0CENT,   0x9E); ///< PCA Center Alignment Enable              
SFR (PCA0CLR,    0x9C); ///< PCA Comparator Clear Control             
SFR (PCA0CN0,    0xD8); ///< PCA Control                              
SFR (PCA0CPH0,   0xFC); ///< PCA Channel 0 Capture Module High Byte   
SFR (PCA0CPH1,   0xEA); ///< PCA Channel 1 Capture Module High Byte   
SFR (PCA0CPH2,   0xEC); ///< PCA Channel 2 Capture Module High Byte   
SFR (PCA0CPL0,   0xFB); ///< PCA Channel 0 Capture Module Low Byte    
SFR (PCA0CPL1,   0xE9); ///< PCA Channel 1 Capture Module Low Byte    
SFR (PCA0CPL2,   0xEB); ///< PCA Channel 2 Capture Module Low Byte    
SFR (PCA0CPM0,   0xDA); ///< PCA Channel 0 Capture/Compare Mode       
SFR (PCA0CPM1,   0xDB); ///< PCA Channel 1 Capture/Compare Mode       
SFR (PCA0CPM2,   0xDC); ///< PCA Channel 2 Capture/Compare Mode       
SFR (PCA0H,      0xFA); ///< PCA Counter/Timer High Byte              
SFR (PCA0L,      0xF9); ///< PCA Counter/Timer Low Byte               
SFR (PCA0MD,     0xD9); ///< PCA Mode                                 
SFR (PCA0POL,    0x96); ///< PCA Output Polarity                      
SFR (PCA0PWM,    0xF7); ///< PCA PWM Configuration                    
SFR (PCON0,      0x87); ///< Power Control 0                          
SFR (PCON1,      0xCD); ///< Power Control 1                          
SFR (PFE0CN,     0xC1); ///< Prefetch Engine Control                  
SFR (PRTDRV,     0xF6); ///< Port Drive Strength                      
SFR (PSCTL,      0x8F); ///< Program Store Control                    
SFR (PSTAT0,     0xAA); ///< PMU Status 0                             
SFR (PSW,        0xD0); ///< Program Status Word                      
SFR (PWMCFG0,    0xC2); ///< PWM Configuration 0                      
SFR (PWMCFG1,    0xC9); ///< PWM Configuration 1                      
SFR (PWMCFG2,    0xD1); ///< PWM Configuration 2                      
SFR (PWMCFG3,    0xDF); ///< PWM Configuration 3                      
SFR (PWMCKDIV,   0xE3); ///< PWM Clock Divider                        
SFR (PWMCPH0,    0x9A); ///< Ch0 Compare Value MSB                    
SFR (PWMCPH1,    0xB6); ///< Ch1 Compare Value MSB                    
SFR (PWMCPH2,    0xBE); ///< Ch2 Compare Value MSB                    
SFR (PWMCPL0,    0x99); ///< Ch0 Compare Value LSB                    
SFR (PWMCPL1,    0xB5); ///< Ch1 Compare Value LSB                    
SFR (PWMCPL2,    0xBD); ///< Ch2 Compare Value LSB                    
SFR (PWMCPUDH0,  0xAC); ///< Ch0 Compare Value Update MSB             
SFR (PWMCPUDH1,  0xB3); ///< Ch1 Compare Value Update MSB             
SFR (PWMCPUDH2,  0xBA); ///< Ch2 Compare Value Update MSB             
SFR (PWMCPUDL0,  0xAB); ///< Ch0 Compare Value Update LSB             
SFR (PWMCPUDL1,  0xB2); ///< Ch1 Compare Value Update LSB             
SFR (PWMCPUDL2,  0xB9); ///< Ch2 Compare Value Update LSB             
SFR (PWMDTINLIM, 0xE2); ///< DTI Negative Limit                       
SFR (PWMDTIPLIM, 0xE1); ///< DTI Positive Limit                       
SFR (PWMH,       0xC4); ///< PWM Counter MSB                          
SFR (PWMIE,      0x9F); ///< PWM Interrupt Enable                     
SFR (PWMIF,      0x9D); ///< PWM Interrupt Flags                      
SFR (PWML,       0xC3); ///< PWM Counter LSB                          
SFR (PWMLIMH,    0xC6); ///< PWM Counter Limit MSB                    
SFR (PWMLIML,    0xC5); ///< PWM Counter Limit LSB                    
SFR (PWMSTATUS,  0x9B); ///< PWM Status                               
SFR (REF0CN,     0xD1); ///< Voltage Reference Control                
SFR (REG0CN,     0xC9); ///< Regulator 0 Control                      
SFR (REVID,      0xB6); ///< Revision Identifcation                   
SFR (RSTSRC,     0xEF); ///< Reset Source                             
SFR (SBCON1,     0x94); ///< UART1 Baud Rate Generator Control        
SFR (SBRLH1,     0x96); ///< UART1 Baud Rate Generator High Byte      
SFR (SBRLL1,     0x95); ///< UART1 Baud Rate Generator Low Byte       
SFR (SBUF0,      0x99); ///< UART0 Serial Port Data Buffer            
SFR (SBUF1,      0x92); ///< UART1 Serial Port Data Buffer            
SFR (SCON0,      0x98); ///< UART0 Serial Port Control                
SFR (SCON1,      0xC8); ///< UART1 Serial Port Control                
SFR (SFRPAGE,    0xA7); ///< SFR Page                                 
SFR (SFRPGCN,    0xBC); ///< SFR Page Control                         
SFR (SFRSTACK,   0xD7); ///< SFR Page Stack                           
SFR (SMB0ADM,    0xD6); ///< SMBus 0 Slave Address Mask               
SFR (SMB0ADR,    0xD7); ///< SMBus 0 Slave Address                    
SFR (SMB0CF,     0xC1); ///< SMBus 0 Configuration                    
SFR (SMB0CN0,    0xC0); ///< SMBus 0 Control                          
SFR (SMB0DAT,    0xC2); ///< SMBus 0 Data                             
SFR (SMB0FCN0,   0xC3); ///< SMBus 0 FIFO Control 0                   
SFR (SMB0FCN1,   0xC4); ///< SMBus 0 FIFO Control 1                   
SFR (SMB0FCT,    0xEF); ///< SMBus 0 FIFO Count                       
SFR (SMB0RXLN,   0xC5); ///< SMBus 0 Receive Length Counter           
SFR (SMB0TC,     0xAC); ///< SMBus 0 Timing and Pin Control           
SFR (SMB1ADM,    0xFF); ///< SMBus 1 Slave Address Mask               
SFR (SMB1ADR,    0xBD); ///< SMBus 1 Slave Address                    
SFR (SMB1CF,     0xBA); ///< SMBus 1 Configuration                    
SFR (SMB1CN0,    0x88); ///< SMBus 1 Control                          
SFR (SMB1DAT,    0xBB); ///< SMBus 1 Data                             
SFR (SMB1FCN0,   0xAD); ///< SMBus 1 FIFO Control 0                   
SFR (SMB1FCN1,   0xAB); ///< SMBus 1 FIFO Control 1                   
SFR (SMB1FCT,    0xF5); ///< SMBus 1 FIFO Count                       
SFR (SMB1RXLN,   0xBC); ///< SMBus 1 Receive Length Counter           
SFR (SMB1TC,     0xB9); ///< SMBus 1 Timing and Pin Control           
SFR (SMOD1,      0x93); ///< UART1 Mode                               
SFR (SP,         0x81); ///< Stack Pointer                            
SFR (SPI0CFG,    0xA1); ///< SPI0 Configuration                       
SFR (SPI0CKR,    0xA2); ///< SPI0 Clock Rate                          
SFR (SPI0CN0,    0xF8); ///< SPI0 Control                             
SFR (SPI0DAT,    0xA3); ///< SPI0 Data                                
SFR (SPI0FCN0,   0x9A); ///< SPI0 FIFO Control 0                      
SFR (SPI0FCN1,   0x9B); ///< SPI0 FIFO Control 1                      
SFR (SPI0FCT,    0xF7); ///< SPI0 FIFO Count                          
SFR (SPI0PCF,    0xDF); ///< SPI0 Pin Configuration                   
SFR (TCON,       0x88); ///< Timer 0/1 Control                        
SFR (TH0,        0x8C); ///< Timer 0 High Byte                        
SFR (TH1,        0x8D); ///< Timer 1 High Byte                        
SFR (TL0,        0x8A); ///< Timer 0 Low Byte                         
SFR (TL1,        0x8B); ///< Timer 1 Low Byte                         
SFR (TMOD,       0x89); ///< Timer 0/1 Mode                           
SFR (TMR2CN0,    0xC8); ///< Timer 2 Control 0                        
SFR (TMR2CN1,    0xFD); ///< Timer 2 Control 1                        
SFR (TMR2H,      0xCF); ///< Timer 2 High Byte                        
SFR (TMR2L,      0xCE); ///< Timer 2 Low Byte                         
SFR (TMR2RLH,    0xCB); ///< Timer 2 Reload High Byte                 
SFR (TMR2RLL,    0xCA); ///< Timer 2 Reload Low Byte                  
SFR (TMR3CN0,    0x91); ///< Timer 3 Control 0                        
SFR (TMR3CN1,    0xFE); ///< Timer 3 Control 1                        
SFR (TMR3H,      0x95); ///< Timer 3 High Byte                        
SFR (TMR3L,      0x94); ///< Timer 3 Low Byte                         
SFR (TMR3RLH,    0x93); ///< Timer 3 Reload High Byte                 
SFR (TMR3RLL,    0x92); ///< Timer 3 Reload Low Byte                  
SFR (TMR4CN0,    0x98); ///< Timer 4 Control 0                        
SFR (TMR4CN1,    0xFF); ///< Timer 4 Control 1                        
SFR (TMR4H,      0xA5); ///< Timer 4 High Byte                        
SFR (TMR4L,      0xA4); ///< Timer 4 Low Byte                         
SFR (TMR4RLH,    0xA3); ///< Timer 4 Reload High Byte                 
SFR (TMR4RLL,    0xA2); ///< Timer 4 Reload Low Byte                  
SFR (TMR5CN0,    0xC0); ///< Timer 5 Control 0                        
SFR (TMR5CN1,    0xF1); ///< Timer 5 Control 1                        
SFR (TMR5H,      0xD5); ///< Timer 5 High Byte                        
SFR (TMR5L,      0xD4); ///< Timer 5 Low Byte                         
SFR (TMR5RLH,    0xD3); ///< Timer 5 Reload High Byte                 
SFR (TMR5RLL,    0xD2); ///< Timer 5 Reload Low Byte                  
SFR (UART0PCF,   0xD9); ///< UART0 Pin Configuration                  
SFR (UART1FCN0,  0x9D); ///< UART1 FIFO Control 0                     
SFR (UART1FCN1,  0xD8); ///< UART1 FIFO Control 1                     
SFR (UART1FCT,   0xFA); ///< UART1 FIFO Count                         
SFR (UART1LIN,   0x9E); ///< UART1 LIN Configuration                  
SFR (UART1PCF,   0xDA); ///< UART1 Pin Configuration                  
SFR (WDTCN,      0x97); ///< Watchdog Timer Control                   
SFR (XBR0,       0xE1); ///< Port I/O Crossbar 0                      
SFR (XBR1,       0xE2); ///< Port I/O Crossbar 1                      
SFR (XBR2,       0xE3); ///< Port I/O Crossbar 2                      

//------------------------------------------------------------------------------
// 16-bit Register Definitions (may not work on all compilers)
//------------------------------------------------------------------------------
SFR16 (ADC0ASA,  0xB5); ///< ADC0 Autoscan Start Address  
SFR16 (ADC0GT,   0xC3); ///< ADC0 Greater-Than            
SFR16 (ADC0,     0xBD); ///< ADC0 Data Word               
SFR16 (ADC0LT,   0xC5); ///< ADC0 Less-Than               
SFR16 (DP,       0x82); ///< Data Pointer                 
SFR16 (PCA0CP0,  0xFB); ///< PCA Channel 0 Capture Module 
SFR16 (PCA0CP1,  0xE9); ///< PCA Channel 1 Capture Module 
SFR16 (PCA0CP2,  0xEB); ///< PCA Channel 2 Capture Module 
SFR16 (PCA0,     0xF9); ///< PCA Counter/Timer            
SFR16 (PWMCP0,   0x99); ///< Ch0 Compare Value LSB        
SFR16 (PWMCP1,   0xB5); ///< Ch1 Compare Value LSB        
SFR16 (PWMCP2,   0xBD); ///< Ch2 Compare Value LSB        
SFR16 (PWMCPUD0, 0xAB); ///< Ch0 Compare Value Update LSB 
SFR16 (PWMCPUD1, 0xB2); ///< Ch1 Compare Value Update LSB 
SFR16 (PWMCPUD2, 0xB9); ///< Ch2 Compare Value Update LSB 
SFR16 (PWM,      0xC3); ///< PWM Counter LSB              
SFR16 (PWMLIM,   0xC5); ///< PWM Counter Limit LSB        
SFR16 (TMR2,     0xCE); ///< Timer 2                      
SFR16 (TMR2RL,   0xCA); ///< Timer 2 Reload               
SFR16 (TMR3,     0x94); ///< Timer 3                      
SFR16 (TMR3RL,   0x92); ///< Timer 3 Reload               
SFR16 (TMR4,     0xA4); ///< Timer 4                      
SFR16 (TMR4RL,   0xA2); ///< Timer 4 Reload               
SFR16 (TMR5,     0xD4); ///< Timer 5                      
SFR16 (TMR5RL,   0xD2); ///< Timer 5 Reload               

//------------------------------------------------------------------------------
// Indirect Register Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Bit Definitions
//------------------------------------------------------------------------------

// ACC (Accumulator)
#define SFR_ACC 0xE0
SBIT (ACC_ACC0, SFR_ACC, 0); ///< Accumulator Bit 0
SBIT (ACC_ACC1, SFR_ACC, 1); ///< Accumulator Bit 1
SBIT (ACC_ACC2, SFR_ACC, 2); ///< Accumulator Bit 2
SBIT (ACC_ACC3, SFR_ACC, 3); ///< Accumulator Bit 3
SBIT (ACC_ACC4, SFR_ACC, 4); ///< Accumulator Bit 4
SBIT (ACC_ACC5, SFR_ACC, 5); ///< Accumulator Bit 5
SBIT (ACC_ACC6, SFR_ACC, 6); ///< Accumulator Bit 6
SBIT (ACC_ACC7, SFR_ACC, 7); ///< Accumulator Bit 7

// ADC0CN0 (ADC0 Control 0)
#define SFR_ADC0CN0 0xE8
SBIT (ADC0CN0_TEMPE,  SFR_ADC0CN0, 0); ///< Temperature Sensor Enable         
SBIT (ADC0CN0_ADGN0,  SFR_ADC0CN0, 1); ///< Gain Control Bit 0                
SBIT (ADC0CN0_ADGN1,  SFR_ADC0CN0, 2); ///< Gain Control Bit 1                
SBIT (ADC0CN0_ADWINT, SFR_ADC0CN0, 3); ///< Window Compare Interrupt Flag     
SBIT (ADC0CN0_ADBUSY, SFR_ADC0CN0, 4); ///< ADC Busy                          
SBIT (ADC0CN0_ADINT,  SFR_ADC0CN0, 5); ///< Conversion Complete Interrupt Flag
SBIT (ADC0CN0_IPOEN,  SFR_ADC0CN0, 6); ///< Idle Powered-off Enable           
SBIT (ADC0CN0_ADEN,   SFR_ADC0CN0, 7); ///< ADC Enable                        

// B (B Register)
#define SFR_B 0xF0
SBIT (B_B0, SFR_B, 0); ///< B Register Bit 0
SBIT (B_B1, SFR_B, 1); ///< B Register Bit 1
SBIT (B_B2, SFR_B, 2); ///< B Register Bit 2
SBIT (B_B3, SFR_B, 3); ///< B Register Bit 3
SBIT (B_B4, SFR_B, 4); ///< B Register Bit 4
SBIT (B_B5, SFR_B, 5); ///< B Register Bit 5
SBIT (B_B6, SFR_B, 6); ///< B Register Bit 6
SBIT (B_B7, SFR_B, 7); ///< B Register Bit 7

// CLIF0 (Configurable Logic Interrupt Flag 0)
#define SFR_CLIF0 0xE8
SBIT (CLIF0_C0FIF, SFR_CLIF0, 0); ///< CLU0 Falling Edge Flag
SBIT (CLIF0_C0RIF, SFR_CLIF0, 1); ///< CLU0 Rising Edge Flag 
SBIT (CLIF0_C1FIF, SFR_CLIF0, 2); ///< CLU1 Falling Edge Flag
SBIT (CLIF0_C1RIF, SFR_CLIF0, 3); ///< CLU1 Rising Edge Flag 
SBIT (CLIF0_C2FIF, SFR_CLIF0, 4); ///< CLU2 Falling Edge Flag
SBIT (CLIF0_C2RIF, SFR_CLIF0, 5); ///< CLU2 Rising Edge Flag 
SBIT (CLIF0_C3FIF, SFR_CLIF0, 6); ///< CLU3 Falling Edge Flag
SBIT (CLIF0_C3RIF, SFR_CLIF0, 7); ///< CLU3 Rising Edge Flag 

// IE (Interrupt Enable)
#define SFR_IE 0xA8
SBIT (IE_EX0,   SFR_IE, 0); ///< External Interrupt 0 Enable
SBIT (IE_ET0,   SFR_IE, 1); ///< Timer 0 Interrupt Enable   
SBIT (IE_EX1,   SFR_IE, 2); ///< External Interrupt 1 Enable
SBIT (IE_ET1,   SFR_IE, 3); ///< Timer 1 Interrupt Enable   
SBIT (IE_ES0,   SFR_IE, 4); ///< UART0 Interrupt Enable     
SBIT (IE_ET2,   SFR_IE, 5); ///< Timer 2 Interrupt Enable   
SBIT (IE_ESPI0, SFR_IE, 6); ///< SPI0 Interrupt Enable      
SBIT (IE_EA,    SFR_IE, 7); ///< All Interrupts Enable      

// IP (Interrupt Priority)
#define SFR_IP 0xB8
SBIT (IP_PX0,   SFR_IP, 0); ///< External Interrupt 0 Priority Control LSB                        
SBIT (IP_PT0,   SFR_IP, 1); ///< Timer 0 Interrupt Priority Control LSB                           
SBIT (IP_PX1,   SFR_IP, 2); ///< External Interrupt 1 Priority Control LSB                        
SBIT (IP_PT1,   SFR_IP, 3); ///< Timer 1 Interrupt Priority Control LSB                           
SBIT (IP_PS0,   SFR_IP, 4); ///< UART0 Interrupt Priority Control LSB                             
SBIT (IP_PT2,   SFR_IP, 5); ///< Timer 2 Interrupt Priority Control LSB                           
SBIT (IP_PSPI0, SFR_IP, 6); ///< Serial Peripheral Interface (SPI0) Interrupt Priority Control LSB

// P0 (Port 0 Pin Latch)
#define SFR_P0 0x80
SBIT (P0_B0, SFR_P0, 0); ///< Port 0 Bit 0 Latch
SBIT (P0_B1, SFR_P0, 1); ///< Port 0 Bit 1 Latch
SBIT (P0_B2, SFR_P0, 2); ///< Port 0 Bit 2 Latch
SBIT (P0_B3, SFR_P0, 3); ///< Port 0 Bit 3 Latch
SBIT (P0_B4, SFR_P0, 4); ///< Port 0 Bit 4 Latch
SBIT (P0_B5, SFR_P0, 5); ///< Port 0 Bit 5 Latch
SBIT (P0_B6, SFR_P0, 6); ///< Port 0 Bit 6 Latch
SBIT (P0_B7, SFR_P0, 7); ///< Port 0 Bit 7 Latch

// P1 (Port 1 Pin Latch)
#define SFR_P1 0x90
SBIT (P1_B0, SFR_P1, 0); ///< Port 1 Bit 0 Latch
SBIT (P1_B1, SFR_P1, 1); ///< Port 1 Bit 1 Latch
SBIT (P1_B2, SFR_P1, 2); ///< Port 1 Bit 2 Latch
SBIT (P1_B3, SFR_P1, 3); ///< Port 1 Bit 3 Latch
SBIT (P1_B4, SFR_P1, 4); ///< Port 1 Bit 4 Latch
SBIT (P1_B5, SFR_P1, 5); ///< Port 1 Bit 5 Latch
SBIT (P1_B6, SFR_P1, 6); ///< Port 1 Bit 6 Latch
SBIT (P1_B7, SFR_P1, 7); ///< Port 1 Bit 7 Latch

// P2 (Port 2 Pin Latch)
#define SFR_P2 0xA0
SBIT (P2_B0, SFR_P2, 0); ///< Port 2 Bit 0 Latch
SBIT (P2_B1, SFR_P2, 1); ///< Port 2 Bit 1 Latch
SBIT (P2_B2, SFR_P2, 2); ///< Port 2 Bit 2 Latch
SBIT (P2_B3, SFR_P2, 3); ///< Port 2 Bit 3 Latch
SBIT (P2_B4, SFR_P2, 4); ///< Port 2 Bit 4 Latch
SBIT (P2_B5, SFR_P2, 5); ///< Port 2 Bit 5 Latch
SBIT (P2_B6, SFR_P2, 6); ///< Port 2 Bit 6 Latch
SBIT (P2_B7, SFR_P2, 7); ///< Port 2 Bit 7 Latch

// P3 (Port  Pin Latch)
#define SFR_P3 0xB0
SBIT (P3_B0, SFR_P3, 0); ///< Port 3 Bit 0 Latch
SBIT (P3_B1, SFR_P3, 1); ///< Port 3 Bit 1 Latch
SBIT (P3_B2, SFR_P3, 2); ///< Port 3 Bit 2 Latch
SBIT (P3_B3, SFR_P3, 3); ///< Port 3 Bit 3 Latch
SBIT (P3_B4, SFR_P3, 4); ///< Port 3 Bit 4 Latch
SBIT (P3_B5, SFR_P3, 5); ///< Port 3 Bit 5 Latch
SBIT (P3_B6, SFR_P3, 6); ///< Port 3 Bit 6 Latch
SBIT (P3_B7, SFR_P3, 7); ///< Port 3 Bit 7 Latch

// PCA0CN0 (PCA Control)
#define SFR_PCA0CN0 0xD8
SBIT (PCA0CN0_CCF0, SFR_PCA0CN0, 0); ///< PCA Module 0 Capture/Compare Flag
SBIT (PCA0CN0_CCF1, SFR_PCA0CN0, 1); ///< PCA Module 1 Capture/Compare Flag
SBIT (PCA0CN0_CCF2, SFR_PCA0CN0, 2); ///< PCA Module 2 Capture/Compare Flag
SBIT (PCA0CN0_CR,   SFR_PCA0CN0, 6); ///< PCA Counter/Timer Run Control    
SBIT (PCA0CN0_CF,   SFR_PCA0CN0, 7); ///< PCA Counter/Timer Overflow Flag  

// PSW (Program Status Word)
#define SFR_PSW 0xD0
SBIT (PSW_PARITY, SFR_PSW, 0); ///< Parity Flag               
SBIT (PSW_F1,     SFR_PSW, 1); ///< User Flag 1               
SBIT (PSW_OV,     SFR_PSW, 2); ///< Overflow Flag             
SBIT (PSW_RS0,    SFR_PSW, 3); ///< Register Bank Select Bit 0
SBIT (PSW_RS1,    SFR_PSW, 4); ///< Register Bank Select Bit 1
SBIT (PSW_F0,     SFR_PSW, 5); ///< User Flag 0               
SBIT (PSW_AC,     SFR_PSW, 6); ///< Auxiliary Carry Flag      
SBIT (PSW_CY,     SFR_PSW, 7); ///< Carry Flag                

// SCON0 (UART0 Serial Port Control)
#define SFR_SCON0 0x98
SBIT (SCON0_RI,    SFR_SCON0, 0); ///< Receive Interrupt Flag             
SBIT (SCON0_TI,    SFR_SCON0, 1); ///< Transmit Interrupt Flag            
SBIT (SCON0_RB8,   SFR_SCON0, 2); ///< Ninth Receive Bit                  
SBIT (SCON0_TB8,   SFR_SCON0, 3); ///< Ninth Transmission Bit             
SBIT (SCON0_REN,   SFR_SCON0, 4); ///< Receive Enable                     
SBIT (SCON0_MCE,   SFR_SCON0, 5); ///< Multiprocessor Communication Enable
SBIT (SCON0_SMODE, SFR_SCON0, 7); ///< Serial Port 0 Operation Mode       

// SCON1 (UART1 Serial Port Control)
#define SFR_SCON1 0xC8
SBIT (SCON1_RI,   SFR_SCON1, 0); ///< Receive Interrupt Flag   
SBIT (SCON1_TI,   SFR_SCON1, 1); ///< Transmit Interrupt Flag  
SBIT (SCON1_RBX,  SFR_SCON1, 2); ///< Extra Receive Bit        
SBIT (SCON1_TBX,  SFR_SCON1, 3); ///< Extra Transmission Bit   
SBIT (SCON1_REN,  SFR_SCON1, 4); ///< Receive Enable           
SBIT (SCON1_PERR, SFR_SCON1, 6); ///< Parity Error Flag        
SBIT (SCON1_OVR,  SFR_SCON1, 7); ///< Receive FIFO Overrun Flag

// SMB0CN0 (SMBus 0 Control)
#define SFR_SMB0CN0 0xC0
SBIT (SMB0CN0_SI,      SFR_SMB0CN0, 0); ///< SMBus Interrupt Flag            
SBIT (SMB0CN0_ACK,     SFR_SMB0CN0, 1); ///< SMBus Acknowledge               
SBIT (SMB0CN0_ARBLOST, SFR_SMB0CN0, 2); ///< SMBus Arbitration Lost Indicator
SBIT (SMB0CN0_ACKRQ,   SFR_SMB0CN0, 3); ///< SMBus Acknowledge Request       
SBIT (SMB0CN0_STO,     SFR_SMB0CN0, 4); ///< SMBus Stop Flag                 
SBIT (SMB0CN0_STA,     SFR_SMB0CN0, 5); ///< SMBus Start Flag                
SBIT (SMB0CN0_TXMODE,  SFR_SMB0CN0, 6); ///< SMBus Transmit Mode Indicator   
SBIT (SMB0CN0_MASTER,  SFR_SMB0CN0, 7); ///< SMBus Master/Slave Indicator    

// SMB1CN0 (SMBus 1 Control)
#define SFR_SMB1CN0 0x88
SBIT (SMB1CN0_SI,      SFR_SMB1CN0, 0); ///< SMBus Interrupt Flag            
SBIT (SMB1CN0_ACK,     SFR_SMB1CN0, 1); ///< SMBus Acknowledge               
SBIT (SMB1CN0_ARBLOST, SFR_SMB1CN0, 2); ///< SMBus Arbitration Lost Indicator
SBIT (SMB1CN0_ACKRQ,   SFR_SMB1CN0, 3); ///< SMBus Acknowledge Request       
SBIT (SMB1CN0_STO,     SFR_SMB1CN0, 4); ///< SMBus Stop Flag                 
SBIT (SMB1CN0_STA,     SFR_SMB1CN0, 5); ///< SMBus Start Flag                
SBIT (SMB1CN0_TXMODE,  SFR_SMB1CN0, 6); ///< SMBus Transmit Mode Indicator   
SBIT (SMB1CN0_MASTER,  SFR_SMB1CN0, 7); ///< SMBus Master/Slave Indicator    

// SPI0CN0 (SPI0 Control)
#define SFR_SPI0CN0 0xF8
SBIT (SPI0CN0_SPIEN,  SFR_SPI0CN0, 0); ///< SPI0 Enable            
SBIT (SPI0CN0_TXNF,   SFR_SPI0CN0, 1); ///< TX FIFO Not Full       
SBIT (SPI0CN0_NSSMD0, SFR_SPI0CN0, 2); ///< Slave Select Mode Bit 0
SBIT (SPI0CN0_NSSMD1, SFR_SPI0CN0, 3); ///< Slave Select Mode Bit 1
SBIT (SPI0CN0_RXOVRN, SFR_SPI0CN0, 4); ///< Receive Overrun Flag   
SBIT (SPI0CN0_MODF,   SFR_SPI0CN0, 5); ///< Mode Fault Flag        
SBIT (SPI0CN0_WCOL,   SFR_SPI0CN0, 6); ///< Write Collision Flag   
SBIT (SPI0CN0_SPIF,   SFR_SPI0CN0, 7); ///< SPI0 Interrupt Flag    

// TCON (Timer 0/1 Control)
#define SFR_TCON 0x88
SBIT (TCON_IT0, SFR_TCON, 0); ///< Interrupt 0 Type Select
SBIT (TCON_IE0, SFR_TCON, 1); ///< External Interrupt 0   
SBIT (TCON_IT1, SFR_TCON, 2); ///< Interrupt 1 Type Select
SBIT (TCON_IE1, SFR_TCON, 3); ///< External Interrupt 1   
SBIT (TCON_TR0, SFR_TCON, 4); ///< Timer 0 Run Control    
SBIT (TCON_TF0, SFR_TCON, 5); ///< Timer 0 Overflow Flag  
SBIT (TCON_TR1, SFR_TCON, 6); ///< Timer 1 Run Control    
SBIT (TCON_TF1, SFR_TCON, 7); ///< Timer 1 Overflow Flag  

// TMR2CN0 (Timer 2 Control 0)
#define SFR_TMR2CN0 0xC8
SBIT (TMR2CN0_T2XCLK0, SFR_TMR2CN0, 0); ///< Timer 2 External Clock Select Bit 0
SBIT (TMR2CN0_T2XCLK1, SFR_TMR2CN0, 1); ///< Timer 2 External Clock Select Bit 1
SBIT (TMR2CN0_TR2,     SFR_TMR2CN0, 2); ///< Timer 2 Run Control                
SBIT (TMR2CN0_T2SPLIT, SFR_TMR2CN0, 3); ///< Timer 2 Split Mode Enable          
SBIT (TMR2CN0_TF2CEN,  SFR_TMR2CN0, 4); ///< Timer 2 Capture Enable             
SBIT (TMR2CN0_TF2LEN,  SFR_TMR2CN0, 5); ///< Timer 2 Low Byte Interrupt Enable  
SBIT (TMR2CN0_TF2L,    SFR_TMR2CN0, 6); ///< Timer 2 Low Byte Overflow Flag     
SBIT (TMR2CN0_TF2H,    SFR_TMR2CN0, 7); ///< Timer 2 High Byte Overflow Flag    

// TMR4CN0 (Timer 4 Control 0)
#define SFR_TMR4CN0 0x98
SBIT (TMR4CN0_T4XCLK0, SFR_TMR4CN0, 0); ///< Timer 4 External Clock Select Bit 0
SBIT (TMR4CN0_T4XCLK1, SFR_TMR4CN0, 1); ///< Timer 4 External Clock Select Bit 1
SBIT (TMR4CN0_TR4,     SFR_TMR4CN0, 2); ///< Timer 4 Run Control                
SBIT (TMR4CN0_T4SPLIT, SFR_TMR4CN0, 3); ///< Timer 4 Split Mode Enable          
SBIT (TMR4CN0_TF4CEN,  SFR_TMR4CN0, 4); ///< Timer 4 Capture Enable             
SBIT (TMR4CN0_TF4LEN,  SFR_TMR4CN0, 5); ///< Timer 4 Low Byte Interrupt Enable  
SBIT (TMR4CN0_TF4L,    SFR_TMR4CN0, 6); ///< Timer 4 Low Byte Overflow Flag     
SBIT (TMR4CN0_TF4H,    SFR_TMR4CN0, 7); ///< Timer 4 High Byte Overflow Flag    

// TMR5CN0 (Timer 5 Control 0)
#define SFR_TMR5CN0 0xC0
SBIT (TMR5CN0_T5XCLK0, SFR_TMR5CN0, 0); ///< Timer 5 External Clock Select Bit 0
SBIT (TMR5CN0_T5XCLK1, SFR_TMR5CN0, 1); ///< Timer 5 External Clock Select Bit 1
SBIT (TMR5CN0_TR5,     SFR_TMR5CN0, 2); ///< Timer 5 Run Control                
SBIT (TMR5CN0_T5SPLIT, SFR_TMR5CN0, 3); ///< Timer 5 Split Mode Enable          
SBIT (TMR5CN0_TF5CEN,  SFR_TMR5CN0, 4); ///< Timer 5 Capture Enable             
SBIT (TMR5CN0_TF5LEN,  SFR_TMR5CN0, 5); ///< Timer 5 Low Byte Interrupt Enable  
SBIT (TMR5CN0_TF5L,    SFR_TMR5CN0, 6); ///< Timer 5 Low Byte Overflow Flag     
SBIT (TMR5CN0_TF5H,    SFR_TMR5CN0, 7); ///< Timer 5 High Byte Overflow Flag    

// UART1FCN1 (UART1 FIFO Control 1)
#define SFR_UART1FCN1 0xD8
SBIT (UART1FCN1_RIE,    SFR_UART1FCN1, 0); ///< Receive Interrupt Enable 
SBIT (UART1FCN1_RXTO0,  SFR_UART1FCN1, 1); ///< Receive Timeout Bit 0    
SBIT (UART1FCN1_RXTO1,  SFR_UART1FCN1, 2); ///< Receive Timeout Bit 1    
SBIT (UART1FCN1_RFRQ,   SFR_UART1FCN1, 3); ///< Receive FIFO Request     
SBIT (UART1FCN1_TIE,    SFR_UART1FCN1, 4); ///< Transmit Interrupt Enable
SBIT (UART1FCN1_TXHOLD, SFR_UART1FCN1, 5); ///< Transmit Hold            
SBIT (UART1FCN1_TXNF,   SFR_UART1FCN1, 6); ///< TX FIFO Not Full         
SBIT (UART1FCN1_TFRQ,   SFR_UART1FCN1, 7); ///< Transmit FIFO Request    

//------------------------------------------------------------------------------
// Interrupt Definitions
//------------------------------------------------------------------------------
#define INT0_IRQn    0  ///< External Interrupt 0      
#define TIMER0_IRQn  1  ///< Timer 0 Overflow          
#define INT1_IRQn    2  ///< External Interrupt 1      
#define TIMER1_IRQn  3  ///< Timer 1 Overflow          
#define UART0_IRQn   4  ///< UART0                     
#define TIMER2_IRQn  5  ///< Timer 2 Overflow / Capture
#define SPI0_IRQn    6  ///< SPI0                      
#define SMBUS0_IRQn  7  ///< SMBus 0                   
#define PMATCH_IRQn  8  ///< Port Match                
#define ADC0WC_IRQn  9  ///< ADC0 Window Compare       
#define ADC0EOC_IRQn 10 ///< ADC0 End of Conversion    
#define PCA0_IRQn    11 ///< PCA0                      
#define CMP0_IRQn    12 ///< Comparator 0              
#define CMP1_IRQn    13 ///< Comparator 1              
#define TIMER3_IRQn  14 ///< Timer 3 Overflow / Capture
#define UART1_IRQn   15 ///< UART1                     
#define SMBUS1_IRQn  16 ///< SMBus 1                   
#define TIMER4_IRQn  17 ///< Timer 4 Overflow / Capture
#define TIMER5_IRQn  18 ///< Timer 5 Overflow / Capture
#define CL0_IRQn     19 ///< Configurable Logic        
#define PWM_IRQn     20 ///< Pulse Width Modulation    

//------------------------------------------------------------------------------
// SFR Page Definitions
//------------------------------------------------------------------------------
#define LEGACY_PAGE 0x00 ///< Legacy SFR Page
#define PG2_PAGE    0x10 ///< Page2          
#define PG3_PAGE    0x20 ///< Page3          
#define PG4_PAGE    0x30 ///< Page4          

//-----------------------------------------------------------------------------
// SDCC PDATA External Memory Paging Support
//-----------------------------------------------------------------------------

#if defined __SDCC

SFR(_XPAGE, 0xE7); // Point to the EMI0CN register

#endif

#endif // SI_EFM8BB52_DEFS_H
//-eof--------------------------------------------------------------------------

