/*
*********************************************************************************
*
*  FILE:    $(Dios)/fls.c
*
*  AUTHOR:  Stephen Fischer
*
*  DATE:    01/11/05
*
*  DESCRIPTION: This file contains the Flash Access class
*               for the Distributed IO System for the CHP3/NetGen.
*
*  COPYRIGHT: ® Ceramic Fuel Cells Limited 2005
*
*
*********************************************************************************
*/
#include <io430.h>
#include <in430.h>
#include <string.h>
//#include "stdint.h"

//#include "wfm_comms.h"
#include "fls_api.h"

#define UNLOCK 0

/******************************************************************************/
/* Note: Assumed to be running from flash. */
void fls_init(void)
{
    istate_t ist = __get_interrupt_state(); 

    __disable_interrupt(); 
    FCTL2 = FWKEY | FSSEL_3 | 0x0011;
    __set_interrupt_state(ist); 
}

/******************************************************************************/
/* Note: Assumed to be running from flash. */
void fls_erase(const uint16_t *seg)
{
    istate_t ist = __get_interrupt_state(); 
 
    __disable_interrupt(); 
    FCTL3 = FWKEY | UNLOCK;
    FCTL1 = FWKEY | ERASE;
    *(uint16_t *)seg = 0xff;
    FCTL1 = FWKEY;
    FCTL3 = FWKEY | LOCK;
    __set_interrupt_state(ist); 
}

/******************************************************************************/
/* Note: Assumed to be running from flash. */
/*       Block write function is not available when running from flash. */
void fls_write(const uint16_t *dst, void *src, uint16_t nWords) {
    uint8_t *s = src;
    uint16_t val;
    
    istate_t ist = __get_interrupt_state(); 
    do {
        val = *s++;
        val |= *s++ << 8;
        __disable_interrupt(); 
        FCTL3 = FWKEY | UNLOCK;
        FCTL1 = FWKEY | WRT;
        *(uint16_t *)dst++ = val;
        FCTL1 = FWKEY;
        FCTL3 = FWKEY | LOCK;
        __set_interrupt_state(ist); 
    } while (--nWords);
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          fls_fwug_cmd
 *  FUNCTIONAL DESCRIPTION: Flash Firmware upgrade command.
 *  FORMAL PARAMETERS:      upgd    : Upgrade comms command.
 *  RETURN VALUE:           error code: Z if all OK, error code on problem.
 *  SIDE EFFECTS:           Will erase whole flash block if dest address is
 *                          on flash boundary.
 ******************************************************************************
 */
uint8_t fls_fwug_cmd(struct comms_fw_upgrade *cmd)
{
    extern void __program_start;
    const uint16_t prog_start = (uint16_t)&__program_start;
    /* Check parameters. */
    if( cmd->addr >= prog_start ||/* Could overwrite running code? */
        cmd->addr + cmd->len >= prog_start ||
        (cmd->addr & 1))
    {
        return WTS_ERR_FWUG_BADADDR;
    }

    /*-- Erase segment if required. */
    if(cmd->addr % fls_segementSize == 0){  /* Segment erase required?  */
        fls_erase((void*)cmd->addr);
    }

    /*-- Write flash block.     */
    fls_write((uint16_t *)(cmd->addr), cmd->data, cmd->len / 2);

    /*-- Verify flash block.    */
    if(memcmp((void*)(cmd->addr), cmd->data, cmd->len) != 0){
        return WTS_ERR_FWUG_WRFAIL;
    }

    return 0;
}

/******************************************************************************/
/* end of fls.c */
