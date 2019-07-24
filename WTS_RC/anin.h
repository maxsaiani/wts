/*
 ******************************************************************************
 *
 *  FILE:    adc.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    29/10/2008
 *
 *  DESCRIPTION: Interface for MSP430 ADC control.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef ADC_H
#define ADC_H
#include "stdint.h"

#include "wts_comms.h"

/*-- Enumeration of ADC inputs after mapping. */
#define ADC_PROBE1          0
#define ADC_PROBE1_TEMP     1
#define ADC_PROBE2          2
#define ADC_PROBE2_TEMP     3
#define ADC_SPARE1          4
#define ADC_WATER           5
#define ADC_CPU_TEMP        6     /* Internal CPU temperature sensor. */
#define ADC_VCC             7     /* VCC (3V3) supply.                */
#define ADC_MUX             8     /* At end to increase settle time.  */

/*-- Mapping of ADC inputs (via MPC multiplexer sequencer) */
#define INCH_PROBE1         (INCH_5)
#define INCH_PROBE1_TEMP    (INCH_4)
#define INCH_PROBE2         (INCH_6)
#define INCH_PROBE2_TEMP    (INCH_7)
#define INCH_SPARE1         (INCH_1)
#define INCH_WATER          (INCH_0)
#define INCH_CPU_TEMP       (INCH_10)   /* Internal CPU temperature sensor. */
#define INCH_VCC            (INCH_11)   /* Internal VCC div 2.      */
#define INCH_MUX            (INCH_2)

#define ADC_CHANNELS  (8)  /* Number of channels, excluding mux channel    */


/*-- Mapping of the multiplexed ADC readings. */
#define AMUX_24V            0
#define AMUX_POLISH_CURRENT 1
#define AMUX_CONDENSATE_CURRENT 2
#define AMUX_5V             3
#define AMUX_1V2            4
#define AMUX_FILL_CURRENT   5
#define AMUX_PURGE_CURRENT  6
#define AMUX_BOOST_CURRENT  7

#define AMUX_CHANNELS (8)   /* Number of channels multiplexed into the MUX  */
                            /* Input.                                       */

#define CONDUCTIVITY_METER_IRQ_FREQUENCY (1600) /* Interrupt rate (HZ) */
#define CONDUCTIVITY_METER_IRQ_PERIOD \
    ((uint16_t)(8000000UL/CONDUCTIVITY_METER_IRQ_FREQUENCY))

void anin_init(void);
void anin_state_machine(void);
void anin_tirq(void);
void anin_rd_to_comms(struct comms_wts_status *cm_st);
#endif /* #ifndef ADC_H */
