/*
********************************************************************************
*
*  FILE:    $(Dios)/rtc.c
*
*  AUTHOR:  Stephen Fischer
*           Chris Gibson
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the real time clock class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*
*  Modified from DIOS code 2008
********************************************************************************
*/
#include <io430.h>

#include "utility.h"
#include "cfcl.h"
#include "wdg_api.h"
#include "rtc_api.h"
#include "pio.h"
#include "wdg.h"

uint16_t rtc_timer[rtc_maxTimers];
/******************************************************************************/

/* The number of ticks per minute is 60s/8.192ms = 7324.21875.
** This means that after 32 minutes there should be 234375 ticks.
** Using integers there would only be 32*7324 = 234368 ticks.
** i.e. without adjustment rtc would loose about 57ms every half hour.
** Therefore using 7324 ticks per minute, every 32 minutes add 7 ticks, or
** every four minutes add a tick, except the last 4 of the 32. For example:
**      Min     Start   Stop    Count   Real
**      0       1       7325    7324     7324.21875
**      1       1       7325    14648   14648.4375
**      2       1       7325    21972   21972.65625
**      3       0       7325    29297   29296.875
*/
#define ticksPerMin     7325

/******************************************************************************/
typedef struct {
    unsigned short nTicks;  /* number of 8.192ms ((2^16)/8MHz) ticks */
    unsigned long  nMinutes;/* This gives 4083 years to overflow. 
                            ** Do not have to worry about overflow in calculations
                            */
    } rtc_uptimes;

rtc_uptimes uptime;

static uint8_t last_systick;    /* Follows systick, in rtc_state_machine    */

/*
 ******************************************************************************
 *  FUNCTION NAME:          rtc_init
 *  FUNCTIONAL DESCRIPTION: Set up variables used by rtc_statemachine
 *                          And reset all the software timers.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
#pragma location="this_code_first"    /* Place near start of flash. */
void rtc_init(void)
{
    int index;

    systick = 0;
    last_systick = 0;

    uptime.nTicks   = 1;
    uptime.nMinutes = 0;
    
    for (index = 0; index < rtc_maxTimers; ++index){
        rtc_timer[ index] = 0;
    }
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          rtc_state_machine
 *  FUNCTIONAL DESCRIPTION: State machine for timer updates, including the
 *                          real time clock.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           Updates global structures: rtc_timer & uptime
 ******************************************************************************
 */
void rtc_state_machine(void)
{
    uint16_t index;
    while(last_systick != systick){  /* For each systick passed. */
        last_systick++;

        /*--- Update count down timers */
        for(index =0; index < rtc_maxTimers; index++){
            if(rtc_timer[index] != 0){
                rtc_timer[index]--;
            }
        }
        /*--- Update tick and minute counters */
        {
            short ticks = uptime.nTicks + 1;
            if (ticks >= ticksPerMin)
            {
                char temp = (char) ++uptime.nMinutes;
                
                ticks = 1;
                if (((temp & 0x03) == 0) && 
                    ((temp & 0x1f) != 0)  )
                    ticks = 0;
            }
            uptime.nTicks = ticks;
        }
        /*--- Pat the dog. */
        wdg_swModule(wdg_rtc);
        wdg_hwTickle();

        /*--- Digital input filtering. */
        pio_din_state_machine();
    }
}

/******************************************************************************/
void rtc_getUpTime(unsigned short *ptr)
{
    *ptr++ = uptime.nTicks;
    *ptr++ = uptime.nMinutes;
    *ptr++ = uptime.nMinutes >> 16;
}

/******************************************************************************/
void rtc_timeStamp( char *str) 
{
    uint16_t tick, tock, tick2;
    uint16_t min; /* approx 45 days */
    
    tick  = systick;
    tock = TAR;
    tick2 = systick;
    if(!(tock & (1U<<15))){  /* Filter against race conditions.  */
        tick = tick2;
    }
    min  = (short) uptime.nMinutes;
    
    putShort(str, min ); putChar (str, ':');
    putByte(str, tick);  putChar (str, '.');
    putShort(str, tock); putChar (str, '>');
    putNull(str);
}

/******************************************************************************/
short rtc_watchTockStart( void) 
{
    return TAR;
}

/******************************************************************************/
short rtc_watchTockStop( short start) 
{
    unsigned short delta = TAR - (unsigned)start;
    
    return delta;
}

/******************************************************************************/
short rtc_watchTickStart( void) 
{
    return uptime.nTicks;
}

/******************************************************************************/
short rtc_watchTickStop( unsigned short start) 
{
    short delta = uptime.nTicks - start;
    
    if (delta < 0)
        delta += ticksPerMin;
    return delta;
}

/******************************************************************************/
short rtc_watchMinuteStart( void) 
{
    return uptime.nMinutes;
}

/******************************************************************************/
short rtc_watchMinuteStop( unsigned short start) 
{
    return ((short unsigned)uptime.nMinutes) - start;
}

/*******************************************************************************/
/* end of rtc.c */
