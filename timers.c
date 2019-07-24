/*
 ******************************************************************************
 *
 *  FILE:    timers.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    28/10/2008
 *
 *  DESCRIPTION: Setup, and interrupt handling of timer interrupts.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#include <io430.h>
#include <in430.h>
#include "timers.h"
#include "pio.h"
#include "steam_flow.h"     /* Rate to step steam flow stepper. */
#include "cooling_air_valve.h"
#include "anin.h"
#include "solenoids.h"

volatile uint8_t systick;       /* Incremented once every 8.192ms in TIMERA */

#pragma location="this_code_first"    /* Place near start of flash. */
void timerA_init(void)
{
    TACCR0 = 0;                             // Set TACCR0 count offset
    TACCR1 = CONDUCTIVITY_METER_IRQ_PERIOD; // Set TACCR1 count offset
    TACCR2 = SOLENOID_IRQ_PERIOD;           // For modulation of solenoid outs
    TACCTL0 &= ~CAP;                        // Set Timer Control Register to 
                                            // compare mode 
    TACCTL0  = CCIE;                        // Compare mode, irq enabled.
    TACCTL1  = CCIE;                        // Timer in compare mode, interrupt
    TACCTL2  = CCIE;

    TACTL = MC_2 |                          // Continuous up mode.
            ID_0 |                          // Input divider 0 : CLK/1
            TASSEL_2 |                      // Select system clock as input.
            TAIE;                           // Timer overflow enabled.
}

/*--- Timer B used for PWM generation, not currently used. */
/*    For PWM generation we are fixing at 12 bit.  This gives us an 
 *    output frequency of approx 2kHz. (8MHz / 2^12 = 1953.125)
 */
#pragma location="this_code_first"    /* Place near start of flash. */
void timerB_init(void)
{
    TBR     = 0;
    TBCCR0  = 0;
    TBCCR4  = 0;                            // Initial PWM1 value.
    TBCCTL4 = OUTMOD_7;                     // Reset / set mode.
    TBCCR5  = 0;                            // Initial PWM2 value.
    TBCCTL5 = OUTMOD_7;                     // Reset / set mode.

    TBCTL   = MC_2 |                        // Continuous up mode.
              ID_0 |                        // Input divider 0 : CLK/1
              TBSSEL_2 |                    // System clock as input (8MHz)
              CNTL_1 |                      // 12 Bit counter.
              TBCLR;                        // Initialise the counters.

    /*-- Attach the output pins to the PWM timer function.    */
    //P4SEL   = P4O4_PWM1_ | P4O5_PWM2_;
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          pwm1_set
 *  FUNCTIONAL DESCRIPTION: Set the required PWM duty cycle for PWM1.
 *  FORMAL PARAMETERS:      level : Required level as 12 bit value.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void pwm1_set(uint16_t level){
    if(level >= (1<<12)){   /* Overflow? */
        level = (1<<12) - 1;
    }
    TBCCR4 = level;
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          pwm2_set
 *  FUNCTIONAL DESCRIPTION: Set the required PWM duty cycle for PWM2.
 *  FORMAL PARAMETERS:      level : Required level as 12 bit value.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void pwm2_set(uint16_t level){
    if(level >= (1<<12)){   /* Overflow? */
        level = (1<<12) - 1;
    }
    TBCCR5 = level;
}

#pragma vector=TIMERA0_VECTOR   /* CCR0 Interrupt vector. */
#pragma location="this_code_first"    /* Place near start of flash. */
/*--- Used to control the steam flow rate stepper motor. */
/*    Look at steam_flow.c for second half of the story. */
static __interrupt void TIMER0_interupt_handler(void)
{
    static uint8_t cycles;
    /*--- Step water flow pump if required. */
    if(steam_flow.enabled){                 /* Should be delivering steam?  */
        if(cycles != 0){                    /* Still whole 8mS cycle to go? */
            cycles--;
        } else {                            /* Finished count down. */
            TACCR0 += steam_flow.rate_period_fract;/* First schedule next irq */
            cycles = steam_flow.rate_period;    /* Then rqd cycle count.     */
            P1OUT ^= P1O2_MOTOR2_STEP;          /* Toggle stepper output bit.*/
        }
    } else {
        cycles = 0;
    }
}

#pragma vector=TIMERA1_VECTOR
#pragma location="this_code_first"      /* Place near start of flash.       */
static __interrupt void TIMER1_interupt_handler(void)
{
    switch(__even_in_range(TAIV, 10)){  /* MSP430 Wacky interrupt vector.   */
        case TAIV_CCIFG1:               /* Capture/compare 1                */
            TACCR1 += CONDUCTIVITY_METER_IRQ_PERIOD;
            anin_tirq();                /* Synchronous ADC sampling.        */
            break;

        case TAIV_CCIFG2:               /* Capture/compare 2.               */
            TACCR2 += SOLENOID_IRQ_PERIOD;  /* Schedule next interrupt. */

            /* Switch on or PWM modulate solenoid outputs. */
            if(fill_solenoid.on){
              if(fill_solenoid.pre_pwm_count == 0){
                  //  P4OUT ^= P4O0_DOUT1_FILL_SOLENOID;
                  P4OUT |= P4O0_DOUT1_FILL_SOLENOID;
              } else {
                  fill_solenoid.pre_pwm_count--;
                  P4OUT |= P4O0_DOUT1_FILL_SOLENOID;
              }
            } else {
                P4OUT &= ~P4O0_DOUT1_FILL_SOLENOID;
            }

            if(purge_solenoid.on){
              if(purge_solenoid.pre_pwm_count == 0){
                    P4OUT ^= P4O1_DOUT2_PURGE_SOLENOID;
              } else {
                  purge_solenoid.pre_pwm_count--;
                  P4OUT |= P4O1_DOUT2_PURGE_SOLENOID;
              }
            } else {
                P4OUT &= ~P4O1_DOUT2_PURGE_SOLENOID;
            }
            break;

        case TAIV_TAIFG:            /* Timer overflow.                  */
            /*--- Control of cooling air valve. */
            P1OUT &= ~P1O1_MOTOR1_STEP;         /* End of stepping pulse.*/

            if(cooling_air_valve.close_valve){
                if(--cooling_air_valve.position == 0){
                    cooling_air_valve.close_valve = 0;/* Have reseated valve. */
                    P5OUT |= P5O5_N_MOTOR1_ENABLE;
                } else {
                    P5OUT &= ~(P5O5_N_MOTOR1_ENABLE|P5O4_MOTOR1_DIRECTION);
                    P1OUT |= P1O1_MOTOR1_STEP;     /* Stepping pulse.  */
                }
            } else if(cooling_air_valve.enabled){
                int rqd_pos = cooling_air_valve.required_pos;
                int pos     = cooling_air_valve.position;

                if(rqd_pos < pos){
                    if(pos <= 0){
                        /*-- Disabled at end of travel. */
                        P5OUT |= P5O5_N_MOTOR1_ENABLE;
                    } else {
                        /*-- One step toward closed position. */
                        cooling_air_valve.position--;
                        P5OUT &= ~(P5O5_N_MOTOR1_ENABLE|P5O4_MOTOR1_DIRECTION);
                        P1OUT |= P1O1_MOTOR1_STEP;     /* Stepping pulse.  */
                    }
                } else if (rqd_pos > pos){
                    if(pos >= 600){
                        /*-- Disabled at end of travel. */
                        P5OUT |= P5O5_N_MOTOR1_ENABLE;
                    } else {
                        /*-- One step toward open position. */
                        cooling_air_valve.position++;
                        P5OUT &= ~P5O5_N_MOTOR1_ENABLE;
                        P5OUT |= P5O4_MOTOR1_DIRECTION;
                        P1OUT |= P1O1_MOTOR1_STEP;     /* Stepping pulse.  */
                    }
                }
            }
            systick++;              /* Update timing source.            */
            break;                  /* RTC timing is based on this.     */
    }
}

#pragma vector = TIMERB0_VECTOR
static __interrupt void timerB0_interupt_handler( void )
{
}
    
#pragma vector = TIMERB1_VECTOR
static __interrupt void timerB1_interupt_handler( void )
{
}
