/*
 ******************************************************************************
 *
 *  FILE:    reflash.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    27/01/2009
 *
 *  DESCRIPTION:
 *      Take data from 0xC000->0xDFFF and flash this to 0xE000->0xFFFF
 *      The code is currently less than 8K.  This means that we should be
 *      able to fit it into a MSP430F135 (twice).  I.E. we can download new
 *      firmware version into 0xC000, then trigger this code to overwrite
 *      to the upper 4K (interrupt vectors and all).
 *
 *  NOTES:
 *      Some tricks need to be used in the linker file so that the reflash
 *      code that runns in the lower image does not overlap with the main code 
 *      in the uppder image when the two areas are merged together.
 *
 *      An in house application "iar2wts_bin" is used to merge the two images
 *      together to provide a binary image "WaterTreatmentSystem.fwi" with
 *      CRC inserted.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2009
 *
 ******************************************************************************
 */
#include <io430.h>
#include <in430.h>

#include "crc_api.h"
#include "wdg.h"
#include "reflash.h"
#include "wts_comms.h"
#include "pio.h"

#define ORIGIN          (0xC000)
#define DESTINATION     (0xE000)
#define LENGTH          (0x2000)

#define FLASH_SEG_SZ    (512)

void do_save_flash(void);

/*
 ******************************************************************************
 *  FUNCTION NAME:          reflash_check
 *  FUNCTIONAL DESCRIPTION: Check the flash content on system startup.
 *                          Performs best attempt at fixing flash content if
 *                          things went bad part way through a re-flash.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
#pragma location="this_code_first"    /* Place near start of flash. */
void reflash_startup_check(void)
{
    if(gsebus_crc_isInvalid((void*)DESTINATION, LENGTH - 2)){
        /*-- There is a CRC error in the main flash, try the copy.  */
        do_reflash();
        /*-- We come here if there is a CRC error in the copy too.
        *   best option is just to carry on and hope it works.
        */
        flash_err = 1;
    } else if(gsebus_crc_isInvalid((void*)ORIGIN, LENGTH - 2)){
        /*-- Second copy has error.  Overwrite this with the main copy. */
        do_save_flash();
    }
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          do_save_flash();
 *  FUNCTIONAL DESCRIPTION: Make a complete copy of the primary flash over
 *                          the secondary flash.
 *                          Performed if there is an error detected in the
 *                          backup image.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Notes:                  Run this with interrupts turned off,
 *                          and watchdog disabled.
 ******************************************************************************
 */
void do_save_flash(void)
{
    uint16_t *source_addr = (uint16_t *)DESTINATION;
    uint16_t *dest_addr   = (uint16_t *)ORIGIN;
    
    int16_t seg_count    = LENGTH / FLASH_SEG_SZ;
     
    /* Unlock flash writes. */
    FCTL3 = FWKEY;  /* Unlock */
    
    do {
        int16_t word_count;
        /* Flash Error LED fast during this process. */
        P3DIR ^= P3O7_N_LED_ALARM;
        
        /* Clear the segment.   */
        FCTL1 = FWKEY | ERASE;
        *dest_addr = 0;             /* Trigger the erase.   */
        while (FCTL3 & BUSY);       /* Wait for completion. */
        
        word_count = FLASH_SEG_SZ / sizeof(uint16_t);
        FCTL1 = FWKEY | WRT;                   
        do {
            *dest_addr++ = *source_addr++;
            while (FCTL3 & BUSY);       /* Wait for completion. */
        }while(--word_count);
    }while (--seg_count);
    FCTL3 = FWKEY | LOCK;
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          reflash_flash
 *  FUNCTIONAL DESCRIPTION: re-flash the flash.  Copy the flash content from
 *                          the download location to the execution location.
 *                          This is the scary bit in the whole process.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Note:                   This codes run from the download location.
 *                          Don't use any function calls.
 *                          Check the assembly listing produced to make sure
 *                          there are no glue function calls added.
 ******************************************************************************
 */
#pragma location="start_of_origin"
static void do_reflash_executed_from_within_copy(void)
{
    uint16_t *source_addr = (uint16_t *)ORIGIN;
    uint16_t *dest_addr   = (uint16_t *)DESTINATION;
    
    __disable_interrupt();
    
    WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog timer to prevent time out
    // reset
    /* Unlock flash writes. */
    FCTL3 = FWKEY;  /* Unlock */
    
    while(dest_addr != NULL){   /* 0 is just after the last vector. */
        uint16_t wr_counter;
        
        /* Clear the segment.   */      
        while (FCTL3 & BUSY);
        FCTL1 = FWKEY | ERASE;
        *dest_addr = 0;         /* Trigger the erase.   */
        
        /* Write the segment.   */
        FCTL1 = FWKEY | WRT;                   
        wr_counter = FLASH_SEG_SZ/sizeof(uint16_t);
        do{
            if( *source_addr != 0xFFFF)
            {
                /* Write one flash word. */
                while (FCTL3 & BUSY);
                *dest_addr = *source_addr;
            }
            dest_addr ++;
            source_addr ++;
        }while(--wr_counter);
    }
    FCTL3 = FWKEY | LOCK;
    /* Pheuph!  Have re-flashed.  Lets just hope it runs now. */
    WDTCTL = WDT_MRST_0_064;
    while(1);   /* Only 64mS to wait for the Grim Reaper. */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          do_reflash
 *  FUNCTIONAL DESCRIPTION: Check the CRC of the new copy of the code, if OK
 *                          call the re-flash routine at the start of that
 *                          new code.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Notes:                  The CRC must be the reset vector.  The actual
 *                          CRC must be placed before the vector table such
 *                          that the final calculated CRC including the table
 *                          up to the final vector (reset) matches the final
 *                          vector.
 *                          Hint: CRC calculations can be made to work in 
 *                          reverse.
 ******************************************************************************
 */
#pragma location="this_code_first"    /* Early execution code. */
uint8_t do_reflash(void)
{
    if(gsebus_crc_isInvalid((void*)ORIGIN, LENGTH - 2)){
        /* Refuse to re-flash if copy not valid.    */
        return WTS_ERR_FWUG_CRC;
    }
    
    /*-- Call the function running WITHIN the memory space of the new flash
    *   copy. */
    do_reflash_executed_from_within_copy();     /* Never returns.           */
    return 0;   /* Never realy returns. */
}
