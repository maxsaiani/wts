/*
*********************************************************************************
*
*  FILE:    $(Dios)/wdg.c
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
********************************************************************************
*/

#include <io430.h>

#include "cfcl.h"

#include "wdg_api.h"

__no_init wdg_moduleMasks wdg_testMask;
__no_init wdg_moduleMasks wdg_moduleMask;

/******************************************************************************/

#define wdg_memMask0 0xaa55
#define wdg_memMask1 0x55aa

__no_init wdg_moduleMasks wdg_memPattern0;
__no_init wdg_moduleMasks wdg_memPattern1;


/******************************************************************************/
void wdg_init( wdg_moduleMasks testMask) 
{
    /* must not use init data */

    wdg_hwTickle();
    wdg_stop();
    
    wdg_moduleMask = wdg_freshMask;    
    wdg_testMask   = testMask;    

    wdg_memPattern0 = wdg_memMask0;
    wdg_memPattern1 = wdg_memMask1;
}

/******************************************************************************/
void wdg_swTickle( void) 
{
    if (wdg_moduleMask == wdg_testMask)
    {
        /* This takes into acount stuck low and high bits in a word, 
        ** as well as run away code.
         */
        if ((wdg_memPattern0 == wdg_memMask0) &&
            (wdg_memPattern1 == wdg_memMask1) )
        {
            wdg_moduleMask = wdg_freshMask;
            wdg_start();
        }
    }
}

/******************************************************************************/
/* end of wdg.c */
