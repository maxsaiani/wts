/*
*********************************************************************************
*
*  FILE:    $(Dios)/utility.c
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains varrious utility routines
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*  NOTE: Changing this file means the software is no longer backwardly compatable
*
*********************************************************************************
*/

#include "cfcl.h"
#include "utility.h"

/*******************************************************************************/
void burnCPUCycles(volatile int ms) 
{
/* at 8MHz, 8000/6 is about one millisecond */
/* at 1MHz, 1333 is about eight millisecond */
#define milliSecondLoopCount 1333

    volatile short i;
    do{
        for (i=0; i < milliSecondLoopCount; ++i);
    } while (--ms);
}

/*******************************************************************************/

#define nibbleToHex( n) if ((n = '0' | (n & 0x0f)) > '9') n += ('@'-'9');

/*******************************************************************************/
char *putTheByte( char *str, unsigned char value)
{
    unsigned char tmp;
    tmp = value >> 4; nibbleToHex( tmp); putChar( str, tmp);
    tmp = value     ; nibbleToHex( tmp); putChar( str, tmp);
    return str;
}

/*******************************************************************************/
char *putTheShort( char *str, unsigned short value)
{
    putByte( str, (value >> 8));
    putByte( str, value);
    return str;
}

/*******************************************************************************/
/* end of utility.c */
