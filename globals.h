/*
 ******************************************************************************
 *
 *  FILE:    globals.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    07/11/2008
 *
 *  DESCRIPTION: Variables used to communicate system state and control
 *              requirements 
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */


#ifndef GLOBALS_H
#define GLOBALS_H
#include "wts_comms.h"

//extern struct  comms_wts_ctrl wts_ctrl;
extern struct  comms_wts_status wts_status;
extern uint8_t comms_ok;

#endif
