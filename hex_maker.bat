set BASE=%CD%

newt build %1
newt create-image -2 %1 0.0.0.1
newt mfg create %1 0.0.0.1
arm-none-eabi-objcopy -I ihex -O ihex --change-addresses=0x08000000 %BASE%\bin\mfgs\%1\mfgimg.hex %BASE%\hexTargets\%1.hex