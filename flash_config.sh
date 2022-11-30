#!/bin/bash

# call configmgr to set initial config if required (give devEUI as 16 hex digits). Other project specific appcore defaults set in the apps/syscfg.yml
if  [ -z $1 ]
then
    echo "Usage : flash_config.sh <deveui> [<appKey>]"
    exit 1
else
DEVEUI=$1
fi

if  [ -z $2 ]
then
  APPKEY=00000000000000000000000000000000
else
  APPKEY=$2
fi

APPEUI=0000000000000000
OUTFILE=config_$DEVEUI.hex
#STM32 IS LSB
ACTIVE_DEVICE=00
INACTIVE_MINS=$(printf '%02x\n' 10)000000
ENABLE_DEVICE_STATE_LEDS=00

echo "INACTIVE MIN $INACTIVE_MINS"

echo "Configuring with devEUI $DEVEUI and appKey $APPKEY"
#if configuring revB base cards add --addConfig 040C:1:01 to following line
java -jar configmgr-linux.jar --hexOutput ./$OUTFILE --addConfig 0101:8:$DEVEUI --addConfig 0102:8:$APPEUI --addConfig 0103:10:$APPKEY --addConfig 0410:4:$INACTIVE_MINS --addConfig 040F:1:$ACTIVE_DEVICE --addConfig 0411:1:$ENABLE_DEVICE_STATE_LEDS
java -jar configmgr-linux.jar -i ./$OUTFILE --dumpConfig config.csv
echo "Config is in $OUTFILE"
