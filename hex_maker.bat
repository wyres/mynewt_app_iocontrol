set BASE=%CD%

:: must have done a newt build on the bootloaders already
newt build wproto_bootloader
:: newt build wbasev2_bootloader not yet done

:: now our target
newt build %1
newt create-image -2 %1 0.0.0.1
<<<<<<< HEAD
newt -v mfg create %1 0.0.0.1
:: check if source is .bin or .hex and ajust the objcopy
:: TODO
arm-none-eabi-objcopy -I binary -O ihex --change-addresses=0x08000000 %BASE%\bin\mfgs\%1\mfgimg.bin %BASE%\hexTargets\%1.hex
=======
newt mfg create %1 0.0.0.1
arm-none-eabi-objcopy -I binary -O ihex --change-addresses=0x08000000 %BASE%\bin\mfgs\%1\mfgimg.bin %BASE%\built\%1.hex

:: copy the results to the built dir to be able to find and identify them easier
cp %BASE%/bin/targets/%1/app/apps/appcorerun/appcorerun.elf %BASE%/built/%1.elf

:: ensure the current.elf is the elf we want to debug (so VC knows) and current.hex for the flash script
cp %BASE%/built/%1.elf %BASE%/built/current.elf
cp %BASE%/built/%1.hex %BASE%/built/current.hex
>>>>>>> 96e1f5d388a231b46e3effe9307d2e27c7a9919f
