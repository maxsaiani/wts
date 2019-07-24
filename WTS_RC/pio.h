/*
 ******************************************************************************
 *
 *  FILE:    pio.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    28/10/2008
 *
 *  DESCRIPTION: Names for IO pins.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef PIO_H
#define PIO_H

#include "stdint.h"

/*-- Give names to the IO bits. */
/* The prefix informs the port, and bit being named.            */
/* P1_0_                               */

#define P1_0_                               (1U<<0)
#define P1O1_MOTOR1_STEP                    (1U<<1)
#define P1O2_MOTOR2_STEP                    (1U<<2)
#define P1O3_WD_TRIG                        (1U<<3)
#define P1I4_LEAK2                          (1U<<4)
#define P1I5_LEAK1                          (1U<<5)
#define P1I6_LEAK4                          (1U<<6)
#define P1I7_LEAK3                          (1U<<7)

#define P2I0_DIN1_TANK_HIGH                 (1U<<0)
#define P2I1_DIN2_TANK_LOW                  (1U<<1)
#define P2I2_DIN3_CONDENSATE_HIGH           (1U<<2)
#define P2I3_DIN4_CONDENSATE_LOW            (1U<<3)
#define P2I4_DIN5_SPARE                     (1U<<4)
#define P2_5_                               (1U<<5)
#define P2_6_                               (1U<<6)
#define P2_7_                               (1U<<7)

#define P3O0_MUX_A0                         (1U<<0)
#define P3O1_MUX_A1                         (1U<<1)
#define P3O2_MUX_A2                         (1U<<2)
#define P3O3_N_TX_ENABLE                    (1U<<3)
#define P3O4_TXD                            (1U<<4)
#define P3I5_RXD                            (1U<<5)
#define P3O6_N_LED_COMMS_STATUS             (1U<<6)
#define P3O7_N_LED_ALARM                    (1U<<7)

#define P4O0_DOUT1_FILL_SOLENOID            (1U<<0)
#define P4O1_DOUT2_PURGE_SOLENOID           (1U<<1)
#define P4O2_DOUT3_PUMP1_FILL               (1U<<2)
#define P4O3_DOUT4_PUMP2_POLISH             (1U<<3)
#define P4O4_DOUT5_PUMP3_CONDENSATE         (1U<<4)
#define P4_5_EXPANSION_PORT_PWM1            (1U<<5)
#define P4O6_PROBE_SAMPLE                   (1U<<6)
#define P4I7_LEAK_DETECTOR                  (1U<<7)

#define P5_0_EXPANSION_PORT_UCLK1           (1U<<0)
#define P5_1_EXPANSION_PORT_SIMO1           (1U<<1)
#define P5_2_EXPANSION_PORT_SOMI1           (1U<<2)
#define P5_3_EXPANSION_PORT_UCLK1           (1U<<3)
#define P5O4_MOTOR1_DIRECTION               (1U<<4)
#define P5O5_N_MOTOR1_ENABLE                (1U<<5)
#define P5O6_MOTOR2_DIRECTION               (1U<<6)
#define P5O7_N_MOTOR2_ENABLE                (1U<<7)

/*-- Analogue inputs.   */
#define P6A0_WATER                          (1U<<0)
#define P6_1_SPARE                          (1U<<1)
#define P6A2_MUXED_INPUTS                   (1U<<2)
#define P6_3_                               (1U<<3)
#define P6A4_PROBE1_TEMP                    (1U<<4)
#define P6A5_PROBE1                         (1U<<5)
#define P6A6_PROBE2_TEMP                    (1U<<6)
#define P6A7_PROBE2                         (1U<<7)

void pio_setup_pin_directions(void);
void pio_setup_pin_interrupts(void);
void pio_din_state_machine(void);
uint16_t pio_din_get(void);

#endif /* #ifndef PIO_H */
