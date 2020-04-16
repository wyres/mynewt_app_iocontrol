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
The 8 'io ports' are configured in the syscfg.yml file in the target. For each io, you will have a line like:
    IO_0: 'defineIO(0, 11, "reset hall", IO_STATE, PULL_UP, 0)'
Each line starts with the virtual IO you are defining (IO_0) and consists of a function call to defineIO with the following parameters:
    # Parameters: 
    #   - channel id being defined (0-7) : this should correspond to the IO_X variable.
    #   - gpio pin (add 16 for group B, eg PB10 will be 26)
    #   - name (used for debug)
    #   - type :    IO_DIN (digital in), 
    #               IO_DOUT (digital out), 
    #               IO_AIN (ADC input), 
    #               IO_BUTTON (button type input with debounce and press length as value, UL sent when released), 
    #               IO_BUTTON_LINKED (as for IO_BUTTON, except the initial value parameter is the ioid of a DOUT IO, and when the button is   #                  pressed then that linked output will be toggled (value = !value)). This lets you have a local override on the output...
    #               IO_STATE (like button but value is 0/1, UL sent on each state change)
    #               IO_PWMOUT (PWM output at frequency between 0.1kHz and 25.5kHz (value /10 * 1000Hz)). A value of 0 means off.
    #   - pull type (for IO_DIO, IO_BUTTON, IO_STATE) : HIGH_Z, PULL_UP, PULL_DOWN
    #   - initial value (for output types)
Once the target is built and flashed, the device will send UL packets containing the data, updated from input IOs, and accept DL packets, with new values to write to output IOs. Note that all 8 values are sent/received to make it simpler. On the UL, output IO values will be the last written one, and on the DL, input IO's values are ignored.

The UL packets are formatted as TLV elements, with the environmental information (temp, pressure, battery etc), the 'ack required' flag, and the cage status : door open or closed, test button pressed, device active/inactive. 
Element         Tag Length  Value
Temp            3   2       Temperature in 1/10 degC
Battery         7   2       Battery voltage in mV
IO status       241 9       0-7 : 1 byte per IO, containing either 
                                - the latest input value for input type IO
                                - the last written value for output type IO
                            8  : device state : 0=inactive, 1=active

The DL packets are formatted as TLV actions (see appcore doc for details), and the action with id 240 (0xF0) sets the output IO values. It takes an 8 byte parameter block, with 1 byte per IO containing the output values for any output IOs.
For example, with a payload of (hex value):
b0 : 06 - appcore header meaning 'downlink actions'
b1 : 01 - number of actions in this packet
b2 : F0 - action id - 0xF0 = 240 = app specific action
b3 : 08 - length of parameter block of this action
b4-b11 : 00 01 02 03 04 05 06 07
        - 8 byte block, 1 byte per IO : so IO_0 will get written with value of 00, IO_1 with value of 01 etc...

