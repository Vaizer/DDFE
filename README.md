# DIM's Display Function Extender 
Device for output diagnostic parameters on Volvo P2 dashboard's (DIM) display.\
Using library https://github.com/coryjfowler/MCP_CAN_lib from Cory J Fowler, big thanks for a lot of useful materials to https://github.com/vtl and Volvo-DDD project.

Included sketch mainly aims pre-FL models (MY2004-), but all accumulated informartion about other MY are included in sketch for you to try!\
Set up for 2001 Volvo V70XC with B5244T engine and automatic transmission, should work without changes on same configuration (confirmed to work also on B5234T and year 2000 V70).

Pre-FL cars (MY2004-) would need getting second OBD2 (or any other) connector **WITH** CAN-L and CAN-H from CEM or it will not work!\
FL cars already have CANs on OBD2. Instructions for connecting second OBD2 and device assebly are also included.  \
For now **MY2003-2004 is not guaranteed to work** as I don't have correct IDs or cars to check if it will work with 2002 or other IDs.\
**2000, 2001, 2002, 2005+ tested.**


![14 АТФ_Т_найс](https://github.com/user-attachments/assets/8d8bf200-8e5b-4547-869b-4e78fdf6be25)

## Current supported readouts:
**Engine (ECM):**
- Atmospheric pressure
- Boost pressure
- Intake air temperature
- Mass airflow 
- Battery voltage
- Coolant temperature
- AC pressure
- Speed (kmh)
- Idle fuel trim
- Long-term fuel trim (LTFT) (Low)
- Long-term fuel trim (LTFT) (High)
- Short-term fuel trim (STFT)
- Misfire counter

**Gearbox (TCM):**
- ATF temperature

**Other sensors could be added through getting commands from VIDA log file and DB.**



![25 итог](https://github.com/user-attachments/assets/8411ec07-3aa5-44c1-9a3b-61005466b560)

## Controls:
- Short press RESET: changing current parameter (page)
- Long press RESET: disabling output from DDFE on screen (to use display an intended)
Screen enables on short press


## Hardware used:
- Arduino Nano
- 2x MCP2515
- DC-DC converter (12v to 5v)
- OBD2 connector (optional)


## More info about project on my Drive2:
- https://www.drive2.ru/l/712830156913249071/
- https://www.drive2.ru/l/717368116278996470/
- https://www.drive2.ru/l/721407721999439468/


Also check Volvo-P2-informer by drPioneer, inspired by my project: https://github.com/drpioneer/Volvo-P2-informer.\ Same hardware, sketch for FL (XC90 2011)

Volvo-DDD by vtl: https://github.com/vtl/volvo-ddd


Not a native english-speaker, not a programmer - sorry for mistakes.
