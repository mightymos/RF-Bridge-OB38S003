# RF-Bridge-OB38S003
RF-Bridge-OB38S003

This is an attempt to port RF-Bridge-EFM8BB1 over to the Sonoff Bridge R2 v2.2.  
This avoids the need to perform hardware modification.  
The radio decoder is an 8052 based On Bright OB38S003.  

Reprogramming the OB38S003 requires erasing the chip because the stock firmware is protected.  
A MSM9066 programmer is needed to flash ($30 USD).  
Hopefully I can capture I2C communication from the programmer.  
It can then be implemented in Tasmota itself or in less expensive hardware.  