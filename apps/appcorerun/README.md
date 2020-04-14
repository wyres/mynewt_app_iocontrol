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

This is the Marmotte cage monitoring project for UGA, version 2.

This is a MyNewt firmware project, to use it you will require knowledge of MyNewt:
Please refer to the documentation at http://mynewt.apache.org/ or ask questions on dev@mynewt.apache.org

Hardware setup:
The BSP for this project is by default for a Wyres W_Proto card. This provides the MCU (STM32L151CC), LoRa radio (SX1272),
the battery power (2x AA), and various leds, gpios and sensors.

Externally, there should be minimally a door status detector wired to pin 3 of the 'maker connector' CN4, which takes it LOW when the door is CLOSED and high when the door is OPEN. In the cage monitoring, this is a hall effect sensor, with a magnet on the door unit.
An optional push button can be wired to pin9 of CN4, to pull it low when pushed for a 'test' phase checking the door sensor, and the backedn lora connection.

An optional UART connection is possible using pins 6 (RX) and 8 (TX) of CN4, running at 115200 baud. This provides a console connection immediately after reboot (for config) and output logging during operation.

Operation:
The basic operation of the state machine is governed by the 'app-core' generic device framework (see the "mynewt-generic-app" package on Wyres github). This provides configuration setup, startup, local UART console, LoRaWAN OTAA and a generic idle-data collection-data upload loop.
Read the README.md for mynewt-generic-app/app-core package for details, including how to configure the LORaWAN devEUI/appKey using the console.

The 'mod-cage' module (found in mod_cage.c) plugs into this framework to deal with the test button and door sensor GPIOs. These are treated as interrupts, which force the immediate sending of an UL to the backend application (ie whenever the door state changes or the button is pressed). 
An application ack mechanism means an UL alert due to such an interruption is resent every 60s until the backend sends a DL to acknowledge it.

A short press on the test button (<5s) will execute a test phase, showing the door sensor read status on the leds (10s of either orange (open) or red (closed)) and then attempting a UL + DL ack to the backend. When the ack is received, both leds will light for 10s.

A long press on the test button (>7s) will toggle the activation status of the device : 
    active:
        - monitoring of the door sensor
        - test button does uplink test
        - led flashes orange once per 2s
        - UL packet every 5 minutes
    inactive:
        - no monitoring of door
        - test request ignored
        - flashes red once per 2s.
        - UL packet every 2 hours

The UL packets contain TLV elements, with the environmental information (temp, pressure, battery etc), the 'ack required' flag, and the cage status : door open or closed, test button pressed, device active/inactive. 
Element         Tag Length  Value
Temp            3   2       Temperature in 1/10 degC
Battery         7   2       Battery voltage in mV
Ack request     240 1       ack id, incremented each time a critical UL is correctly acked.
Cage status     241 12      0-3 : last time door opened : timestamp in ms since last restart
                            4-7 : last time door closed : timestamp in ms since last restart
                            8   : door state : 0=open, 1=closed
                            9   : unacknowledged alert waiting (0=no, 1=yes)
                            10  : test phase in progress (0=no, 1=yes)
                            11  : device state : 0=inactive, 1=active

If the ack request TLV is present, the backend should (must) send a DL packet containing the following bytes:
06 01 1C 01 <ackId>
to stop the alert UL happening every 60s.
