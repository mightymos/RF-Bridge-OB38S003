# Introduction

An alternative firmware for 433 MHz radio to wifi bridges, targetting these boards/microcontrollers:  
This microcontroller is present in the  433 MHz radio to wifi bridge.  

| Board | Microcontroller | Passthrough |  RCSwitch | Portisch |
| ------------- | ------------- | ------------- | ------------- | ------------- |
| Sonoff Bridge R2 v1.0 (black box) | OB38S003 | Supported | Decode only | No |
| Sonoff Bridge R2 v2.2 (white box) | EFM8BB1 | Supported | Decode only | No |
| EFM8BB1 Busy Bee Low Cost Kit Board | EFM8BB1 | Supported | Decode only | No |

These microcontrollers were originally intended for radio decoding but lacked support for additional protocols.  
As a consequence many people just bypass the microcontroller with a hardware modification:  
https://github.com/arendst/Tasmota/discussions/13283  

Decoding may then be done directly on the ESP8265 (e.g., ESPHome or Tasmota or typically used for this purpose).  


The intent here is to avoid the need to perform hardware modification.  
This can be accomplished in two ways:  

1) microcontroller mirrors the voltage levels on the radio data pins over to uart pins (used as gpio) already connected to ESP8265.  
   (this essentially bypasses the microcontroller but using software instead of hardware modification)  
   
2) microcontroller decodes radio packets and sends formatted to ESP8265 over uart/serial  
   (similar to factory firmware but with ability for source code protocols to be added, behavior modified, etc.)  
   
A benefit to keeping the microcontroller involved is that the radio LED and buzzer may still be controlled.  
The downside is the effort required to develop firmware and flash by the end user.  

The firmware radio decoding is inspired by 1) RF-Bridge-EFM8BB1 (Portisch) and 2) rc-switch projects.  

# Status
THIS IS A WORK IN PROGRESS and should not be used by typical users.  

Erasing and reprogramming the microcontroller is cumbersome without the official programmer.  
However, an open source and inexpensive programmer is discussed under Flasher section.  

# Installation
Install SDCC compiler for your platform:  
https://sdcc.sourceforge.net/  

On command line run make.  
For now modify Makefile to select desired target.  
Built firmware placed in build directory.  
See Flasher section below.  

# Previous Work

An successful attempt was made to compile "Portisch" with the open source SDCC compiler.  
I personally found the source code to be difficult to read.  
I hope to be able to use this work to use Portisch on the newer Sonoff boxes.  
https://github.com/mightymos/SonOfPortisch

There are at least three versions of rcswitch.  
We attempt to use the simplest and most understandable for now (from sui77).  
https://github.com/sui77/rc-switch  
https://github.com/1technophile/rc-switch  
https://github.com/arendst/Tasmota/tree/development/lib/lib_rf/rc-switch  
 

# Flasher
Reprogramming the OB38S003 requires erasing the chip because the stock firmware is protected.  
This means that, once erased, the stock firmware cannot be recovered because it has not been read out.  
A MSM9066 programmer costs $30 USD on AliExpress as of 7/14/2023 and $27 on eBay as of 11/20/2023.  

# Flasher (alternative)
Logic analyzer decodings of several flasher operations/programming cycles have been captured:  
https://github.com/mightymos/msm9066_capture  

An Arduino based flasher written for the built in ESP8265/ESP8266/ESP32 is available:  
https://github.com/mightymos/OnbrightFlasher

While still cumbersome to use, several people have successfully flashed short hex files manually and by script.

# Special Thanks
We thank Vincent Defert for the use of modified makefiles and familiarity with 8051/8052 based microcontrollers:  
https://github.com/area-8051/uni-STC
