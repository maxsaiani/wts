/*
 ******************************************************************************
 *
 *  FILE:    comms.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    17/10/08
 *
 *  DESCRIPTION: Higher level communications.
 *              Poll for new messages, interpret and form response when one 
 *              arrives.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef COMMS_H
#define COMMS_H

#include "gsebus_ser.h"
#include "gsebus.h"

void comms_poll(void);

#endif /* #ifndef COMMS_H */
