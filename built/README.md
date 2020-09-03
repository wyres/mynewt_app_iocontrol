This is the destination directory for hex_maker.bat. It should produce for a given <targetname>:
 - <targetname>.elf
 - <targetname>.hex
 - current.elf
 - current.hex
The .hex contains the bootloader and the app, ready to load directly in the STM32. 
The .elfs contain the debug info neccessary for GDB type debugging - the 'current.elf' is the last build done so that your debugger run script can just 
reference this rather than explicit target names (in the case where you have multiple different targets in the same project)

To flash the w_proto or wbasev2 cards, use a ST-LinkV2 with either OpenOCD (using the flash_stm32_stlink.bat) or the ST Loader tool directly with the .hex.

To debug OpenOCD is recommended, used with VisualStudio Code and an ST-Link v2 SWD adapter.
