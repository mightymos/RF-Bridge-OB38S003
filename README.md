# RF-Bridge-OB38S003
RF-Bridge-OB38S003

This is an attempt to port RF-Bridge-EFM8BB1 over to the Sonoff Bridge R2 v2.2.  
This avoids the need to perform hardware modification.  
The radio decoder is an 8052 based On Bright OB38S003.  

Reprogramming the OB38S003 requires erasing the chip because the stock firmware is protected.  
A MSM9066 programmer is needed to flash ($30 USD).  

Captures of the erase/write/verify programming cycle have been captured here:
https://github.com/mightymos/msm9066_capture

If someone wanted to create a programmer using less expensive/existing hardware or within Tasmota itself...that would be cool.  