
/*
*********************************************************************************
*
*  FILE:    $(Dios)/crc_api.h
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

#ifndef __CRC_API_H__
#define __CRC_API_H__
//#include <stdint.h>
#include <stddef.h>
#include "stdint.h"
#include "cfcl.h"   /* Result codes. */

#define crc_overhead 2

/*-- As above but with little endian CRC's  */
cfcl_results gsebus_crc_isInvalid(void *buf, uint16_t sz);
void gsebus_crc_generate(void *buf, uint16_t sz);

#endif /* __CRC_API_H__ */
