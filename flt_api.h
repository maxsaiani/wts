
/*
*********************************************************************************
*
*  FILE:    $(Dios)/flt_api.h
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the crc class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*********************************************************************************
*/

#ifndef __FLT_API_H__
#define __FLT_API_H__

void flt_init( void);

unsigned short flt_debounce( unsigned short bits);

#endif /* __FLT_API_H__ */
