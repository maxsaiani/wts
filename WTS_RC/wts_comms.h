/*
 ******************************************************************************
 *
 *  FILE:    wts_comms.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    28/10/08
 *
 *  DESCRIPTION: Structures used to communicate between CCP and WTS
 *      CCP: Critical Control process.
 *      WTS: Water Treatment system.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef WTS_COMMS_H
#define WTS_COMMS_H

#include "gsebus.h"
#include "stdint.h"

/*--- GSEBUS "commands" that the WTS knows. */
#define WTS_CMD_WR_DATA      (0x08)  /* Write data to location.      */
#define WTS_CMD_RD_DATA      (0x09)  /* Read data from location.     */
#define WTS_CMD_WR_RD_DATA   (0x0A)  /* Write then read location     */

/*--  These "location ID's" are used with the read, write and 
 *     read write commands                                      */
/*--- Location identifiers stolen from the GSE  specification.  */
#define WTS_DADR_FW_ID          (0x01)
#define WTS_DADR_CMPLR_VER      (0x02)
#define WTS_DADR_TGT_DEV        (0x03)

/*--- Location identifiers made up specifically for WTS comms.      */
#define WTS_DADR_CTRL_STATUS    (0x10)  /* Control / status "location"  */
#define WTS_DADR_FW_BLOCK       (0x11)  /* Write one "block" of firmware*/
#define WTS_DADR_REFLASH        (0x12)  /* Re-write code flash.     */

struct comms_wts_ctrl_bits {
    uint16_t DeminFillValve:1;
    uint16_t DeminPurgeValve:1;
    uint16_t DeminFillPump:1;
    uint16_t DeminPolishPump:1;
    uint16_t CondensatePump:1;
    uint16_t SteamEnable:1;
    uint16_t CoolAirEnable:1;
    uint16_t spares:9;
};

struct comms_wts_ctrl{
    union {
        struct comms_wts_ctrl_bits flg;
        uint16_t flgs;
    };
    uint16_t steam_flow;        /* Steam flow ul/min                        */
    int16_t  cool_air_valve_pos;/* Requested cooling air valve position.    */
    uint16_t pwm1;              /* PWM output 1 (Cooling air?).             */
    uint16_t pwm2;              /* PWM output 2 (Steam flow?).              */
};

struct comms_fw_upgrade{
    uint16_t addr;              /* Data destination.        */
    uint8_t  len;               /* Length in bytes.         */
    uint8_t  data[256-4-3];     /* 128 bytes of firmware.   */
};

#define WTS_ERR_BASE            (20)

#define WTS_ERR_FWUG_BADADDR (1 + WTS_ERR_BASE) /* Unacceptable address  */
#define WTS_ERR_FWUG_WRFAIL  (2 + WTS_ERR_BASE) /* Write verification fail.*/
#define WTS_ERR_FWUG_CRC     (3 + WTS_ERR_BASE) /* Bad CRC on reflash command*/

struct comms_wts_status_bits {
    uint16_t TankHigh:1;
    uint16_t TankLow:1;
    uint16_t CondensateHigh:1;
    uint16_t CondensateLow:1;
    uint16_t Spare1:1;
    uint16_t Leak1:1;
    uint16_t Leak2:1;
    uint16_t Leak3:1;
    uint16_t Leak4:1;
    uint16_t LeakDetectPresent:1;
    uint16_t spares:6;
};

struct comms_wts_status{
    uint16_t sw_version;
    uint16_t UpTimeTick;
    uint32_t UpTimeMinutes;
    uint16_t BadCrcCount;
    uint16_t cpu_temperature;
    union {
        struct comms_wts_status_bits bit;
        uint16_t bits;
    };

    uint8_t  port_raw_inputs[5];
    uint8_t  anin_filt_overflows;       /* Inc if insufficient CPU time for  */
                                        /* analogue input filtering.        */
    uint16_t cool_air_pos_estimate;

    uint16_t anin_probe1_conductivity;
    uint16_t anin_probe1_temperature;
    uint16_t anin_probe2_conductivity;
    uint16_t anin_probe2_temperature;
    uint16_t anin_spare1;               /* (Spare input)    */
    uint16_t anin_water_meter;          /* (Spare input)    */
    uint16_t anin_3V6;					/* Supply voltage check. was anin_temp*/
    uint16_t resurved;                  /* Was anin_pvc2;   */
    
    uint16_t anin_24V;                  /* 24V supply voltage check */
    uint16_t anin_polish_current;       /* Pump                     */  
    uint16_t anin_condensate_current;   /* Pump                     */
    uint16_t anin_5V;                   /* Supply voltage check.    */
    uint16_t anin_1V2;                  /* Supply voltage check.    */
    uint16_t anin_fill_current;         /* Solenoid                 */
    uint16_t anin_purge_current;        /* Solenoid                 */
    uint16_t anin_boost_current;        /* Pump                     */
};

#endif  /* #ifndef WTS_COMMS_H */
