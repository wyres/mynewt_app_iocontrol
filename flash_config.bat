@echo off
:: avoid env corruption
SETLOCAL 

:: get command name and cmd dir in case
SET me=%~n0
SET parent=%~dp0
set BASE=%CD%

set OPENOCD=c:/soft/openocd-0.10.0

:: call configmgr to set initial config if required (give devEUI as 16 hex digits). Other project specific appcore defaults set in the apps/syscfg.yml
set deveui=%1
if -%deveui%-==-- (
    echo "Usage : %me% <deveui> [<appKey>]"
    exit /b
)
set appkey=00112233445566778899AABBCCDDEEFF
if not -%2-==-- ( set appkey=%2 )
set outfile=config_%deveui%.hex

echo Configuring with devEUI %deveui% and appKey %appkey%
:: if configuring revB base cards add --addConfig 040C:1:01 to following line
java -jar configmgr.jar --hexOutput ./%outfile% --addConfig 0101:8:%deveui% --addConfig 0103:10:%appkey% 
:: openocd -f %OPENOCD%/scripts/interface/stlink-v2.cfg -f %OPENOCD%/scripts/target/stm32l1.cfg -c "init; halt; program config.hex verify; reset;shutdown"
:: openocd not working with PROM writes yet
java -jar configmgr.jar -i ./%outfile% --dumpConfig config.csv
echo Config is in %outfile%


