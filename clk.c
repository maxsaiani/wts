/*
*********************************************************************************
*
*  FILE:    $(Dios)/clk.c
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the system clock class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*********************************************************************************
*/
#include <io430.h>
#include "utility.h"
#include "cfcl.h"
#include "clk_api.h"
#include "wdg_api.h"

#define clkSetupFailure 10
short clk_startup;

/******************************************************************************/
#pragma location="this_code_first"    /* Place near start of flash. */
void clk_init(void) 
{
    /*burnCPUCycles( 200); / allow the osc to recover on slow power up */

    /* (111-00000) 
       DCO =111 => no mod and highest possible freq. selected */
    DCOCTL  = 0xE0;
    
    /* (0-0-00-0-111) 
       XT2osc is ON, 
       Rsel=111 => int Res for highest nominal freq. selected */
    BCSCTL1 = 0x07;
    
    /* (01-00-0-00-0) 
       MCLK=DCOCLK/1, 
       SMCLK=DCOCLK/1, 
       int res = ON (RC clock is running) */
    BCSCTL2 = 0x40;
    
    /* wait 10ms until XT2 osc settles */
    burnCPUCycles( 10);
    
    /*clear osc fault flag and no interrupts */ 
    IE1  &= ~0x02;
    clk_startup = 0;
    do {
        IFG1 &= ~0x02;
        if (++clk_startup > clkSetupFailure)
            wdg_start(); /* hope to stop board failing to power up */
    } while (IFG1 & 0x02);
    
    /* (10-00-1-00-0) 
       MCLK=XT2CLK/1, 
       SMCLK=XT2CLK/1, 
       int res = ON (RC clock is running) */
    /*BCSCTL2 = 0x98; / run at 4MHz */
    BCSCTL2 = 0x88;   /* run at 8MHz */
}

/********************************************************************************
NOTE :- Language extensions enabled for Interrupt Service Routines.
        Only ISR functions should appear after this comment block.
        The compiler works out how many registers are saved for the 
        interrupt context switch.
     :- Do not use multiplication!
********************************************************************************/
#pragma language=extended

/*******************************************************************************/
/* done in compiler routine Cstart
interrupt [RESET_VECTOR] void clk_resetIsr( void) 
{
}
*/

/*******************************************************************************/
/* end of clk.c */
