#!/bin/bash

INPUT=keys.txt
DEVEUIBASE=00000000000000
COUNT=1
while read -r line
do
  APPKEY=$line
  DEVEUI=$DEVEUIBASE$(printf '%02x\n' $COUNT)
  echo "flash $DEVEUI $APPKEY"
  ./flash_config.sh $DEVEUI $APPKEY
  COUNT=$((COUNT+1))
done < "$INPUT"
