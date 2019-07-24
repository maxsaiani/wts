/*
*********************************************************************************
*
*  FILE:    $(Dios)/fss_api.h
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the Flash Segement System class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*  NOTE: Changing this file means the software is no longer backwardly compatable
*
*********************************************************************************
*/

#ifndef __FLS_API_H__
#define __FLS_API_H__

#include "cfcl.h"
#include "stdint.h"
#include "wts_comms.h"

#define fls_segementSize        (512)
#define fls_segementSizeInWords (512/2)

void fls_init( void);
void fls_InterruptAccess( cfcl_boolean value);
void fls_erase(const uint16_t *seg);
void fls_write(const uint16_t *dst, void *src, uint16_t nWords);

uint8_t fls_fwug_cmd(struct comms_fw_upgrade *cmd);

#endif /* __FLS_API_H__ */
