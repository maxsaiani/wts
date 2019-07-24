/*
*********************************************************************************
*
*  FILE:    $(Dios)/cfcl.h
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file consists of include files
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*********************************************************************************
*/

#ifndef __CFCL_H__
#define __CFCL_H__

#include "wts.h"    /* For cfclSoftwareVersion. */


typedef enum {
    false = 0,
    true  = 1
    } cfcl_boolean;

typedef enum {
    cfcl_good = 0,
    cfcl_error
    } cfcl_results;

typedef enum {
    cfcl_done  = 0,
    cfcl_first = 0,
    cfcl_inProgress
    } cfcl_progress;

typedef enum {
    pid_MinOverMax = 0,
    pid_MaxOverMin
    } pid_precedences;

#endif /* __CFCL_H__ */
