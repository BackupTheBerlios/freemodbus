/*
 * FreeModbus Libary: STR71x Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: demo.c,v 1.1 2006/09/04 01:41:48 wolti Exp $
 */

/* ----------------------- lwIP includes ------------------------------------*/
#include "lwip/opt.h"
#include "lwip/sio.h"
#include "lwip/sys.h"
#include "ppp/ppp.h"
#include "arch/cc.h"

/* ----------------------- FreeRTOS includes --------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ----------------------- Platform includes --------------------------------*/
#include "eic.h"
#include "netif/serial.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"

/* ----------------------- Defines ------------------------------------------*/
#define mainMB_TASK_PRIORITY    ( tskIDLE_PRIORITY + 3 )
#define PROG                    "FreeModbus"
#define REG_INPUT_START         1000
#define REG_INPUT_NREGS         4

#define PPP_USER                "FreeModbus"
#define PPP_PASS                "insecure"

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    CONNECTING, CONNECTED, DISCONNECT
} ePPPThreadControl;

/* ----------------------- Static variables ---------------------------------*/
static unsigned short usRegInputStart = REG_INPUT_START;
static unsigned short usRegInputBuf[REG_INPUT_NREGS];
static ePPPThreadControl ePPPThrCtl;


/* ----------------------- Static functions ---------------------------------*/
static void     vlwIPInit( void );
static void     vMBServerTask( void *arg );
static void     vPPPStatusCB( void *ctx, int errCode, void *arg);

sio_fd_t        stdio_fd;
sio_fd_t        ppp_fd;

/* ----------------------- Start implementation -----------------------------*/
int
main( void )
{
    char c;
    char buffer[ 6 ];
    EIC_Init(  );
    EIC_IRQConfig( ENABLE );

    /* Use UART0 as stdin/stdout for debug purposes. */
    stdio_fd = sio_open_new( 0, 115200, 8, SIO_STOP_1, SIO_PAR_NONE );

    /* Use UART1 as PPP device. */
    ppp_fd = sio_open_new( 1, 115200, 8, SIO_STOP_1, SIO_PAR_NONE );

    /* Initialize lwIP protocol stack. */
    vlwIPInit(  );

    if( sys_thread_new( vMBServerTask, NULL, mainMB_TASK_PRIORITY ) != NULL )
    {
        vTaskStartScheduler(  );
    }

    /* Should never get here! */
    return 0;
}

void
vlwIPInit( void )
{
    /* Initialize lwIP and its interface layer. */
    sys_init(  );
    mem_init(  );
    memp_init(  );
    pbuf_init(  );
    netif_init(  );
    ip_init(  );
    tcpip_init( NULL, NULL );
    pppInit( );
    pppSetAuth( PPPAUTHTYPE_NONE, PPP_USER, PPP_PASS );
}

void
vMBServerTask( void *arg )
{
    ePPPThreadControl ePPPThrCtlCur;
    int ppp_con_fd;
    do
    {   
        vPortEnterCritical( );
        ePPPThrCtl = CONNECTING;
        vPortExitCritical( );
        if( ( ppp_con_fd = pppOpen( ppp_fd, vPPPStatusCB, NULL ) ) == PPPERR_NONE )
        {
            /* Check every 50ms if the state of the connecton has changed.
             * This could either mean it was aborted or successful.
             */
            do
            {
                vTaskDelay( ( portTickType ) ( 50UL / portTICK_RATE_MS ) );
                vPortEnterCritical( );
                ePPPThrCtlCur = ePPPThrCtl;
                vPortExitCritical( );
            } while( ePPPThrCtlCur == CONNECTING );
            
            while( ePPPThrCtlCur == CONNECTED )
            {
                vPortEnterCritical( );
                ePPPThrCtlCur = ePPPThrCtl;
                vPortExitCritical( );
                
                /* Application code here. */
                vTaskDelay( ( portTickType ) ( 1000UL / portTICK_RATE_MS ) );
            }
            
            /* FIXME: pppClose bugs because thread is not stopped. */
            /* Connection has been closed. */
            pppClose( ppp_con_fd );
        }

        /* Wait 1s until reopening the connection. */
        vTaskDelay( ( portTickType ) ( 1000UL / portTICK_RATE_MS ) );
    } while( pdTRUE );
}

void 
vPPPStatusCB( void *ctx, int errCode, void *arg)
{
    ePPPThreadControl ePPPThrCtlNew;
    struct ppp_addrs *ppp_addrs;
    switch(errCode) {
    /* No error. */
    case PPPERR_NONE:
        ePPPThrCtlNew = CONNECTED;
        ppp_addrs = arg;
        printf("vPPPStatusCB: PPPERR_NONE\r\n");
        printf(" our_ipaddr=%s\r\n", _inet_ntoa(ppp_addrs->our_ipaddr.addr));
        printf(" his_ipaddr=%s\r\n", _inet_ntoa(ppp_addrs->his_ipaddr.addr));
        printf(" netmask = %s\r\n", _inet_ntoa(ppp_addrs->netmask.addr));
        printf(" dns1 = %s\r\n", _inet_ntoa(ppp_addrs->dns1.addr));
        printf(" dns2 = %s\n\r", _inet_ntoa(ppp_addrs->dns2.addr));
        break;

    /* Invalid parameter. */
     case PPPERR_PARAM:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_PARAM\r\n");
        break;

    /* Unable to open PPP session. */
    case PPPERR_OPEN:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_OPEN\r\n");
        break;
        
    /* Invalid I/O device for PPP. */
    case PPPERR_DEVICE:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_DEVICE\r\n");
        break;

    /* Unable to allocate resources. */
    case PPPERR_ALLOC:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_ALLOC\r\n");
        break;

    /* User interrupt. */
    case PPPERR_USER:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_USER\r\n");
        break;

    /* Connection lost. */
    case PPPERR_CONNECT:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_CONNECT\r\n");
        break;

    /* Failed authentication challenge. */
    case PPPERR_AUTHFAIL:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_AUTHFAIL\r\n");
        break;

    /* Failed to meet protocol. */
    case PPPERR_PROTOCOL:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: PPPERR_PROTOCOL\r\n");
        break;

    default:
        ePPPThrCtlNew = DISCONNECT;
        printf("vPPPStatusCB: unknown errCode %d\r\n", errCode);
        break;
    }
    vPortEnterCritical( );
    ePPPThrCtl = ePPPThrCtlNew;
    vPortExitCritical( );
}

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
    return MB_ENOREG;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
