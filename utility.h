
/*
*********************************************************************************
*
*  FILE:    $(Dios)/utility.h
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

#ifndef __UTILITY_H__
#define __UTILITY_H__

#define wasteMicroSec(number)\
  do{volatile short burn; for (burn = 0; burn < number; ++burn);}while(0)

#define copyData( src, dst, index) do {*dst++ = *src++;} while (--index);

void burnCPUCycles( int ms);

#define putNull(  str)        *str   = '\0'
#define putChar(  str, value) *str++ = value

#define putByte(  str, value) str = putTheByte( str, value)
#define putShort( str, value) str = putTheShort( str, value)

char *putTheByte ( char *str, unsigned char  value);
char *putTheShort( char *str, unsigned short value);

#endif /* __UTILITY_H__ */
