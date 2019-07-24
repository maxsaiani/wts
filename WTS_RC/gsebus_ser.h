/*
 ******************************************************************************
 *
 *  FILE:    gsebus_ser.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    17/10/2008
 *
 *  DESCRIPTION: Interface functions for gsebus based serial interface.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef GSEBUS_SER_H
#define GSEBUS_SER_H

//#include <stdint.h>
#include "stdint.h"
#include "gsebus.h"


void ser_init(void);

gsebus_header_t * gsebus_rx_pkt(void);

void gsebus_tx_nack(void);

/*-- Response construction functions.   */
void gsebus_formtx_nack(void);
void gsebus_formtx_ack (void);
void gsebus_formtx_add_uint8(uint8_t ch);
void gsebus_formtx_add_uint16(uint16_t wd);
void gsebus_formtx_add_cstr(const char *s);
void gsebus_formtx_add_zero_fill(uint8_t sz);
void gsebus_formtx_add_mem(void *src, uint8_t sz);
void gsebus_formtx_finalise(void);

#endif      /* #ifndef GSEBUS_SER_H */
