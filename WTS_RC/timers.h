/*
 ******************************************************************************
 *
 *  FILE:    timers.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    29/10/2008
 *
 *  DESCRIPTION: Interface functions for MPC430 timer control
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */

#include "stdint.h"
extern volatile uint8_t systick;  /* Incremented once every 8.192ms in TIMERA */
void timerA_init(void);
void timerB_init(void);
void pwm1_set(uint16_t level);
void pwm2_set(uint16_t level);
