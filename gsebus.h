/*
 ******************************************************************************
 *
 *  FILE:    gsebus.h
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    10/07/08
 *
 *  DESCRIPTION: Interface description for the sending and retrieving packets
 *              on the gsebus.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 ******************************************************************************
 */
#ifndef GSEBUS_H
#define GSEBUS_H

//#include <stdint.h>
#include "stdint.h"

/*--- Header structure for gsebus packets. (Excluding the STX char). */
typedef struct{
    uint8_t saddr;      /* Source address.  */
    uint8_t taddr;      /* Target address.  */
    uint8_t cmd;        /* Command byte.    */
    uint8_t len;        /* Packet length, including trailing CRC */
}gsebus_header_t;

#define GSEBUS_ADDR_ID_CCP      (0x01)  /* Seems to be hard coded to one*/
#define GSEBUS_ADDR_ID_GSE      (0x98)  /* Address of gse on gse bus.   */
#define GSEBUS_ADDR_ID_WTS      (0x20)  /* Address of wts on gse bus.   */

#define GSEBUS_STX              (0x02)
#define GSEBUS_ETX              (0x03)

#define GSEBUS_ACK              (0x06)
#define GSEBUS_NACK             (0x15)


#endif  /* ifndef GSE_H */
