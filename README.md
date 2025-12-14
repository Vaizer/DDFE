Device for output diagnostic parameters on Volvo P2 dashboard's (DIM) display.
Using library https://github.com/coryjfowler/MCP_CAN_lib from Cory J Fowler

![14 АТФ_Т_найс](https://github.com/user-attachments/assets/8d8bf200-8e5b-4547-869b-4e78fdf6be25)

Uploaded file set up for 2001 Volvo V70XC with B5244T engine and automatic transmission, should work without changes on same configuration (confirmed to work also on B5234T and year 2000 V70).
Before FL (MY2004-) cars would need getting second OBD2 (or any other) connector WITH CAN-L and CAN-H from CEM or it will not work! FL cars already have CANs on OBD2.  
For now MY2003-2004 is not guaranteed to work as I don't have correct IDs or cars to check if it will work with 2002 or other IDs. 2000-2001, 2002, 2005+ tested.

Instructions for connecting second OBD2 and device assebly are also included.

![25 итог](https://github.com/user-attachments/assets/8411ec07-3aa5-44c1-9a3b-61005466b560)

Controls:
- Short press RESET: changing current parameter (page)
- Long press RESET: disabling output from DDFE on screen (to use display an intended)
Screen enables on short press

Hardware used:
- Arduino Nano
- 2x MCP2515
- DC-DC converter (12v to 5v)
- OBD2 connector (optional)

More info about project on my Drive2:
- https://www.drive2.ru/l/712830156913249071/
- https://www.drive2.ru/l/717368116278996470/
- https://www.drive2.ru/l/721407721999439468/

Also check Volvo-P2-informer by drPioneer, inspired by my project: https://github.com/drpioneer/Volvo-P2-informer. Same hardware, sketch for FL (XC90 2011)
