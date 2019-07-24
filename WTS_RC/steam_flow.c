/*
 ******************************************************************************
 *
 *  FILE:    steam_flow.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    28/10/2008
 *
 *  DESCRIPTION: Setup of global variables used to define the steam flow rate.
 *              These globals are accessed by the timer interrupt.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */

#include <io430.h>

#include "fail_safe.h"
#include "steam_flow.h"
#include "pio.h"

struct steam_flow steam_flow;   /* Defines the step rate. */

/*
 ******************************************************************************
 *  FUNCTION NAME:          steamflow_init
 *  FUNCTIONAL DESCRIPTION: Set up for steam rate stepper control.
 *  FORMAL PARAMETERS:      Nome
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void steam_flow_init(void)
{
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          steamflow_rate_set
 *  FUNCTIONAL DESCRIPTION: Set up the steamflow_period global variable to
 *                          give steam flow stepper the required step rate.
 *  FORMAL PARAMETERS:      steam_flow_ul_min:
 *                              Required flow rate (micro liters per minute).
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Notes:  2 strokes per second gives a flow rate of 20 ul/minute.
 *          It takes 200 full steps to give one rotation.  As stepper
 *          controller is set up for 8 step pulses per full step, this leads
 *          to 2 * 200 * 8 = 3.2kHz required for 20ml/minute.
 *          8MHz / 3.2kHz = 2500 - minimum period.
 ******************************************************************************
 */
void steam_flowrate_set(uint16_t flowrate)
{
    if(fail_safe_is_failed()){  /* Over ride setting if unit in failsafe mode */
        flowrate = 0;
    }

    if(flowrate == 0){
        /* -- Flag steam flow as disabled.  */
        steam_flow.enabled = 0;
        P5OUT |= P5O7_N_MOTOR2_ENABLE;      /* Disable output driver FETS   */
        P1OUT &= ~P1O2_MOTOR2_STEP;         /* Ensure LED is off.           */
    } else {
        uint32_t tmp;
        tmp = 2500UL * 20000 / flowrate;
        steam_flow.enabled = 1;
        steam_flow.rate_period = tmp >> 16;
        steam_flow.rate_period_fract  = tmp;
        P5OUT &= ~P5O7_N_MOTOR2_ENABLE;     /* Enable output driver FETS.   */
    }
}
