#include <io430.h>
#include <in430.h>
#include "stdint.h"

#include "pio.h"
#include "cooling_air_valve.h"
#include "fail_safe.h"

struct cooling_air_valve cooling_air_valve;

void cooling_air_valve_init(void)
{
    /* Pulse stepper reset input. */

    /*--- Trigger valve closing. */
    cooling_air_valve.enabled = 1;
    cooling_air_valve_set(0, 0);
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          cooling_air_valve_set
 *  FUNCTIONAL DESCRIPTION: Set parameters for cooling air valve control.
 *  FORMAL PARAMETERS:      enabled: 
 *                              Cooling air enabled.  The valve is completely 
 *                              closed when ever this parameter goes from 
 *                              enabled to disabled or vise a versa.
 *                          position:
 *                              Required valve position.
 *                              The units for this is 0.1% of full scale.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void cooling_air_valve_set(uint8_t enabled, int16_t position)
{
    if(fail_safe_is_failed()){  /* Over ride setting if unit in failsafe mode */
        enabled = 0;
    }

    enabled = (enabled != 0);   /* Convert uint8_t to bitboolean. */
    if(enabled ^ cooling_air_valve.enabled){
        cooling_air_valve.enabled = enabled;
        __disable_interrupt();
        P5OUT &= ~(P5O5_N_MOTOR1_ENABLE | P5O4_MOTOR1_DIRECTION);
        cooling_air_valve.position = 600;   /* Perform 600 closing steps */
        cooling_air_valve.close_valve = 1;
        __enable_interrupt();
    }
    if(enabled){
        if(position < 0) position = 0;
        if(position > 1000) position = 1000;
        cooling_air_valve.required_pos = position * 3 / 5;
    } else {
        cooling_air_valve.required_pos = 0;
    }
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          cooling_air_valve_get_pos
 *  FUNCTIONAL DESCRIPTION: Return estimated valve position (.1% precision).
 *  FORMAL PARAMETERS:
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
uint16_t cooling_air_get_pos(void)
{
    return 1000UL * cooling_air_valve.position / 600;
}
