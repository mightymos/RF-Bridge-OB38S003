# RF-Bridge-OB38S003
RF-Bridge-OB38S003

THIS IS A WORK IN PROGRESS and should not be used by typical users (yet).

This project ports RF-Bridge-EFM8BB1 (Portisch) over to the Sonoff Bridge R2 v2.2.  
This avoids the need to perform hardware modification to support additional radio protocols.  
The radio decoder is an 8052 based On Bright OB38S003.

Portisch is referenced to understand state machines which handle AT-style commands (e.g., beep, send).
Portisch radio packet decoding seemed to be difficult to understand.
rc-switch project was referenced for further understanding radio packet decoding.

Reprogramming the OB38S003 requires erasing the chip because the stock firmware is protected.  
A MSM9066 programmer is needed to flash ($30 USD as of 7/14/2023).  

Captures of the erase/write/verify programming cycle have been captured here:
https://github.com/mightymos/msm9066_capture

Use of a cheaper flasher or using Tasmota itself is probably required for widespread use.  