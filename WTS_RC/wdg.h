/*
*********************************************************************************
*
*  FILE:    $(Dios)/wdg.h
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the watch dog class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*  NOTE: Changing this file means the software is no longer backwardly compatable
*
*********************************************************************************
*/

#ifndef __WDG_H__
#define __WDG_H__

#include "pio.h"

//#pragma memory=dataseg(LDATA)

/* actually wdg_modules, but maximises future expansion */
typedef unsigned short wdg_moduleMasks;
 
__no_init extern wdg_moduleMasks wdg_moduleMask;

#pragma memory=default

void wdg_init( wdg_moduleMasks testMask); 
void wdg_swTickle( void);

/* port bit setup in dio */
#define wdg_hwTickle()       do{P1OUT ^= P1O3_WD_TRIG;}while(0)

#define wdg_swModule( value) wdg_moduleMask |= value

#define wdg_start()          WDTCTL = WDT_ARST_1000
#define wdg_stop()           WDTCTL = WDT_ARST_1000 | WDTHOLD

#endif /* __WDG_H__ */
