/*
 ******************************************************************************
 *
 *  FILE:    anin.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    18/11/2008
 *
 *  DESCRIPTION: Processing of the analogue inputs (ADC readings).
 *              Part of the ADC processing is done in conductivity.c, owing
 *              to the need to synchronise the ADC converter with the
 *              conductivity circuit.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */

#include <string.h>
#include <io430.h>
#include <in430.h>
#include <string.h>
#include "stdint.h"

#include "utility.h"
#include "anin.h"
#include "pio.h"

static uint16_t anin_av  [ADC_CHANNELS];   /* Copy of averaged readings.   */
static uint32_t anin_acum[ADC_CHANNELS];   /* Accumulation for averages.   */
static uint16_t amux_av  [AMUX_CHANNELS];
static uint32_t amux_acum[AMUX_CHANNELS];
static uint16_t sequence_counter;           /* Sequence counter.            */
static uint16_t overflows;

/*-- Macro magic to turn a channel number into a register name. */
/*   ADC_CHNL(ADC_PROBE1) should become ADC12MEM0               */
#define __CAT__(x,y) x##y
#define ADC_CHNL__(x) __CAT__(ADC12MEM,x)
#define ADC_CHNL(x) ADC_CHNL__(x)

/*-- Use same ID to give interrpt mask. */
#define ADC12IE_MSK(x) (1<<x)

/*
 ******************************************************************************
 *  FUNCTION NAME:          anin_init
 *  FUNCTIONAL DESCRIPTION: set up for state variables for analogue input
 *                          filtering.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void anin_init(void)
{
    memset(anin_av, 0, sizeof(anin_av));
    
    /*-- Set up ADC */
    /*
     * > Reference of 2.5V
     * > 16 clock cycle sample and hold time for both upper and lower 8 channel
     *   groups.
     */
    ADC12CTL0 = MSC | ADC12ON | REFON | REF2_5V | SHT0_2 | SHT1_2;

    /*
     * > When conversions are started the sequence starts at ADC ctrl/mem idx 0
     * > ADC does one conversion sequence then stops.
     * > SMCK / 8 selected as clock source for ADC.
     */
    ADC12CTL1 = CONSEQ_1 | ADC12SSEL_3 | ADC12DIV_7 | SHP;

    /*
     * Define conversion sequence.
     * All use Vr+=Vref and Vr-
     */
    ADC12MCTL0 = SREF_5|INCH_PROBE1;
    ADC12MCTL1 = SREF_5|INCH_PROBE1_TEMP;
    ADC12MCTL2 = SREF_5|INCH_PROBE2;
    ADC12MCTL3 = SREF_5|INCH_PROBE2_TEMP;
    ADC12MCTL4 = SREF_5|INCH_SPARE1;
    ADC12MCTL5 = SREF_5|INCH_WATER;
    ADC12MCTL6 = SREF_5|INCH_CPU_TEMP;
    ADC12MCTL7 = SREF_5|INCH_VCC;
    ADC12MCTL8 = SREF_5|INCH_MUX       | EOS;
    
    ADC12CTL0 |= ENC;               /* Enable ADC converter.        */

    /*
     * Enable interrupt for external multiplexer conversion complete.
     */
    ADC12IE = ADC12IE_MSK(ADC_MUX);
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          anin_state_machine
 *  FUNCTIONAL DESCRIPTION: If averages are
 *                          Once 256 readings are accumulated transfer the
 *                          averages across.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void anin_state_machine(void)
{
    /*-- This code was moved to the interrupt. */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          anin_rd_to_comms
 *  FUNCTIONAL DESCRIPTION: Copy readings to comms output structure.
 *  FORMAL PARAMETERS:      Pointer to comms status to copy results to.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void anin_rd_to_comms(struct comms_wts_status *cm_st)
{
    cm_st->anin_filt_overflows      = overflows;
    cm_st->cpu_temperature = 
        ((int32_t)anin_av[ADC_CPU_TEMP]-1614)*70400L/4095;
    cm_st->anin_probe1_conductivity = anin_av[ADC_PROBE1];
    cm_st->anin_probe1_temperature  = anin_av[ADC_PROBE1_TEMP];
    cm_st->anin_probe2_conductivity = anin_av[ADC_PROBE2];
    cm_st->anin_probe2_temperature  = anin_av[ADC_PROBE2_TEMP];
    cm_st->anin_spare1              = anin_av[ADC_SPARE1];
    cm_st->anin_water_meter         = anin_av[ADC_WATER];
    cm_st->anin_3V6                 = anin_av[ADC_VCC];
    cm_st->resurved                 = 0xffff;

    cm_st->anin_24V                 = amux_av[AMUX_24V];
    cm_st->anin_polish_current      = amux_av[AMUX_POLISH_CURRENT];
    cm_st->anin_condensate_current  = amux_av[AMUX_CONDENSATE_CURRENT];
    cm_st->anin_5V                  = amux_av[AMUX_5V];
    cm_st->anin_1V2                 = amux_av[AMUX_1V2];
    cm_st->anin_fill_current        = amux_av[AMUX_FILL_CURRENT];
    cm_st->anin_purge_current       = amux_av[AMUX_PURGE_CURRENT];
    cm_st->anin_boost_current       = amux_av[AMUX_BOOST_CURRENT];
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          anin_state_machine
 *  FUNCTIONAL DESCRIPTION: Accumulate averages for analogue readings.
 *                          Once 256 readings are accumulated, wait for the
 *                          main line to copy the averages accross.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           
 *  Notes:                  Called by timer interrupt.
 ******************************************************************************
 */
void anin_tirq(void)
{
    /*-- Take conductivity readings and perform conductivity signal     */
    /*   modulation.    */
    uint16_t sc;    /* Copy of sequence counter. */
    sc = sequence_counter;
    switch(sc & 3){
        case 0:
            P4OUT |= P4O6_PROBE_SAMPLE;
            break;
        case 2:
            anin_acum[ADC_PROBE1] += ADC_CHNL(ADC_PROBE1);
            anin_acum[ADC_PROBE2] += ADC_CHNL(ADC_PROBE2);
            /*-- Drain conductivity probe caps.        */
            P4OUT &= ~P4O6_PROBE_SAMPLE;
            break;
    }
    /*-- Accumulate averages of the non-synchronous ADC results */
    anin_acum[ADC_PROBE1_TEMP]  += ADC_CHNL(ADC_PROBE1_TEMP);
    anin_acum[ADC_PROBE2_TEMP]  += ADC_CHNL(ADC_PROBE2_TEMP);
    anin_acum[ADC_SPARE1]       += ADC_CHNL(ADC_SPARE1);
    anin_acum[ADC_WATER]        += ADC_CHNL(ADC_WATER);
    anin_acum[ADC_CPU_TEMP]     += ADC_CHNL(ADC_CPU_TEMP);
    anin_acum[ADC_VCC]          += ADC_CHNL(ADC_VCC);
    amux_acum[sc % 8] += ADC_CHNL(ADC_MUX);

    sequence_counter = ++sc;    
    ADC12CTL0 |= ADC12SC;       /* Trigger next conversion. */

    if(sc % 256 == 0){
        anin_av[ADC_PROBE1_TEMP] = anin_acum[ADC_PROBE1_TEMP] / 256;
        anin_acum[ADC_PROBE1_TEMP] = 0;
        anin_av[ADC_PROBE2_TEMP] = anin_acum[ADC_PROBE2_TEMP] / 256;
        anin_acum[ADC_PROBE2_TEMP] = 0;
        anin_av[ADC_SPARE1]      = anin_acum[ADC_SPARE1] / 256;
        anin_acum[ADC_SPARE1] = 0;
        anin_av[ADC_WATER]       = anin_acum[ADC_WATER] / 256;
        anin_acum[ADC_WATER] = 0;
        anin_av[ADC_CPU_TEMP]    = anin_acum[ADC_CPU_TEMP] / 256;
        anin_acum[ADC_CPU_TEMP] = 0;
        anin_av[ADC_VCC]    = anin_acum[ADC_VCC] / 256;
        anin_acum[ADC_VCC] = 0;
        if(sc % 1024 == 0){
            anin_av[ADC_PROBE1] = anin_acum[ADC_PROBE1] / 256;
            anin_acum[ADC_PROBE1] = 0;
            anin_av[ADC_PROBE2] = anin_acum[ADC_PROBE2] / 256;
            anin_acum[ADC_PROBE2] = 0;
            if(sc % 2048 == 0){          
                /* -- Unrolled loop. */
                amux_av[0] = amux_acum[0] / 256;
                amux_acum[0] = 0;
                amux_av[1] = amux_acum[1] / 256;
                amux_acum[1] = 0;
                amux_av[2] = amux_acum[2] / 256;
                amux_acum[2] = 0;
                amux_av[3] = amux_acum[3] / 256;
                amux_acum[3] = 0;
                amux_av[4] = amux_acum[4] / 256;
                amux_acum[4] = 0;
                amux_av[5] = amux_acum[5] / 256;
                amux_acum[5] = 0;
                amux_av[6] = amux_acum[6] / 256;
                amux_acum[6] = 0;
                amux_av[7] = amux_acum[7] / 256;
                amux_acum[7] = 0;
            }
        }
    }
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          anin_irq
 *  FUNCTIONAL DESCRIPTION: End of ADC interrupt, update address of multiplexed
                            input, and accumulated average totals.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Note:                   The only ADC12 interrupt that we enable is the
 *                          one for when the MUX conversion is complete.
 *                          This allows us to update the multiplexer as soon
 *                          as possible to allow more settling time.
 ******************************************************************************
 */
#pragma vector=ADC_VECTOR
#pragma location="this_code_first"    /* Place near start of flash. */
static __interrupt void ADC_interupt_handler(void)
{
    ADC_CHNL(ADC_MUX);  /* Dummy read to clear interrupt.   */
    P3OUT = (P3OUT & ~ 7) | ((sequence_counter + 1) & 7);
}
