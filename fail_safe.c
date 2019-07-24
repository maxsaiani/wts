/*
 ******************************************************************************
 *
 *  FILE:    fail_safe.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    19/11/2008
 *
 *  DESCRIPTION: Machine check tasks.
 *              Perform checks on sw and hardware.
 *              Kick into a fail safe mode if problems encountered.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#include <io430.h>
#include "stdint.h"
#include "pio.h"
#include "steam_flow.h"
#include "solenoids.h"
#include "rtc_api.h"
#include "cooling_air_valve.h"
#include "pio.h"
#include "fail_safe.h"
#include "timers.h"

struct {
    union {
        struct {
            uint16_t comms_timeout:1;
        }bit;
        uint16_t bits;
    };
}fail;

/*
 ******************************************************************************
 *  FUNCTION NAME:          fail_safe_init
 *  FUNCTIONAL DESCRIPTION:
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void fail_safe_init(void)
{
    fail.bits = 0;  /* Reset all failure bits. */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          fail_safe_state_machine
 *  FUNCTIONAL DESCRIPTION: 
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void fail_safe_state_machine(void)
{
    /*--- Simple to begin with. */
    /*    Just kick into safe mode if comms disappear.  */
    if(rtc_expired(rtc_msg_timeout)){
        fail.bit.comms_timeout = 1;
    } else {
        /* We let failure automatically restore on comms recovery.  */
        fail.bit.comms_timeout = 0;
    }

    if(fail.bits == 0){
        /*--- Not presently in failure mode. */
        P3OUT |= P3O7_N_LED_ALARM;  /* Extinguish failure LED.  */
        return;
    }

    /*--- Only makes it here if there is a failure. */
    P3OUT &= ~P3O7_N_LED_ALARM;     /* Light failure LED.  */

    /*--- Reset solenoid drivers. */
    fill_solenoid_set(0);
    purge_solenoid_set(0);

    /*--- Stop steam flow.  */
    steam_flowrate_set(0);

    /*--- Close cooling air valve.  */
    cooling_air_valve_set(0, 0);

    /*--- Reset all outputs that are under direct SW control. */
    P4OUT &= ~( P4O2_DOUT3_PUMP1_FILL       |
                P4O3_DOUT4_PUMP2_POLISH     |
                P4O4_DOUT5_PUMP3_CONDENSATE );
               
     /*--- Pull the PWM outputs next to 0V */
    pwm1_set(0);
    pwm2_set(0);
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          fail_safe_get_state
 *  FUNCTIONAL DESCRIPTION: Returns NZ if unit should be fail safe mode.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
uint8_t fail_safe_is_failed(void)
{
    return fail.bits != 0;  /* Gives 1 if any failure bits set 0 otherwise. */
}
