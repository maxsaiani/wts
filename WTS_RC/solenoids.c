/*
 ******************************************************************************
 *
 *  FILE:    solenoids.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    18/11/2008
 *
 *  DESCRIPTION: Interface for solenoid control.
 *              Solenoids are pulled in with a constant on voltage, then
 *              held in place with a PWM modulated signal
 *              Most of the real work is done in a timer interrupt, see
 *              timer.c
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#include "solenoids.h"
#include "fail_safe.h"
struct solenoid fill_solenoid;
struct solenoid purge_solenoid;

void fill_solenoid_set(uint8_t on)
{
    if(fail_safe_is_failed()){  /* Over ride setting if unit in failsafe mode */
        on = 0;
    }

    if(on && !fill_solenoid.on){    /* Just turned on.  */
        /*--- Constant DC voltage for 5 seconds before PWM modulation. */
        fill_solenoid.pre_pwm_count = 5 * SOLENOID_IRQ_FREQUENCY;
    }
    fill_solenoid.on = on;
}

void purge_solenoid_set(uint8_t on)
{
    if(fail_safe_is_failed()){  /* Over ride setting if unit in failsafe mode */
        on = 0;
    }

    if(on && !purge_solenoid.on){    /* Just turned on.  */
        /*--- Constant DC voltage for 5 seconds before PWM modulation. */
        purge_solenoid.pre_pwm_count = 5 * SOLENOID_IRQ_FREQUENCY;
    }
    purge_solenoid.on = on;
}
