/*
 ******************************************************************************
 *
 *  FILE:    gsebus_ser.c
 *
 *  AUTHOR:  Chris Gibson
 *
 *  DATE:    16/10/08
 *
 *  DESCRIPTION: Serial comms setup for "gsebus" based comms.
 *              For this level of embedded system it makes sence to implement
 *              the gsebus protocol at this interrupt level of the serial
 *              interface.  This allows short cuts with packet framing detection
 *              and the like.
 *
 *  COPYRIGHT: ® Ceramic Fuel Cells Limited 2008
 *
 *  ORIGINS:    Heavily modified version of ser.c (Stephen Fischer)
 *
 ******************************************************************************
 */
#include <stddef.h>
#include <io430.h>
#include <in430.h>
#include "gsebus_ser.h"
#include "stdint.h"

#include "cfcl.h"
#include "pio.h"
#include "rtc_api.h"
#include "ser_api.h"
#include "crc_api.h"
#include "utility.h"
#include "globals.h"

/******************************************************************************/
#define BAUD (57600)

#define  RS485TXDIR() (P3OUT  &=~P3O3_N_TX_ENABLE)
#define  RS485RXDIR() (P3OUT |= P3O3_N_TX_ENABLE)

#define  COMMS_STAT_LED_ON()  (P3OUT &= ~P3O6_N_LED_COMMS_STATUS)
#define  COMMS_STAT_LED_OFF() (P3OUT |= P3O6_N_LED_COMMS_STATUS)

#define SER_TXPKT_GUARDTIME rtc_8ms    /* Rx -> Tx delay.                  */
//#define SER_TXPKT_GUARDTIME (rtc_nodelay) /* Rx -> Tx delay.                  */
#define SER_RX_CH_TIMEOUT   rtc_8ms     /* Timout between Rx chars for      */
                                        /* incomplete packet test.          */
//#define SER_TX_HOLDTIME     rtc_8ms     
//#define SER_TX_HOLDTIME     (0)         /* Small delay.                   */
#define SER_TX_HOLDTIME     (rtc_nodelay) /* No delay.                      */

#define SER_TX_IND_LIGHT_TIME     rtc_90ms
#define SER_CCP_MSG_TIMEOUT       rtc_30s


/*-- Here be the dreaded globals. */
static uint8_t rxtx_buf[256];  /* Combined Receive and Transmit buffer */
static volatile uint8_t rx_index;
static volatile uint8_t tx_index;
static volatile uint8_t tx_size;

/*   tx_index and tx_size also contain the state of the transmitter */
/*   and reciever.                                                  */

/*   If tx_size == 0 then tx'er is idle.  (Rx mode.)                */
/*   If tx_size > 0  && tx_index == 0 then                          */
/*   If tx_size > 0  && tx_index < tx_size then tx is in progress.  */
/*   If tx_size > 0  && tx_index >= tx_size then in post tx delay.  */

void ser_init( void) 
{
    __disable_interrupt();
    
    /* reset SWRST bit to re enable USART and other settings */
    UCTL0 &= ~SWRST;

    /* reset, idle line mult.proc.protocol, UART func, no loopback,
       8 bit, one stop bit, odd parity, parity disable */
    UCTL0  = CHAR;

#if BAUD == 57600
    /* UBR10+UBR00 = 0x008b from 8M/57600=138.88 139=0x8b */
    UBR00  = 0x8b;
    UBR10  = 0x00;    
#elif BAUD = 115200
    /* UBR10+UBR00 = 0x0045 from 8M/115200=69.44 69=0x45 */
    UBR00  = 0x45;
    UBR10  = 0x00;    
#else
#error Unhandled baud rate.
#endif
    /* frc=SUM(00001111)/8=4/8=0.5 from example, 
       Baud Rate=BRCLK/(UBR+frc)=115K1Hz ( CRO measurement = ~116KHz)*/
    UMCTL0 = 0x0F;    

    /* UCLKI=UCLK, BRCLK=SMCLK=8M, 
       no URXS signal,mul.proc.com.feature,set when transmit empty */
    UTCTL0 = SSEL1;    

    /* (0-0-0-0-1-0-0-0) 
       each char. received sets URXIFG, err.char received alter URXIFG */
    URCTL0 = URXEIE;    

    /* P3.4,5 = USART0 RXD, TXD */
    P3SEL |= 0x30;

    /* P3.4 output direction USART0 TXEnable, TXD*/
    RS485RXDIR();
    P3DIR |= 0x18;
   
    /* USART0 Tx and Rx  module enable  */
    ME1   |= (UTXIFG0 | URXIFG0);
    
    /* USART0 Rx Interrupt enable  */
    IE1   |= (URXIE0); 
    
    rx_index = 0;
    tx_index = 0;
    tx_size  = 0;
    __enable_interrupt();
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          ser_state_machine
 *  FUNCTIONAL DESCRIPTION: Should be periodically called for serial comms
 *                          maintenance.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
static void ser_state_machine(void)
{
    uint8_t tmp_tx_sz, tmp_tx_idx;      /* Local copies of global volatiles */

    tmp_tx_sz = tx_size;
    
    if(tmp_tx_sz == 0){                         /* In Rx mode?              */
        if(rx_index != 0){                      /* Part of a packet seen?   */
            if(rtc_expired(rtc_ser_timeout)){   /* Gap between chars to big?*/
                rx_index = 0;                   /* Time out on packet rx.   */
            }
        }
    } else {                                    /* Some form of Tx mode.    */
        tmp_tx_idx = tx_index;                  /* Copy the volatile.   */
        if(tmp_tx_idx == 0){                    /* Pre Tx delay?            */
            if(rtc_expired(rtc_ser_timeout)){   /* Delay time up?           */
                RS485TXDIR();                   /* RS485 buffer outward.    */
                burnCPUCycles(1);               /* Settle delay.            */
                IE1 |= UTXIE0;                  /* Start txing.             */
                /* (The interrupt should kick system out of this state)     */
            }
        } else if(tmp_tx_idx >= tmp_tx_sz){     /* In post Tx delay?        */
            if(rtc_expired(rtc_ser_timeout)){   /* Delay time up?           */
                if(UTCTL0 & TXEPT){             /* Final byte gone?         */
                   RS485RXDIR();                /* Turn RS485 back inbound. */
                   tx_size=0;                   /* Back into Rx state.      */
                   tx_index=0;
                   rx_index=0;
                   RXBUF0;                      /* Discard any junk RX char.*/
                   IE1 |= URXIE0;                /* Enable Rx'er interrupt.  */
                }
            }
        }
    }
    if(rtc_expired(rtc_ser_led)){
        COMMS_STAT_LED_OFF();
    }
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_rx_pkt
 *  FUNCTIONAL DESCRIPTION: Investigate the Rx buffer for complete "gsebus"
 *                          packet.  If found return pointer to packet data.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           NULL if nothing new to interpret.
 *                          pointer to packet if found packet (with our
 *                          desination address).
 *  SIDE EFFECTS:           Background checking of timeouts for Rx Buffer.
 ******************************************************************************
 */
/*--- Offsets of items within Rx buffer. */
#define GSEBUS_IDX_STX      (0)     /* Start of packet char.    */
#define GSEBUS_IDX_SADR     (1)     /* Source address.          */
#define GSEBUS_IDX_TADR     (2)     /* Target Address.          */
#define GSEBUS_IDX_CMD      (3)     /* "Command" byte.          */
#define GSEBUS_IDX_LEN      (4)     /* Packet length.           */
#define GSEBUS_IDX_PAYLOAD  (5)     /* Payload data starts here.*/

gsebus_header_t * gsebus_rx_pkt(void)
{
    /*--- First some house keeping. */
    ser_state_machine();

    /*--- Then look at Rx buffer content. */
    if(rx_index > GSEBUS_IDX_LEN){           /* Packet length valid?     */
        uint8_t len = rxtx_buf[GSEBUS_IDX_LEN];
        if(rx_index > len + 1) {            /* Complete packet? */
            if(rxtx_buf[GSEBUS_IDX_TADR] != GSEBUS_ADDR_ID_WTS){
                rx_index = 0;   /* Packet not for us, or framing bad. */
                return NULL;
            }
            if(rxtx_buf[len + 1] != GSEBUS_ETX){
                rx_index = 0;   /* Packet not for us, or framing bad. */
                return NULL;
            }
            IE1 &= ~URXIE0;             /* Disable Rx interrupt.    */
            rx_index = 0;
            if(gsebus_crc_isInvalid(&rxtx_buf[1], len -2)){
                gsebus_formtx_nack();
                gsebus_formtx_finalise();
                rx_index = 0;
                wts_status.BadCrcCount++;   /* Maintain CRC error stats. */
                return NULL;
            }
            return (gsebus_header_t *)&rxtx_buf[1];/* Have valid packet, pass it back. */
        }
    }
    return NULL;
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_start
 *  FUNCTIONAL DESCRIPTION: Form response for transmission - Start of formation.
 *                          All responses are either an ACK or a NACK,
 *                          separate functions provided.
 *  FORMAL PARAMETERS:      cmd:    Command code.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           tx_size  set to 0 until whole packet formed.
 *                          tx_index reused as write pointer.
 *                          rxtx_buf filled in with response header
 ******************************************************************************
 */
static void gsebus_formtx_start(uint8_t cmd)
{
    tx_size = 0;        /* Leave size at 0 until whole packet formed. */

    gsebus_header_t *hdr = (gsebus_header_t *) &rxtx_buf[1];

    rxtx_buf[GSEBUS_IDX_STX] = GSEBUS_STX;  /* Start of transmission char.  */
    hdr->taddr = hdr->saddr;                /* Who we are responding to.    */
    hdr->saddr = GSEBUS_ADDR_ID_WTS;        /* We sent this.                */
    hdr->cmd   = cmd;
    tx_index = GSEBUS_IDX_PAYLOAD;
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_nack
 *  FUNCTIONAL DESCRIPTION: Use supplied rx packet data to form a nack
 *                          packet.  gsebus_formtx_end call needs to 
 *                          follow to send packet off.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           tx_index, tx_size, and rxtx_buf modified.
 *  Notes:                  A gsebus_formtx_ack call can follow a
 *                          gsebus_formtx_nack call if you need to change
 *                          your mind (to simplify the packet processing code)
 ******************************************************************************
 */
void gsebus_formtx_nack(void)
{
    gsebus_formtx_start(GSEBUS_NACK);
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_tx_nack
 *  FUNCTIONAL DESCRIPTION: As above but also finalises packet.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           tx_index, tx_size, and rxtx_buf modified.
 *  Notes:                  A gsebus_formtx_ack call can follow a
 *                          gsebus_formtx_nack call if you need to change
 *                          your mind (to simplify the packet processing code)
 ******************************************************************************
 */
void gsebus_tx_nack(void)
{
    gsebus_formtx_nack();
    gsebus_formtx_finalise();
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_start_ack
 *  FUNCTIONAL DESCRIPTION: Start forming an acknowledgement packet.
 *                          Separate functions provided for adding payload
 *                          content, and finalising packet for sending.
 *  FORMAL PARAMETERS:      cmd:    Command code.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           tx_size  set to 0 until whole packet formed.
 *                          tx_index reused as write pointer.
 *                          rxtx_buf   filled in with response header
 ******************************************************************************
 */
void gsebus_formtx_ack(void)
{
    gsebus_formtx_start(GSEBUS_ACK);
    COMMS_STAT_LED_ON();
    rtc_tickDelay(rtc_ser_led, SER_TX_IND_LIGHT_TIME);
    rtc_tickDelay(rtc_msg_timeout, SER_CCP_MSG_TIMEOUT);
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_add_uint8(uint8_t ch)
 *  FUNCTIONAL DESCRIPTION: Add a char into the tx buffer.
 *                          OK to pass signed chars, and unsigned to this same
 *                          function.
 *  FORMAL PARAMETERS:      ch : char to add to tx packet.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void gsebus_formtx_add_uint8(uint8_t ch)
{
    rxtx_buf[tx_index++] = ch;
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_add_uint16(uint16_t ch)
 *  FUNCTIONAL DESCRIPTION: Add a unsigned short into the tx packet.
 *                          OK to pass signed and unsigned values.
 *  FORMAL PARAMETERS:      wd : 16 bit word to add to tx buffer.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 *  Notes:                  Byte sent as little endian value.
 ******************************************************************************
 */
void gsebus_formtx_add_uint16(uint16_t wd)
{
    rxtx_buf[tx_index++] = wd;        /* Low byte.    */
    rxtx_buf[tx_index++] = wd >> 8;   /* High byte.   */
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_add_cst()
 *  FUNCTIONAL DESCRIPTION: Add a null terminated string (Inc termination).
 *                          OK to pass signed and unsigned values.
 *  FORMAL PARAMETERS:      s : String to copy to tx buffer.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void gsebus_formtx_add_cstr(const char *s)
{
    do{
        rxtx_buf[tx_index++] = *s;
    } while(*s++);
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_add_zfill()
 *  FUNCTIONAL DESCRIPTION: Zero fill a section of the Tx packet.
 *                          Mainly used for reserved areas of packets.
 *  FORMAL PARAMETERS:      sz : size - number of bytes to zero fill.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void gsebus_formtx_add_zero_fill(uint8_t sz)
{
    do{
        rxtx_buf[tx_index++] = 0;
    } while(--sz);
}


/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_add_mem()
 *  FUNCTIONAL DESCRIPTION: Copy memory area into comms packet.
 *  FORMAL PARAMETERS:      src: Data source to copy.
 *                          sz : Size of data to copy.
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void gsebus_formtx_add_mem(void *src, uint8_t sz)
{
    uint8_t *s = src;
    do{
        rxtx_buf[tx_index++] = *s++;
    } while(--sz);
}

/*
 ******************************************************************************
 *  FUNCTION NAME:          gsebus_formtx_end(void)
 *  FUNCTIONAL DESCRIPTION: Finish framing of packet and prepare for tx.
 *                          Adds CRC and End of frame char and queue for
 *                          sending.  Prime the timer for Rx->Tx delay.
 *  FORMAL PARAMETERS:      None
 *  RETURN VALUE:           None
 *  SIDE EFFECTS:           None 
 ******************************************************************************
 */
void gsebus_formtx_finalise(void)
{
    rxtx_buf[GSEBUS_IDX_LEN] = tx_index + 1;/* Put length into packet header. */
    gsebus_crc_generate(&rxtx_buf[1], tx_index -1);/* Add CRC to end of packet*/
    tx_index += 2;                          /* Update Idx to include CRC.     */
    rxtx_buf[tx_index++] = GSEBUS_ETX;      /* Append frame end char.         */
    tx_size = tx_index;                     /* Switch to pre tx delay.        */
    tx_index = 0;
    rtc_tickDelay(rtc_ser_timeout, SER_TXPKT_GUARDTIME);
    /*-- This will go off as soon as the Rx -> Rx delay is satisfied.    */
}

/*******************************************************************************
NOTE :- Language extensions enabled for Interrupt Service Routines.
        Only ISR functions should appear after this comment block.
        The compiler works out how many registers are saved for the 
        interrupt context switch.
     :- Do not use multiplication!
*******************************************************************************/
#pragma language=extended

/******************************************************************************/
#if __VER__ < 200
interrupt [UART0RX_VECTOR] void ser_rxIsr(void) 
#else
__interrupt void ser_rxIsr(void);   /* Prototype */
#pragma vector=UART0RX_VECTOR
__interrupt void ser_rxIsr(void)
#endif
{
    uint8_t rxChar = RXBUF0;
    uint8_t index = rx_index;

    if(index==0) {                              /* Awaiting start char?     */
        if(rxChar != GSEBUS_STX) return;        /* None found yet.          */
    }
    rtc_tickDelay(rtc_ser_timeout, SER_RX_CH_TIMEOUT);
    rxtx_buf[index++] = rxChar;                 /* Record char.             */
    rx_index = index;                           /* Update buf write index   */
}

/******************************************************************************/
#if __VER__ < 200
interrupt [UART0TX_VECTOR] void ser_txIsr( void) 
#else
__interrupt void ser_txIsr(void);   /* Prototype */
#pragma vector=UART0TX_VECTOR
__interrupt void ser_txIsr(void)
#endif
{
    uint8_t tmp;
    tmp = tx_index++;
    TXBUF0 = rxtx_buf[tmp++];

    if(tmp >= tx_size){             /* Whole message sent?      */
        rtc_tickDelay(rtc_ser_timeout, SER_TX_HOLDTIME);
        IE1 &= ~(UTXIE0);           /* Disable this interrupt.  */
    }
}

/******************************************************************************/
/* end of ser.c */
