/*
********************************************************************************
*
*  FILE:    $(Dios)/rtc_api.h
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the real time clock class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
********************************************************************************
*/

#ifndef __RTC_API_H__
#define __RTC_API_H__

#include "stdint.h"

#include "rtc.h"

/******************************************************************************/

/******************************************************************************/

typedef enum {
    rtc_msg_timeout, /* when no ccp comms for 5s, become unhappy            */
    //rtc_pid_check,   /* check the pid flash constants every minute          */
    //rtc_pid_pause,   /* recalc the PID algo every 98ms                      */
    //rtc_pwm_check,   /* check the pwm flash constants every minute          */
    rtc_ser_timeout, /* when no char received after 16ms reset the receiver */
    rtc_ser_led,     /* Status LED timer.                                   */
    //rtc_tcs_pause,   /* let analogue to stablise or wait for a sample       */
    //rtc_tcs_recal,   /* perform a self calibration every minute             */
    //rtc_pul_deb0,    /* Debounce for pulse counter                          */
    //rtc_pul_deb1,    /* Debounce for pulse counter                          */
    //rtc_pul_deb2,    /* Debounce for pulse counter                          */
    //rtc_pul_deb3,    /* Debounce for pulse counter                          */
    //rtc_pul_deb4,    /* Debounce for pulse counter                          */
    rtc_index
    } rtc_timerChannels;


extern volatile uint8_t systick; /* Incremented once every 8.192ms in TIMERA */
                                 /* Timer overflow. */
    
#if (rtc_index > rtc_maxTimers)
#error "Too many rtc_indexes specified, max is 16"
#endif

/******************************************************************************/

#define rtc_minute_offset       rtc_60s/4
#define rtc_tcs_recal_offset    0*rtc_minute_offset
#define rtc_pid_check_offset    1*rtc_minute_offset
#define rtc_pwm_check_offset    2*rtc_minute_offset

#endif /* __RTC_API_H__ */
