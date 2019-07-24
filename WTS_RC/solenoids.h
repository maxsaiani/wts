/*
 ******************************************************************************
 *
 *  FILE:    solenoids.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    18/11/2008
 *
 *  DESCRIPTION: Driver for solenoid control
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 *
 *  Note:   Most of the real work is done in a timer interrupt (see timers.c)
 *
 ******************************************************************************
 */
#ifndef SOLENOIDS_H
#define SOLENOIDS_H

#include "stdint.h"

struct solenoid{
    uint8_t  on;                /* Current solenoid state.          */
    uint16_t pre_pwm_count;     /* Count down of constant on time   */
};

extern struct solenoid fill_solenoid;
extern struct solenoid purge_solenoid;

#define SOLENOID_IRQ_FREQUENCY (4000)   /* 4kHz interrupt */
#define SOLENOID_IRQ_PERIOD ((uint16_t)(8000000UL / SOLENOID_IRQ_FREQUENCY)) 

void fill_solenoid_set(uint8_t on);
void purge_solenoid_set(uint8_t on);

#endif /* #ifndef SOLENOIDS_H */
