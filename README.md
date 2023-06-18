# PZEM-004T v3.0 ESP-IDF Library  
Basically a copy/paste from Jakub Mandula's excellent Arduino Library: https://github.com/mandulaj/PZEM-004T-v30 and added some ESP-IDF sauce.  
I needed this component for my power monitor project https://github.com/wernervanmele/esp-idf-hassio-powermon  
  
 **What works ?**  
- All measurements.
- Reset energy counter.  
- Change the device address.
  
 **Not (yet) Tested/Implemented**  
  - Alarms are not implemented.  
  

Tested with ESP-IDF Framework v5.0.2  
Only tested on a ESP32-WROOM Chip (mainly the MH-ET Live MiniKit v2.0).  
Tried wihtout luck with a Wemos Lolin D32 S3 Mini.  
  
**How to use ?**  
Put the components folder in the root of your project, if everything goes well the module is automatically detected during compile time ( I hope, "it works on my pc (c)" ).
Check the README.md in one of the examples to configure the UART.  
  
  
 
disclaimer: I'm not a developer or electronics specialist, use this code at your own risk and enjoy.  