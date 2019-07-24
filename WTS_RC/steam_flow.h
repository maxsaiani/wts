/*
 ******************************************************************************
 *
 *  FILE:    steamflow.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    28/10/2008
 *
 *  DESCRIPTION: Interface functiosn for steam flowrate control.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef STEAM_FLOW_H
#define STEAM_FLOW_H

#ifndef STDINT_H
#include "stdint.h"
#endif

struct steam_flow{
    uint8_t  enabled;           /* NZ if steam flow enabled.                */
    uint8_t  rate_period;       /* Stepper pulse rate to give required flow.*/
    uint16_t rate_period_fract; /* Fractional component of above.           */
};
extern struct steam_flow steam_flow;

void steam_flow_init(void);
void steam_flowrate_set(uint16_t flowrate);

#endif /* #ifndef STEAM_FLOW_H */
