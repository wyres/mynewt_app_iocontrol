@echo on
:: flash the app hex file name in %1 to the attached target using a jlink on usb 0 
:: and the config in config/config_38B8EBE00000%2.hex
:: make hex image using hex_maker.bat
:: make config using flash_config.bat (in ./config)
:: make command file : see https://wiki.segger.com/J-Link_Commander#Connecting_to_a_specific_J-Link
echo usb 50113956 > cmd.jlink
echo st >> cmd.jlink
echo si SWD >> cmd.jlink
echo speed 4000 >> cmd.jlink
echo h >> cmd.jlink
echo loadfile %1 >> cmd.jlink
echo loadfile config/config_38B8EBE00000%2%.hex >> cmd.jlink
echo r >> cmd.jlink
echo qc >> cmd.jlink
jlink -device STM32L151CC -commandfile cmd.jlink

exit /b