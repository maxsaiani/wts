/*
********************************************************************************
*
*  FILE:    $(Dios)/rtc.h
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

#ifndef __RTC_H__
#define __RTC_H__
#include "stdint.h"

/******************************************************************************/
/* Note all delays are rounded until the next systick. */
#define rtc_nodelay (-1)
#define rtc_0ms     0       /* Delay until next systick.    */
#define rtc_8ms     1
#define rtc_16ms    2
#define rtc_24ms    3
#define rtc_32ms    4
#define rtc_40ms    5
#define rtc_48ms    6
#define rtc_65ms    8
#define rtc_81ms    10
#define rtc_90ms    11
#define rtc_98ms    12
#define rtc_122ms   15
#define rtc_180ms   22
#define rtc_205ms   25
#define rtc_250ms   30
#define rtc_500ms   61
#define rtc_1s      122
#define rtc_2s      244
#define rtc_5s      610
#define rtc_10s     1220
#define rtc_30s     3662
#define rtc_50s     6103
#define rtc_60s     7324
#define rtc_120s    14648
#define rtc_268s    32714 /* max delay */



/*******************************************************************************/

#define rtc_maxTimers 16

extern uint16_t rtc_timer[rtc_maxTimers];

/*--- The + 1 in the rtc_tickDelay means that the delay will be at _least_  */
/*    the value specified.  May though be a whole systick longer.           */
#define rtc_tickDelay(timer, delay) do{rtc_timer[timer] = delay + 1;}while(0)
#define rtc_expired(timer)          (rtc_timer[timer] == 0)
#define rtc_notExpired(timer)       (rtc_timer[timer] != 0)

/******************************************************************************/

typedef struct {
    unsigned short start;
    unsigned short delta;
    } rtc_watches;

/* 125ns counts */
#define rtc_watchStartTock(value) value.start = rtc_watchTockStart()
#define rtc_watchStopTock( value) value.delta = rtc_watchTockStop(value.start)

/* 8.192ms counts */
#define rtc_watchStart(value)   value.start = rtc_watchTickStart()
#define rtc_watchStop( value)   value.delta = rtc_watchTickStop(value.start)

/* 60s counts */
#define rtc_watchStartMinute(value) value.start = rtc_watchMinuteStart()
#define rtc_watchStopMinute(value)  value.delta=rtc_watchMinuteStop(value.start)

void rtc_init(void);
void rtc_state_machine(void);
void rtc_getUpTime(unsigned short *ptr);
void rtc_timeStamp(char *str);

short rtc_watchTockStart(void);
short rtc_watchTockStop(short start);

short rtc_watchTickStart(void);
short rtc_watchTickStop(unsigned short start);

short rtc_watchMinuteStart(void);
short rtc_watchMinuteStop(unsigned short start);

#endif /* __RTC_H__ */
