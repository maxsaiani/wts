/*
 ******************************************************************************
 *
 *  FILE:    cooling_air_valve.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    14/11/2008
 *
 *  DESCRIPTION: Interface for control of cooling air valve.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef COOLING_AIR_VALVE_H
#define COOLING_AIR_VALVE_H
#include "stdint.h"

struct cooling_air_valve{
    uint8_t  enabled:1;
    uint8_t  close_valve:1;     /* Reset valve into closed position.    */
    int16_t  position;          /* Dead reckoning of valve position.    */
    int16_t  required_pos;      /* Required valve position.             */
};

extern struct cooling_air_valve cooling_air_valve;

void cooling_air_valve_init(void);
void cooling_air_valve_set(uint8_t enable, int16_t rate_of_opening);
uint16_t cooling_air_get_pos(void);

#define COOL_STEP_SCALE     (0.001) /* 0.1% per minute. */
#define COOL_STEP_RATE      ((int16_t)(1/(COOL_STEP_SCALE*600/60*0.008192)))

#endif   /* #ifndef COOLING_AIR_VALVE_H */
