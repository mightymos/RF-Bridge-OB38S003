# Introduction

This is an alternative firmware for radio decoding implemented on an 8052 based On-Bright OB38S003 microcontroller.
This microcontroller is present in the Sonoff Bridge R2 v2.2 433 MHz radio receiver.  
The intent is to avoid the need to perform hardware modification while still allowing support for additional radio protocols.  

The firmware is inspired by 1) RF-Bridge-EFM8BB1 (Portisch) and 2) rc-switch projects.  

Description of Sonoff R2 v2.2 bridge hardware modification is here:  
https://github.com/arendst/Tasmota/discussions/13283

# Status
THIS IS A WORK IN PROGRESS and should not be used by typical users (yet).  
Please note that HELP is needed (see Flasher sections below). 

That said, decoding of "protocol 1" seems to work reliably.  
For example door reed sensors - both stock and modified - have been successfully decoded.  
https://github.com/mightymos/ReedTripRadio

Transmission also works - however, strange things happen when both transmission and receiving are enabled.  
Therefore transmitting is disabled for now.  

# Flasher
Reprogramming the OB38S003 requires erasing the chip because the stock firmware is protected.  
This means that, once erased, the stock firmware cannot be recovered because it has not been read out.  
A MSM9066 programmer is required presently ($30 USD as of 7/14/2023 on AliExpress).  

# Flasher (alternative)
Logic analyzer decodings of several flasher operations/programming cycles have been captured:  
https://github.com/mightymos/msm9066_capture

Help is needed to create an alternative programmer/flasher:  
https://github.com/mightymos/msm9066_capture/issues/1


Use of a cheaper flasher or using Tasmota itself is probably required for widespread use.  


# Special Thanks
We thank Vincent Defert for the use of modified makefiles and familiarity with 8051/8052 based microcontrollers:  
https://github.com/area-8051/uni-STC
