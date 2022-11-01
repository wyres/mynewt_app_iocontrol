#!/bin/bash
# linux version of hex_maker

BASE=`pwd`
NEWT_COMMAND=newt

TARGET=$1

if [ "${TARGET}" == "version" ]
then
    ${NEWT_COMMAND} version || exit 1
    arm-none-eabi-gcc -v
    arm-none-eabi-objcopy --version
    exit 0
fi

if [ "${TARGET}" == "boot" ]
then
# REM: must have done a newt build on the bootloaders already
    echo "Building bootloader : wproto"
    ${NEWT_COMMAND} clean wproto_bootloader || exit 1
    ${NEWT_COMMAND} build wproto_bootloader || exit 1
    if [ ! -d "${BASE}/bin/targets/wproto_bootloader/app/boot/" ]
    then
        echo "build created @mcuboot, copying to boot in ${BASE}/bin/targets/wproto_bootloader/app/boot/"
        cp -r ${BASE}/bin/targets/wproto_bootloader/app/@mcuboot/boot/ ${BASE}/bin/targets/wproto_bootloader/app
    fi
    echo "Building bootloader : wbasev2"
    ${NEWT_COMMAND} clean wbasev2_bootloader || exit 1
    ${NEWT_COMMAND} build wbasev2_bootloader || exit 1
    if [ ! -d "${BASE}/bin/targets/wbasev2_bootloader/app/boot/" ]
    then
        echo "build created @mcuboot, copying to boot in ${BASE}/bin/targets/wbasev2_bootloader/app/boot/"
        cp -r ${BASE}/bin/targets/wbasev2_bootloader/app/@mcuboot/boot/ ${BASE}/bin/targets/wbasev2_bootloader/app
    fi
    exit 0
fi

# REM: now our target
echo "Building target ${TARGET} : starting"
${NEWT_COMMAND} build ${TARGET} || exit 1
${NEWT_COMMAND} create-image -2 ${TARGET} 0.0.0.1 || exit 1
${NEWT_COMMAND} mfg create ${TARGET} 0.0.0.1 || exit 1
arm-none-eabi-objcopy -I binary -O ihex --change-addresses=0x08000000 \
    ${BASE}/bin/mfgs/${TARGET}/mfgimg.bin \
    ${BASE}/built/${TARGET}.hex || exit 1
# REM: copy the results to the built dir to be able to find and identify them easier
cp ${BASE}/bin/targets/${TARGET}/app/apps/appcorerun/appcorerun.elf ${BASE}/built/${TARGET}.elf || exit 1

# REM: ensure the current.elf is the elf we want to debug (so VC knows) and current.hex for the flash script
cp ${BASE}/built/${TARGET}.elf ${BASE}/built/current.elf || exit 1
cp ${BASE}/built/${TARGET}.hex ${BASE}/built/current.hex || exit 1
echo "Finished target ${TARGET} : result is in ${BASE}/built"
