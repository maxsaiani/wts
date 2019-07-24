/*
 ******************************************************************************
 *
 *  FILE:    reflash.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    27/01/2009
 *
 *  DESCRIPTION: Code to copy the content of downloaded flash image into
 *              main execution area.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2009
 *
 ******************************************************************************
 */
#ifndef REFLASH_H
#define REFLASH_H
#include "cfcl.h"

void reflash_startup_check(void);
uint8_t do_reflash(void);

#endif /* #ifndef REFLASH_H */
