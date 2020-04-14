/**
 Wyres private code
 */

#include <string.h>
#include <assert.h>
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"

#include "build.h"

#include "wyres-generic/wutils.h"
#include "wyres-generic/timemgr.h"
#include "wyres-generic/rebootmgr.h"
#include "app-core/app_core.h"

static const char* BUILD_TARGET_NAME=(MYNEWT_VAL(TARGET_NAME));   // set by mynewt
static int BUILD_VERSION_MAJOR=(MYNEWT_VAL(BUILD_VERSION_MAJOR));
static int BUILD_VERSION_MINOR=(MYNEWT_VAL(BUILD_VERSION_MINOR));
static int BUILD_VERSION_DEVNUMBER=(MYNEWT_VAL(BUILD_VERSION_DEVNUMBER));
static const char* BUILD_DATE=__DATE__ " " __TIME__;

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */
int
main(int argc, char **argv)
{

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif
    // Allow debugger to get its rear in gear (otherwise misses initial breakpoints) and signal booting
    hal_gpio_init_out(LED_1, 1);
    for (int i=0;i<5;i++) {
        TMMgr_busySleep(500);
        hal_gpio_write(LED_1, 1);
        TMMgr_busySleep(500);
        hal_gpio_write(LED_1, 0);
    }
    hal_gpio_deinit(LED_1);

    // init everyone
    sysinit();

    // unittest or real app?
#ifdef UNITTEST
    bool ret = true;
    // call the unittests here
    ret &= unittest_gps();
    ret &= unittest_cfg();
//    assert(ret);        // If UTs fail, assert - this should probably return a value from the main?
#endif /* UNITTEST */
    log_info("%s v[%d.%d.%d] built[%s]", BUILD_TARGET_NAME, BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_DEVNUMBER, BUILD_DATE);
    // startup app tasks etc
    app_core_start(BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_DEVNUMBER, BUILD_DATE, BUILD_TARGET_NAME);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);

    return 0;
}
