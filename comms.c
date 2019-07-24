/*
 ******************************************************************************
 *
 *  FILE:    comms.c
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
#include <stddef.h>
#include <io430.h>
#include <in430.h>
#include <string.h>
//#include <stdio.h>

#include "clk_api.h"
#include "pio.h"
#include "gsebus_ser.h"
#include "gsebus.h"
#include "wts.h"
#include "globals.h"
#include "steam_flow.h"
#include "cooling_air_valve.h"
#include "solenoids.h"
#include "anin.h"
#include "wts_comms.h"
#include "fls_api.h"
#include "reflash.h"
#include "comms.h"
#include "timers.h"

#include "rtc.h"
/*
 ******************************************************************************
 *  FUNCTION NAME:          ctrl_status_wr
 *  FUNCTIONAL DESCRIPTION: Take supplied packet for required outputs, and
 *                          set up system variables to achieve those outputs.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
static void ctrl_status_wr(const uint8_t *payload)
{
    struct comms_wts_ctrl *ctrl;

    ctrl = (struct comms_wts_ctrl*) &payload[1];

    /*--- Adjust outputs according to control structure received from CCP */
    steam_flowrate_set(ctrl->flg.SteamEnable? ctrl->steam_flow: 0);
    cooling_air_valve_set(
            ctrl->flg.CoolAirEnable, ctrl->cool_air_valve_pos);
    fill_solenoid_set(ctrl->flg.DeminFillValve);
    purge_solenoid_set(ctrl->flg.DeminPurgeValve);

    /*--- Set digital outputs as required. */
    if(ctrl->flg.DeminFillPump){
        P4OUT |= P4O2_DOUT3_PUMP1_FILL;
    } else {
        P4OUT &= ~P4O2_DOUT3_PUMP1_FILL;
    }
    if(ctrl->flg.DeminPolishPump){
        P4OUT |= P4O3_DOUT4_PUMP2_POLISH;
    } else {
        P4OUT &= ~P4O3_DOUT4_PUMP2_POLISH;
    }
    if(ctrl->flg.CondensatePump){
        P4OUT |= P4O4_DOUT5_PUMP3_CONDENSATE;
    } else {
        P4OUT &= ~P4O4_DOUT5_PUMP3_CONDENSATE;
    }

    /*--- PWM outputs. */
    pwm1_set(ctrl->pwm1);
    pwm2_set(ctrl->pwm2);
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          form_status_response
 *  FUNCTIONAL DESCRIPTION: Form a standard status response packet.
 *                          This response is given for both 
 *                          GSEBUS_CMD_RD_DATA and GSEBUS_CMD_RW_DATA 
 *                          commands to location WTS_DADR_CTRL_STATS
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
static void ctrl_status_rd(void)
{
    if(flash_err){
        /*-- Trash normal response. */
        gsebus_formtx_nack();
        gsebus_formtx_add_cstr("Bad CRC");
        return;
    }
    gsebus_formtx_ack();
    gsebus_formtx_add_uint8(WTS_DADR_CTRL_STATUS);

    /*---- Update some of the stuff before sending. */
    wts_status.sw_version = cfclSoftwareVersion;

    rtc_getUpTime(&wts_status.UpTimeTick);
    /*wts_status.BadCrcCount; Maintained in gsebus_ser.c */
    wts_status.bits = pio_din_get();

    wts_status.port_raw_inputs[0] = P1IN;
    wts_status.port_raw_inputs[1] = P2IN;
    wts_status.port_raw_inputs[2] = P3IN;
    wts_status.port_raw_inputs[3] = P4IN;
    wts_status.port_raw_inputs[4] = P5IN;
    
    wts_status.cool_air_pos_estimate = cooling_air_get_pos();

    /*--- Copy in analogue readings. */
    anin_rd_to_comms(&wts_status);

    __disable_interrupt();
    gsebus_formtx_add_mem(&wts_status, sizeof(wts_status));
    __enable_interrupt();
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          cmd_wr_data
 *  FUNCTIONAL DESCRIPTION: "Write data" as specified for a given location.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
static uint8_t cmd_rd_data(uint8_t *payload)
{
    /*-- Rd data command starts with an "address" */
    switch(*payload){
        case WTS_DADR_FW_ID:
            gsebus_formtx_ack();        /* Something we can deal with.  */
            gsebus_formtx_add_uint8(WTS_DADR_FW_ID);
            gsebus_formtx_add_cstr(WTS_FIRMWARE_VERSION);
            return 0;   /* No error. */
        case WTS_DADR_CMPLR_VER:
            gsebus_formtx_ack();        /* Something we can deal with.  */
            gsebus_formtx_add_uint8(WTS_DADR_CMPLR_VER);
            gsebus_formtx_add_cstr(WTS_COMPILER_VERSION);
            return 0;   /* No error. */
        case WTS_DADR_TGT_DEV:
            gsebus_formtx_ack();        /* Something we can deal with.  */
            gsebus_formtx_add_uint8(WTS_DADR_TGT_DEV);
            gsebus_formtx_add_cstr(WTS_TARGET_DEVICE_DESCRIPTION);
            return 0;   /* No error. */
        case WTS_DADR_CTRL_STATUS:
            ctrl_status_rd();           /* Give system status.          */
            return 0;
        default:
            break;
    }
    return 1;                           /* Bad command request.         */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          cmd_wr_data
 *  FUNCTIONAL DESCRIPTION: Process supplied data for a given location_id
 *                          If "writing" is supported for a given location ID
 *                          the data is written to the specified "location".
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Note:                   The idea of a location is a concept only, we just
 *                          treat different locations as different commands.
 ******************************************************************************
 */
static uint8_t cmd_wr_data(uint8_t *payload)
{
    uint8_t status;
    switch(payload[0]){             /* Payload starts with the location ID. */
        case WTS_DADR_CTRL_STATUS:  /* Control / status packet.             */
            ctrl_status_wr(payload);/* Do something with payload data.      */
            gsebus_formtx_ack();    /* Acknolwage that we've accepted the data*/
            gsebus_formtx_add_uint8(WTS_DADR_TGT_DEV); /* What was accepted */
            return 0;   /* No error. */
        case WTS_DADR_FW_BLOCK:
            status = fls_fwug_cmd((struct comms_fw_upgrade *)(&payload[1]));
            gsebus_formtx_ack();
            gsebus_formtx_add_uint8(WTS_DADR_FW_BLOCK); /* What was accepted */
            gsebus_formtx_add_uint8(status);            /* How we went. */
            return 0;
        case WTS_DADR_REFLASH:
            status = do_reflash();  /* Will hopefully upgrade the firmware */
            gsebus_formtx_ack();
            gsebus_formtx_add_uint8(WTS_DADR_REFLASH); /* What was accepted */
            gsebus_formtx_add_uint8(status);
            return 0;
    }
    return 1;           /* Is error unsupported address. */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          cmd_rw_data
 *  FUNCTIONAL DESCRIPTION: Process supplied data for a given location_id
 *                          as per a cmd_wr_data command, and respond with 
 *                          data for the same location_id, as per a cmd_rd_data.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Note:                   There need be no correlation between the data
 *                          written and the data read.  For example the command
 *                          can give the required system output states, and
 *                          the returned data can be the current status of the
 *                          inputs.
 ******************************************************************************
 */
static uint8_t cmd_rw_data(uint8_t *payload)
{
    switch(payload[0]){             /* Payload starts with the location ID. */
        case WTS_DADR_CTRL_STATUS: /* Control / status packet.             */
            ctrl_status_wr(payload);/* Same as for cmd_wr_data command      */
            ctrl_status_rd();       /* Respond as per cmd_rd_data command   */
            return 0;
    }
    return 1;   /* Unsupported address. */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          cmd_poll
 *  FUNCTIONAL DESCRIPTION: Look for new Rx packets, and if they have arrived
 *                          process them.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void comms_poll(void)
{
    gsebus_header_t *hdr;
    uint8_t *payload;
    uint8_t is_error = 0;
    hdr = gsebus_rx_pkt();      /* Poll for new message on serial interface */
    if(hdr == NULL) return;                     /* No new message.          */
    payload = ((uint8_t *)hdr) + sizeof(*hdr);  /* Payload follows header.  */
    switch(hdr->cmd){                           /* Message to process.      */
        case WTS_CMD_RD_DATA:
            is_error = cmd_rd_data(payload);
            break;
        case WTS_CMD_WR_DATA:
            is_error = cmd_wr_data(payload);
            break;
        case WTS_CMD_WR_RD_DATA:
            is_error = cmd_rw_data(payload);
            break;
        default:
            is_error = 1;
            break;
    }
    if(is_error){
        gsebus_formtx_nack();                   /* Report for failure.     */
    }
    gsebus_formtx_finalise();
}
