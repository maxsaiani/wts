/*
 ******************************************************************************
 *
 *  FILE:    wts.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    17/10/08
 *
 *  DESCRIPTION: Defines and  globals for the water treatment system.
 *  
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */

#ifndef WTS_H
#define WTS_H

#define STRINGALISE_(x) #x
#define STRINGALISE(x) STRINGALISE_(x)

/* Hardware V4, FW Revision 1.08  */
#define cfclSoftwareVersion 0x4109

#define WTS_FIRMWARE_VERSION \
    "CFCL WTS V"STRINGALISE(cfclSoftwareVersion)" " __DATE__
#define WTS_COMPILER_VERSION            STRINGALISE(__VER__)
#define WTS_TARGET_DEVICE_DESCRIPTION   "MSP430F149IPM"

extern unsigned char flash_err;

#endif /* #ifndef WTS_H */
