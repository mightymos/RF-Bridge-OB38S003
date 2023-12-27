# Introduction

This is an alternative firmware targetting the 8052 based On-Bright OB38S003 microcontroller.  
This microcontroller is present in the Sonoff Bridge R2 v2.2 433 MHz radio to wifi bridge.  


The OB38S003 was originally intended for radio decoding but lacked support for additional protocols.  
As a consequence many people just bypass the microcontroller with a hardware modification:  
https://github.com/arendst/Tasmota/discussions/13283  

Decoding may then be done directly on the ESP8265 (e.g., ESPHome supports rcswitch).  


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
THIS IS A WORK IN PROGRESS and should not be used by typical users (yet).  

That said, decoding of protocols 1 and 2 seems to work reliably.  
For example door reed sensors - both stock and modified - have been successfully decoded.  
https://github.com/mightymos/ReedTripRadio  

| Feature | detail | status |
| ------------- | ------------- | ------------- |
| Protocol testing | we need some scheme to evaluate reliability | TODO |
| Sniffing | allow viewing timing of unknown radio protocols | TODO |
| Transmission | operate as remote control | TODO |
| Port to R2 v1.0 bridge (black box) | requires makefile | TODO |

# Installation
Install SDCC compiler for your platform:  
https://sdcc.sourceforge.net/  

On command line run make.  
Built firmware placed in build directory.  
See flasher section below (requires msm9066 flasher for now).

# Previous Work

An attempt was made to compile portisch with the open source SDCC compiler.  
8KB code space appeared to be exceeded.  
I personally found the source code to be difficult to read.  
https://github.com/mightymos/SonOfPortisch

There are at least three versions of rcswitch.  
We attempt to use the simplest and most understandable for now (from sui77).  
https://github.com/sui77/rc-switch  
https://github.com/1technophile/rc-switch  
https://github.com/arendst/Tasmota/tree/development/lib/lib_rf/rc-switch  
 

# Flasher
Reprogramming the OB38S003 requires erasing the chip because the stock firmware is protected.  
This means that, once erased, the stock firmware cannot be recovered because it has not been read out.  
A MSM9066 programmer is required currently ($30 USD as of 7/14/2023 on AliExpress; $27 on eBay as of 11/20/2023).  

# Flasher (alternative)
Logic analyzer decodings of several flasher operations/programming cycles have been captured:  
https://github.com/mightymos/msm9066_capture  

TODO: ADD link to alternative flasher written for built in ESP8256
TODO: ADD notes about limitations of flasher 


# Special Thanks
We thank Vincent Defert for the use of modified makefiles and familiarity with 8051/8052 based microcontrollers:  
https://github.com/area-8051/uni-STC
