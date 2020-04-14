/**
 * Copyright 2019 Wyres
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, 
 * software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
 * either express or implied. See the License for the specific 
 * language governing permissions and limitations under the License.
*/

This is a generic IO handling device.

This is a MyNewt firmware project, to use it you will require knowledge of MyNewt:
Please refer to the documentation at http://mynewt.apache.org/ or ask questions on dev@mynewt.apache.org

Hardware setup:
The BSP for this project is by default for a Wyres W_Proto card. This provides the MCU (STM32L151CC), LoRa radio (SX1272),
the battery power (2x AA), and various leds, gpios and sensors.

Up to 8 GPIOs are handled, mapped from their generic id 1-8 to real GPIO pins in the specific target (which should apply to a specific use case)

An optional UART connection is possible using pins 6 (RX) and 8 (TX) of CN4, running at 115200 baud. This provides a console connection immediately after reboot (for config) and output logging during operation.

Operation:
The basic operation of the state machine is governed by the 'app-core' generic device framework (see the "mynewt-generic-app" package on Wyres github). This provides configuration setup, startup, local UART console, LoRaWAN OTAA and a generic idle-data collection-data upload loop.
Read the README.md for mynewt-generic-app/app-core package for details, including how to configure the LORaWAN devEUI/appKey using the console.

The 'mod-io' module (found in mod_io.c) plugs into this framework to deal with the GPIOs. 

The UL packets contain TLV elements, with the environmental information (temp, pressure, battery etc), the 'ack required' flag, and the cage status : door open or closed, test button pressed, device active/inactive. 
Element         Tag Length  Value
Temp            3   2       Temperature in 1/10 degC
Battery         7   2       Battery voltage in mV
IO status       241 12      
                            11  : device state : 0=inactive, 1=active

