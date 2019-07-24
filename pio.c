/*
 ******************************************************************************
 *
 *  FILE:    pio.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    28/10/2008
 *
 *  DESCRIPTION: Interface for input output port setup, and input denouncing.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 *  Note: Ports are accessed directly by functions that use the pins.
 *  This is just to cater for pio functions of general nature.
 *
 ******************************************************************************
 */
#include <io430.h>
#include <in430.h>
#include "stdint.h"
#include "pio.h"
#include "flt_api.h"
#include "wts_comms.h"

static uint16_t dins = 0;   /* Filtered digital inputs. */

/*
 ******************************************************************************
 *  FUNCTION NAME:          setupPorts
 *  FUNCTIONAL DESCRIPTION: Initial port level and direction setup.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           
 *  Note:   The PxSEL, and interrupt control bits are not touched.  It is 
 *          the responsibility of the associated peripheral driver to set
 *          these up if required.
 ******************************************************************************
 */
#pragma location="this_code_first"    /* Place near start of flash. */
void pio_setup_pin_directions(void)
{
    /*--- Port 1    */
    P1OUT   = 0x00;     /* Initial state of outputs.                    */
    P1DIR = 
        P1_0_ |
        P1O1_MOTOR1_STEP |
        P1O2_MOTOR2_STEP |
        P1O3_WD_TRIG;

    /*--- Port 2 (Use default reset setup for port- all inputs)         */
    P2OUT   = 0x00;     /* Initial state of outputs.                    */
    P2DIR   = P2_5_ |P2_6_ |P2_7_;/* Drive turn unconnected pins into outputs.*/
 
    /*--- Port 3    */
    P3OUT =             /* Initial state of outputs.                    */
        P3O3_N_TX_ENABLE |
        P3O6_N_LED_COMMS_STATUS |
        P3O7_N_LED_ALARM;

    P3DIR =             /* Pins required to be outputs.                 */
        P3O0_MUX_A0 | P3O1_MUX_A1 | P3O2_MUX_A2 |
        P3O3_N_TX_ENABLE  | P3O4_TXD |
        P3O6_N_LED_COMMS_STATUS |  P3O7_N_LED_ALARM;

    /*--- Port 4    */
    P4OUT = 0x00;       /* Initial state of outputs.                    */
    P4DIR = 
        P4O0_DOUT1_FILL_SOLENOID    |
        P4O1_DOUT2_PURGE_SOLENOID   |
        P4O2_DOUT3_PUMP1_FILL       |
        P4O3_DOUT4_PUMP2_POLISH     |
        P4O4_DOUT5_PUMP3_CONDENSATE |
        P4_5_EXPANSION_PORT_PWM1    |
        P4O6_PROBE_SAMPLE;

    /*--- Port 5    */
    P5OUT  = 0x00;      /* Set all bits on Port 5 low                   */
    P5DIR  = 0xFF;      /* Set all bits on Port 5 as outputs            */

    /*--- Port 6 used as ADC inputs. */
    P6SEL = 0xFF;       /* Disable digital input circuits.  */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          pio_dio_state_machine
 *  FUNCTIONAL DESCRIPTION: Debounce filtering on digital inputs.
 *                          This should be called once every systick.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void pio_din_state_machine(void)
{
    uint8_t p2, p4;
    uint16_t p1;
    p1 = P1IN;
    p2 = P2IN;
    p4 = P4IN;
    
    dins = flt_debounce(
            (p2 & 
                (P2I0_DIN1_TANK_HIGH | P2I1_DIN2_TANK_LOW | 
                 P2I2_DIN3_CONDENSATE_HIGH | P2I3_DIN4_CONDENSATE_LOW |
                 P2I4_DIN5_SPARE) ) |
            (p4 & P4I7_LEAK_DETECTOR) |
            (((p1 & 
               (P1I4_LEAK2 | P1I5_LEAK1 | P1I6_LEAK4 | P1I7_LEAK3 )) << 8)));
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          pio_din_get
 *  FUNCTIONAL DESCRIPTION: Return debounce filtered digital inputs.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
uint16_t pio_din_get(void)
{
    union {
        struct comms_wts_status_bits bit;
        uint16_t bits;
    } ret;
    
    ret.bits = dins & 0x1f;  /* First 5 bits mapped 1 to 1 */

    /* Don't let the Port bit names misslead you. */
    /* LEAK2 is for level 1 etc. */    
    ret.bit.Leak1 = !(dins & (P1I4_LEAK2 << 8));
    ret.bit.Leak2 = !(dins & (P1I5_LEAK1 << 8));
    ret.bit.Leak3 = !(dins & (P1I6_LEAK4 << 8));
    ret.bit.Leak4 = !(dins & (P1I7_LEAK3 << 8));
    ret.bit.LeakDetectPresent = !(dins & P4I7_LEAK_DETECTOR);
    
    /*-- Remapped bits. */
    return ret.bits;
}
