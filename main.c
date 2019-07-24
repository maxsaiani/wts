#include <stdio.h>
#include <stdlib.h>
#include <io430.h>
#include <in430.h>
//#include <math.h>
#include "pio.h"
#include "timers.h"

#include "clk_api.h"
#include "cooling_air_valve.h"
#include "rtc_api.h"
#include "gsebus_ser.h"
#include "comms.h"
#include "steam_flow.h"
#include "anin.h"
#include "fail_safe.h"
#include "wdg.h"
#include "reflash.h"
#include "fls_api.h"

#define MC (0x020)

uint8_t flash_err = 0;

#pragma location="this_code_first"    /* Place near start of flash. */

void main(void)
{
    /*-- Start up enough stuff to check the flash. */
    WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog timer to prevent time out reset
    pio_setup_pin_directions();
    wdg_hwTickle();
    clk_init();
    rtc_init();
    fls_init();
    reflash_startup_check();

    /*-- Flash is OK, continue as normal.   */
    timerA_init();
    timerB_init();
    fail_safe_init();
    ser_init();
    anin_init();
    steam_flow_init();
    cooling_air_valve_init();
    
    __enable_interrupt();           // enable interrupts
    
    while(1){                       /* Forever loop.            */
        wdg_hwTickle();             /* Toggle external watchdog.    */
        /* sleep()? */
        rtc_state_machine();        /* Keep timers up to date.  */
        comms_poll();               /* Process comms messages.  */
        anin_state_machine();       /* Filter analogue inputs.  */
        fail_safe_state_machine();
    }
}
