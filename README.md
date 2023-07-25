# RF-Bridge-OB38S003
RF-Bridge-OB38S003

This is an attempt to port RF-Bridge-EFM8BB1 (Portisch) over to the Sonoff Bridge R2 v2.2.  
This avoids the need to perform hardware modification.  
The radio decoder is an 8052 based On Bright OB38S003.

Portisch is mainly referenced to understand state machines which handle AT-style commands (e.g., beep, send) between radio microcontroller and ESP8266 wifi chip.
rc-switch project was referenced for understanding radio packet decoding.

Reprogramming the OB38S003 requires erasing the chip because the stock firmware is protected.  
A MSM9066 programmer is needed to flash ($30 USD as of 7/14/2023).  

Captures of the erase/write/verify programming cycle have been captured here:
https://github.com/mightymos/msm9066_capture

If someone wanted to create a flasher using less expensive/existing hardware or within Tasmota itself...that would be cool.  