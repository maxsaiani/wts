/*
 ******************************************************************************
 *
 *  FILE:    fail_safe.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    20/11/2008
 *
 *  DESCRIPTION: Evaluation and implementation of fail safe checking.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef FAIL_SAFE_H
#define FAIL_SAFE_H

#include "stdint.h"

void fail_safe_init(void);
void fail_safe_state_machine(void);
uint8_t fail_safe_is_failed(void);

#endif /* ifdef FAIL_SAFE_H */
