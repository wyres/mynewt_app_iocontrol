set BASE=%CD%

newt build %1
newt create-image -2 %1 0.0.0.1
newt -v mfg create %1 0.0.0.1
:: check if source is .bin or .hex and ajust the objcopy
:: TODO
arm-none-eabi-objcopy -I binary -O ihex --change-addresses=0x08000000 %BASE%\bin\mfgs\%1\mfgimg.bin %BASE%\hexTargets\%1.hex