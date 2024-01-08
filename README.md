Application to create a LoRaWAN connected object with multiple IO lines that can be controlled from the backend.
It uses the wyres w_proto or w_basev2 cards BSP (selected in the target), and the wyres generic utility library as a base.

It uses the wyres app-core generic state machine, with specific module in this project to handle generic GPIO lines (mod-io).

To build the project, ensure that you have:
 - 'newt' build tool version 1.8 or later (from the apache mynewt project)
 - 'gcc' tool set for cross-compile to arm (gcc-arm-none-eabi package, version 2.35 or later : eg the gcc-arm-none-eabi-9-2019-q4-major-win32.zip package)

NOTE: recent update to hex_maker.bat and to apacheèmynewt-core fork due to new versions of newt (didn't put the bootloader build in the right place) and gcc (over zealous array bounds checking)...

The easiest way to run the build is just to use the hex_maker.bat at the command line (with mingw64 or MSys2 shell):
```
> hex_maker.bat <mynewt target name>
```
Even if you are just running the newt commands in linux, look at the batch file to see the good order (and the wee hack for the bootloader build)

This will produce in the 'built' directory:
 - <targetname>.elf
 - <targetname>.hex
 - current.elf
 - current.hex
 
(The individual commands in this batch file can be executed directly at the command line if you want.)

The .hex file contains both the the bootloader and the app, ready to load directly in the STM32 using (for example) st-util.exe with an st-link. 

 Note: before bui_lding the main application target, you MUST build at least once the boot loader target (hex_maker.bat boot) so it is available to be merged into the combined 'manufacturing' hex file.
 

The .elfs contain the debug info neccessary for GDB type debugging - the 'current.elf' is the last build done so that your debugger run script can just 
reference this rather than explicit target names (in the case where you have multiple different targets in the same project)

To flash the w_proto or wbasev2 cards, use a ST-LinkV2 with either OpenOCD (using the flash_stm32_stlink.bat) or the ST Loader tool directly with the .hex.

To debug OpenOCD is recommended, used with VisualStudio Code and an ST-Link v2 SWD adapter.

Specific IO:
See the readme in the app directory for details on creating new mynewt targets for specific cabling of the card IO.

## Linux version

Building the hex file
```
./hex_maker.sh wbasev2_bootloader # only on time per base

./hex_maker.sh wbasev2_io_eu868_river_prod
```

Building the config file including the DEVEUI APPKEY and other config.
```
./flash_config.sh <DEVEUI> <APPKEY>
```
This will generate a config_DEVEUI.hex file that you will flash after the main hex file (example after wbasev2_io_eu868_river_prod.hex)


You can generate multiple config file with `genconfig.sh`. You must create a file with the APPKEY, one APPKEY per line. Then edit the genconfig.sh. This will call flash_config.sh multiple time with the generated DEVEUI and the APPKEY from the file.

# MyNewt project structure
A quick overview..
 
Starting with the project mynewt_app_iocontrol (sorry for the french):

 project.yml
 - Ceci décrit les ‘repos’ utilisé par le projet (qui se trouve dans ./repos) et leur origin git
 - En principe le ‘newt upgrade’ les cherche, mais il est plus sur de les recouperer par git clone à la main (en créent les noms de repetoires correspondant dans ./repos…)

 ./targets 
- chaqu’un contient le configuration dans leurs fichers yml le build pour leur cas (quel carte, config lora, etc)
-	pour faire les hex, il y a aussi du config pour chaque target dans ./mfgs à faire
-	le target ‘wproto_io_eu868_none_dev’ sera le bon bon pour commencer

 ./repos: Les projets git utiliser par ce projet :
-	Apache-mynewt-core : le noyau mynewt qui propose l’acces au hw, system de taches, etc
-	App-generic : framework application  pour creér une application de type capteur/actuateur’, qui s’occupe de tout la partie démarrage, console, config, connexion lorawan, boucle d’execution en machine à etat. Les capteurs ou autres action sont en forme de ‘modules’. Voir le readme.md dans le app-core repetoire du project git
-	Generic : blocks de code C utilitaires, pour diverse outils.. Par exemple, gérer du config, construire un machine à etat,  des drivers de capteurs, gérer les logs, etc. Plus ou moins independant du noyau mynewt… Inclut aussi une api ‘lorawan’ qui encapcule le stack lorawan du project ‘lorawan’..
-	Lorawan : Une portage sur mynewt de la stack lorawan stackforce.
-	Mcuboot : bootloader en version mynewt
-	Mynew-proto-bsp : BSP for W_PROTO card (schema ci-joint)
-	Wyres-gitlab-nynewt-bsp : BSP for W_BASEV2 card

 ./apps:
-	code spécific to this projet : 
-	main.c, qui demarre le ‘app-geneic’, qui gere tout le reste de l’execution
-	mod_io.c, qui est le module pour faire les actions sur les GPIOs selon le config qu’il trouve dans le syscfg.yml…
-	onewire.c/DS18820.c, un driver pour une capteur temperature avec le protocol onewire


