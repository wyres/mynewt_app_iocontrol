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
/**
 * Cage door and test button handling app module
 *  - short button press = test reception of lora : ignore if 'device inactive'
 *      - force UL with ack request TLV, and enqueue 2nd force UL to ensure get reponse
 *      - flash leds during 'data collection' to show whats happening
 *  - long button press - change device state (if active, deactivate, if inactive, activate)
 *  - door close callback : 
 *      - force UL, set flag to indicate alert not acked, request app ack
 *      - when get ack action then reset flag, reset idle times
 *  - tick cb:
 *      - if alert not acked flag set, then force UL (ie get 1 UL/min until acked)
 */

#include "os/os.h"
#include "bsp/bsp.h"

#include "wyres-generic/wutils.h"
#include "wyres-generic/timemgr.h"
#include "wyres-generic/ledmgr.h"
#include "wyres-generic/rebootmgr.h"
#include "wyres-generic/movementmgr.h"
#include "wyres-generic/sensormgr.h"

#include "app-core/app_core.h"
#include "app-core/app_msg.h"

// Use the PTI module id, as won't have both at same time
#define MY_MOD_ID   (APP_MOD_PTI)

#define USER_BUTTON  ((int8_t)MYNEWT_VAL(BUTTON_IO))
#define DOOR_CONTACT  ((int8_t)MYNEWT_VAL(DOOR_IO))

#define DOOR_OPEN (0)
#define DOOR_CLOSED (1)

// define our specific ul tags that only the marmottes can deocde
#define UL_TAG_CAGE_STATE (APP_CORE_UL_APP_SPECIFIC_START)

// COntext data
static struct appctx {
    uint32_t lastButtonReleaseTS;
    uint32_t lastDoorOpenedTS;
    uint32_t lastDoorClosedTS;
    uint8_t ackId;
    bool unackedAlert;
    bool test;
} _ctx;

static void buttonChangeCB(void* ctx, SR_BUTTON_STATE_t currentState, SR_BUTTON_PRESS_TYPE_t currentPressType);
static void doorChangeCB(void* ctx, SR_BUTTON_STATE_t currentState, SR_BUTTON_PRESS_TYPE_t currentPressType);
static void alertAcked(uint8_t* v, uint8_t l);
// Map the button state to a door state (as door is closed when hall effect has NO magnet -> which is a 'released' state for the button api)
static uint8_t mapDoorState(uint8_t buttonState) {
    if (buttonState==SR_BUTTON_RELEASED) {
        return DOOR_CLOSED;
    }
    return DOOR_OPEN;
}
// My api functions
static uint32_t start() {
    if (_ctx.test) {
        // Long light of correct led depending on door open or closed
        if (mapDoorState(SRMgr_getButton(DOOR_CONTACT))==DOOR_OPEN) {
            ledStart(LED_1, FLASH_ON, 10);
        } else {
            ledStart(LED_2, FLASH_ON, 10);
        }
        log_debug("MP:test cage check : 10s");
        return 10*1000;      // to let user see leds
    }
    log_debug("MP:start cage check : 1s");
    return 1*1000;
}

static void stop() {
    log_debug("MP:done");
}
static void off() {
    // ensure sensors are low power mode
}
static void deepsleep() {
    // ensure sensors are off
}
static bool getData(APP_CORE_UL_t* ul) {
    log_info("MP: UL cage door[%s] test[%s] unacked alert[%s]", 
            (mapDoorState(SRMgr_getButton(DOOR_CONTACT))==DOOR_OPEN?"OPEN":"CLOSED"),
            (_ctx.test?"YES":"NO"),
            (_ctx.unackedAlert?"YES":"NO"));
    // write to UL TS and current states
    /* structure equiv:
     * uint32_t lastOpenedTS;
     * uint32_t lastClosedTS;
     * uint8_t isDoorClosed;      // 0 = no, 1=yes
     * uint8_t unackedAlert;
     * uint8_t test
     * uint8_t device state : 0=deactivated, 1=activated
     */
    uint8_t ds[12];
    Util_writeLE_uint32_t(ds, 0, _ctx.lastDoorOpenedTS);
    Util_writeLE_uint32_t(ds, 4, _ctx.lastDoorClosedTS);
    ds[8] = mapDoorState(SRMgr_getButton(DOOR_CONTACT));
    ds[9] = (_ctx.unackedAlert?1:0);
    ds[10] = (_ctx.test?1:0);
    ds[11] = (AppCore_isDeviceActive()?1:0);
    app_core_msg_ul_addTLV(ul, UL_TAG_CAGE_STATE, 12, &ds[0]);
    if (_ctx.unackedAlert || _ctx.test) {
        // request ack from app, giving id of this request (1 byte)
        app_core_msg_ul_addTLV(ul, APP_CORE_UL_APP_ACK_REQ, 1, &_ctx.ackId);
    }
    return true;       // all critical!
}
static void tick() {
    // Check if 'unacked alert' or 'test' flag is set, if so force UL now for this module
    if (_ctx.unackedAlert || _ctx.test) {
        AppCore_forceUL(MY_MOD_ID);
    }
}

static APP_CORE_API_t _api = {
    .startCB = &start,
    .stopCB = &stop,
    .offCB = &off,
    .deepsleepCB = &deepsleep,
    .getULDataCB = &getData,
    .ticCB = &tick,    
};
// Initialise module
void mod_cage_init(void) {
    // hook app-core for env data
    AppCore_registerModule("CAGE", MY_MOD_ID, &_api, EXEC_PARALLEL);
    AppCore_registerAction(APP_CORE_DL_APP_ACK, alertAcked);

    if (USER_BUTTON>=0) {
        // add cb for button press, no context required
        SRMgr_registerButtonCB(USER_BUTTON, buttonChangeCB, NULL);
    }
    if (DOOR_CONTACT>=0) {
        // using extio as input irq
        SRMgr_registerButtonCB(DOOR_CONTACT, doorChangeCB, NULL);
    }
    log_info("MP: cage operation initialised");
}

// internals
// DL action indicating backend got my critical UL
static void alertAcked(uint8_t* v, uint8_t l) {
    // clear flag saying alert (and test)
    _ctx.unackedAlert = false;
    _ctx.ackId++;       // Ready for next time
    if (_ctx.test) {
        _ctx.test = false;
        // both LED on for 10s to show connection is good
        ledStart(LED_1, FLASH_ON, 10);
        ledStart(LED_2, FLASH_ON, 10);
    }
    log_info("UL alert ackd!");
}

// callback each time button changes state
static void buttonChangeCB(void* ctx, SR_BUTTON_STATE_t currentState, SR_BUTTON_PRESS_TYPE_t currentPressType) {
    if (currentState==SR_BUTTON_RELEASED) {
        log_info("MP:button released, duration %d ms, press type:%d", 
            (SRMgr_getLastButtonReleaseTS(USER_BUTTON)-SRMgr_getLastButtonPressTS(USER_BUTTON)),
            SRMgr_getLastButtonPressType(USER_BUTTON));
        // If short or medium press, then connection test (only if device is active)
        if (currentPressType==SR_BUTTON_SHORT || currentPressType==SR_BUTTON_MED) {
            if (AppCore_isDeviceActive()) {
                _ctx.test = true;
                _ctx.lastButtonReleaseTS = SRMgr_getLastButtonReleaseTS(USER_BUTTON);
                log_info("MP:doing cnx test");
                // ask for immediate UL with only us consulted
                AppCore_forceUL(MY_MOD_ID);
            }
        } else {
            // long presses mean change active state of device
            if (AppCore_isDeviceActive()) {
                log_info("MP:device deactivated");
                AppCore_setDeviceState(false);      // goto inactive
                // ask for immediate UL with only us consulted
                AppCore_forceUL(MY_MOD_ID);
            } else {
                log_info("MP:device activated");
                AppCore_setDeviceState(true);      // goto active
                // ask for immediate UL with only us consulted
                AppCore_forceUL(MY_MOD_ID);
            }
        }
    } else {
        log_info("MP:button pressed");
    }
}
// callback each time extio changes state
static void doorChangeCB(void* ctx, SR_BUTTON_STATE_t currentState, SR_BUTTON_PRESS_TYPE_t currentPressType) {
    if (AppCore_isDeviceActive()) {
        if (mapDoorState(currentState)==DOOR_OPEN) {
            log_info("MP:door opened");
            _ctx.lastDoorOpenedTS = TMMgr_getRelTimeMS();
            _ctx.unackedAlert = true;
            // ask for immediate UL with only us consulted
            AppCore_forceUL(MY_MOD_ID);
        } else {
            log_info("MP:door closed");
            _ctx.lastDoorClosedTS = TMMgr_getRelTimeMS();
            _ctx.unackedAlert = true;
            // ask for immediate UL with only us consulted
            AppCore_forceUL(MY_MOD_ID);
        }
    } else {
        log_info("MP:door change but device inactive");
    }
}