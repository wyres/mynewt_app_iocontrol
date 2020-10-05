echo off
set BASE=%CD%

set TARGET=%1
if "%TARGET%"=="boot" (
:: must have done a newt build on the bootloaders already
    echo "Building bootloader"
    newt build wproto_bootloader || EXIT /B 1
::    newt build wbasev2_bootloader || EXIT /B 1
    exit /B 0
)
if "%TARGET%"=="version" (
    newt version || EXIT /B 1
    exit /B 0
)

:: now our target
echo "Building target %TARGET% : starting"
newt build %TARGET% || EXIT /B 1
newt create-image -2 %TARGET% 0.0.0.1 || EXIT /B 1
newt mfg create %TARGET% 0.0.0.1 || EXIT /B 1
arm-none-eabi-objcopy -I binary -O ihex --change-addresses=0x08000000 %BASE%\bin\mfgs\%TARGET%\mfgimg.bin %BASE%\built\%TARGET%.hex || EXIT /B 1
:: copy the results to the built dir to be able to find and identify them easier
cp %BASE%/bin/targets/%TARGET%/app/apps/appcorerun/appcorerun.elf %BASE%/built/%TARGET%.elf || EXIT /B 1

:: ensure the current.elf is the elf we want to debug (so VC knows) and current.hex for the flash script
cp %BASE%/built/%TARGET%.elf %BASE%/built/current.elf || EXIT /B 1
cp %BASE%/built/%TARGET%.hex %BASE%/built/current.hex || EXIT /B 1
echo "Finished target %TARGET% : result is in %BASE%/built"
