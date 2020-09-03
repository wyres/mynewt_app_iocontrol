@echo on
:: flash the app hex file name in %1 to the attached target using a st-linkv2 (you can also use the st-tool directly)
:: you should have madee hex image first using hex_maker.bat
set BASE=%CD%
:: and now flash it
set OPENOCD=c:/soft/openocd-0.10.0
:: openocd limitation : scripts MUST use a full path not relative one?
openocd -f %OPENOCD%/scripts/interface/stlink-v2.cfg -f %OPENOCD%/scripts/target/stm32l1.cfg -c "init; halt; program %1 verify; reset;shutdown"

::
::Command: flash erase_address [pad] [unlock] address length
::
::    Erase sectors starting at address for length bytes. Unless pad is specified, address must begin a flash sector, and address + length - 1 must end a sector. Specifying pad erases extra data at the beginning and/or end of the specified region, as needed to erase only full sectors. The flash bank to use is inferred from the address, and the specified length must stay within that bank. As a special case, when length is zero and address is the start of the bank, the whole flash is erased. If unlock is specified, then the flash is unprotected before erase starts. 
::
::Command: flash filld address double-word length
::Command: flash fillw address word length
::Command: flash fillh address halfword length
::Command: flash fillb address byte length
::
::   Fills flash memory with the specified double-word (64 bits), word (32 bits), halfword (16 bits), or byte (8-bit) pattern, starting at address and continuing for length units (word/halfword/byte). No erasure is done before writing; when needed, that must be done before issuing this command. Writes are done in blocks of up to 1024 bytes, and each write is verified by reading back the data and comparing it to what was written. The flash bank to use is inferred from the address of each block, and the specified length must stay within that bank. 

exit /b