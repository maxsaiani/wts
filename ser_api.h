/*
*********************************************************************************
*
*  FILE:    ser_api.h
*
*  AUTHOR:  Stephen Fischer
*           Chris Gibson
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the serial communications class
*               for the Distributed IO System for the CHP3/NetGen.
*               Modified for use with the "gsebus"
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005, 2008
*
*********************************************************************************
*/

#ifndef __SER_API_H__
#define __SER_API_H__

/******************************************************************************/
#include "stdint.h"

void  ser_init(void);
void  ser_state_machine(void);
void  ser_start_transmission(void);

#endif /* __SER_API_H__ */
