/*
*********************************************************************************
*
*  FILE:    $(Dios)/wdg_api.h
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
*  NOTE: Changing this file may make the software no longer backwardly compatable
*
*********************************************************************************
*/

#ifndef __WDG_API_H__
#define __WDG_API_H__

#include "wdg.h"

typedef enum {
    wdg_freshMask   = 0x0000,  /* Must be zero */
    wdg_rtc         = 0x0001,  /* Must be here */
    wdg_null        = 0x0002,
    wdg_pid         = 0x0004,
    wdg_adc         = 0x0008,
    wdg_tcs         = 0x0010,  /* diosA */
    wdg_lui         = 0x0010,  /* diosB */
    wdg_spare2      = 0x0020,
    wdg_spare1      = 0x0040,
    wdg_spare0      = 0x0080,
    wdg_all         = 0x001f   /* does not include the spares */
    } wdg_modules;
    
#endif /* __WDG_API_H__ */
