
/*
*********************************************************************************
*
*  FILE:    $(Dios)/flt.c
*
*  AUTHOR:  Stephen Fischer
*  MODS:    Chris Gibson
*
*  DATE:    01/11/05
*  MOD DATE:17/11/2008
*
*  DESCRIPTION: This file contains the digital filter class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*********************************************************************************
*/

#include "cfcl.h"

#include "flt_api.h"

/*
#define unitTestFunction 1
*/
#ifdef unitTestFunction
#define nHistoryBits 3
#else
#define nHistoryBits 5
#endif

#define bitMask      ((1 << nHistoryBits) - 1)
#define nBits        16


typedef struct {
    unsigned char history;
    unsigned char stable;
    } flt_datas;

flt_datas flt_data[ nBits];


/*******************************************************************************/
void flt_init( void)
{ 
    unsigned short index;

    for (index=0; index < nBits; ++index)
    {
        flt_data[index].history = 0;
        flt_data[index].stable  = 0;
    }
}

/*******************************************************************************/
unsigned short flt_debounce( unsigned short bits)
{
    flt_datas *ptr = &flt_data[0];
    unsigned short index  = 1;
    unsigned short result = 0;
    
    do {
        unsigned short temp;
        temp = ptr->history;
        temp <<= 1;

        if (bits & index){
            temp |= 1;
        }

        temp &= bitMask;

        ptr->history = temp;

        if(temp == 0 || temp == bitMask){
            ptr->stable = temp;
        }

        if(ptr->stable != 0){
            result |= index;
        }

        ++ptr;
    } while (index <<= 1);
    
    return result;
}

/******************************************************************************/

#ifdef unitTestFunction

void flt_unitTest( void)
{
    unsigned short value;
    
    value = flt_debounce( 0x8181);    
    /* v=0x0000 */ value |= 0x2000;
    value = flt_debounce( 0x8081);    
    /* v=0x0000 */ value |= 0x2000;
    value = flt_debounce( 0x8101);    
    /* v=0x8001 */ value |= 0x2000;
    value = flt_debounce( 0x8180);    
    /* v=0x8001 */ value |= 0x2000;
    value = flt_debounce( 0x8181);    
    /* v=0x8101 */ value |= 0x2000;
    value = flt_debounce( 0x4000);    
    /* v=0x8101 */ value |= 0x2000;
    value = flt_debounce( 0x4181);    
    /* v=0x8101 */ value |= 0x2000;
    value = flt_debounce( 0x4080);    
    /* v=0x4101 */ value |= 0x2000;
    value = flt_debounce( 0x4181);    
    /* v=0x4181 */ value |= 0x2000;
    value = flt_debounce( 0x4000);    
    /* v=0x4181 */ value |= 0x2000;
    value = flt_debounce( 0x4101);    
    /* v=0x4181 */ value |= 0x2000;
    value = flt_debounce( 0x4000);    
    /* v=0x4101 */ value |= 0x2000;
}

#endif

/******************************************************************************/
/* end of flt.c */
