# Introduction

An alternative firmware for 433 MHz radio to wifi bridges, targetting these boards/microcontrollers:  

| Board | Microcontroller | Passthrough |  RCSwitch | Portisch | Notes |
| ------------- | ------------- | ------------- | ------------- | ------------- | ------------- |
| Sonoff Bridge R2 v1.0 (black box) | EFM8BB1 | Supported | Decode + Transmit (RfRaw 0xA5/0xA8) | Supported | Flashing works, see instructions below for Tasmota firmware upgrade |
| Sonoff Bridge R2 v2.2 (white box) | OB38S003 | Supported | Decode + Transmit (RfRaw 0xA5/0xA8) | Supported | Requires reprogramming with official or open source flasher |
| EFM8BB1 Busy Bee Low Cost Kit Board | EFM8BB1 | Supported | Decode + Transmit (RfRaw 0xA5/0xA8) | Supported | Requires external receiver and/or transmitter |

These microcontrollers were originally intended for radio decoding but lacked support for additional protocols.  
As a consequence many people just bypass the microcontroller with a hardware modification:  
https://github.com/arendst/Tasmota/discussions/13283  

Decoding may then be done directly on the ESP8265 (e.g., ESPHome/Tasmota/ESPurna typically used for this purpose).  


The intent here is to avoid the need to perform hardware modification.  
This can be accomplished in two ways:  

1) Mirroring the voltage levels on the radio data pins over to uart pins (used as gpio) already connected to ESP8265.  
   (this uses the microcontroller as a passthrough but using software instead of hardware modification)  
   
2) Decoding/encoding radio packets on the microcontroller and sending formatted to ESP8265 over uart/serial  
   (similar to factory firmware but with ability for source code protocols to be added, behavior modified, etc.)  
   
A benefit to keeping the microcontroller involved is that the radio LED and buzzer may still be controlled.  
The downside is the effort required to develop firmware and flash by the end user.  

The firmware radio decoding is inspired by 1) RF-Bridge-EFM8BB1 (Portisch) and 2) rc-switch projects.  

# Status
THIS IS A WORK IN PROGRESS and requires multiple flashing steps.   
 
For many users it is probably easiest to flash the passthrough mode.  
Then perform any actual decoding/encoding on the ESP8265 using Tasmota/ESPHome.

# Installation
Install SDCC compiler for your platform:  
https://sdcc.sourceforge.net/  

On command line run make.  
For now modify Makefile to select desired target.  
Built firmware placed in build directory.  
See Flasher section below.  

# Tasmota/ESPHome
Using passthrough hex file.

The following pins can be used in ESPHome / Tasmota:  
-flash **tasmota-sensors.bin** to ESP on RFbridge  
-configure module as e.g. **Generic(0)**)  
-save  

now set pins to:  
TX pin to GPIO1        (RFSend)  
RX pin to GPIO3        (RFRecv)  
WIFI LED pin to GPIO13 (LedLink)  

# Flasher (official)
OB38S003 reprogamming requires erasing the chip because the stock firmware is protected.  
The stock firmware cannot be recovered because it has not been read out.  
An official MSM9066 programmer or open source flasher (see below) can be used.  


EFM8BB1 reprogramming can be done with Tasmota.  
The module must be set as Sonoff Bridge (25) to allow flashing:  
https://tasmota.github.io/docs/Modules/  
Follow the instructions for flashing Portisch but using this firmware:  
https://tasmota.github.io/docs/devices/Sonoff-RF-Bridge-433/  


# Flasher (open source)
An Arduino based flasher written for the built in ESP8265/ESP8266/ESP32 is available:  
https://github.com/mightymos/OnbrightFlasher

Logic analyzer decodings of several flasher operations/programming cycles had been captured:  
https://github.com/mightymos/msm9066_capture  

While cumbersome to use, several people have successfully flashed hex files.  


# Previous Work

A successful attempt was made to compile "Portisch" with the open source SDCC compiler.  
It is difficult to fit all decoding protocols and sniffing features in code and ram spaces.  
This effort allowed porting to the R2 V2.2 Sonoff (white box).  
https://github.com/mightymos/SonOfPortisch

There are at least three versions of rcswitch.  
We attempt to use the simplest and most understandable for now (from sui77).  
https://github.com/sui77/rc-switch  
https://github.com/1technophile/rc-switch  
https://github.com/arendst/Tasmota/tree/development/lib/lib_rf/rc-switch  


# Special Thanks
We thank Vincent Defert for the use of modified makefiles and familiarity with 8051/8052 based microcontrollers:  
https://github.com/area-8051/uni-STC
