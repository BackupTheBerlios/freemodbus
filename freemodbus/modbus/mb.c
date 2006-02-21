 /*
  * FreeRTOS Modbus Libary: A Modbus serial implementation for FreeRTOS
  * Copyright (C) 2006 Christian Walter <wolti@sil.at>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  *
  * File: $Id: mb.c,v 1.4 2006/02/21 23:11:24 wolti Exp $
  */

/* ----------------------- System includes ----------------------------------*/
#include "assert.h"
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbfunc.h"

#include "mbport.h"
#include "mbrtu.h"
#if MB_ASCII_ENABLED == 1
#include "mbascii.h"
#endif

/* ----------------------- Static variables ---------------------------------*/
static UCHAR *ucMBFrame;
static UCHAR ucMBAddress;

static peMBFrameSend peMBFrameSendCur;
static peMBFrameStart peMBFrameStartCur;
static peMBFrameReceive peMBFrameReceiveCur;
static peMBFrameInit peMBFrameInitCur;

BOOL( *pxMBFrameCBByteReceived ) ( void );
BOOL( *pxMBFrameCBTransmitterEmpty ) ( void );
BOOL( *pxMBPortCBTimer1Expired ) ( void );
BOOL( *pxMBPortCBTimer2Expired ) ( void );

BOOL( *pxMBFrameCBReceiveFSMCur ) ( void );
BOOL( *pxMBFrameCBTransmitFSMCur ) ( void );

static xMBFunctionHandler xFuncHandlers[MB_FUNC_HANDLERS_MAX] = {
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
    {MB_FUNC_OTHER_REPORT_SLAVEID, eMBFuncReportSlaveID},
#endif
#if MB_FUNC_READ_INPUT_ENABLED > 0
    {MB_FUNC_READ_INPUT_REGISTER, eMBFuncReadInputRegister},
#endif
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
    {MB_FUNC_READ_HOLDING_REGISTER, eMBFuncReadHoldingRegister},
#endif
#if MB_FUNC_READ_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_REGISTER, eMBFuncWriteRegister},
#endif
};

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBInit( eMBMode eMode, UCHAR ucSlaveAddress, ULONG ulBaudRate, eMBParity eParity )
{
    eMBErrorCode eStatus = MB_ENOERR;

    /* check preconditions */
    if( ( ucSlaveAddress == MB_ADDRESS_BROADCAST ) ||
        ( ucSlaveAddress < MB_ADDRESS_MIN ) || ( ucSlaveAddress > MB_ADDRESS_MAX ) )
    {
        return MB_EINVAL;
    }
    ucMBAddress = ucSlaveAddress;

    switch ( eMode )
    {
        case MB_RTU:
            peMBFrameInitCur = eMBRTUInit;
            peMBFrameStartCur = eMBRTUStart;
            peMBFrameSendCur = eMBRTUSend;
            peMBFrameReceiveCur = eMBRTUReceive;

            pxMBFrameCBByteReceived = xMBRTUReceiveFSM;
            pxMBFrameCBTransmitterEmpty = xMBRTUTransmitFSM;
            pxMBPortCBTimer1Expired = xMBRTUTimerT15Expired;
            pxMBPortCBTimer2Expired = xMBRTUTimerT35Expired;
            break;
        case MB_ASCII:
#if MB_ASCII_ENABLED > 0
            peMBFrameInitCur = eMBASCIIInit;
            peMBFrameStartCur = eMBASCIIStart;
            peMBFrameSendCur = eMBASCIISend;
            peMBFrameReceiveCur = eMBASCIIReceive;

            pxMBFrameCBByteReceived = xMBASCIIReceiveFSM;
            pxMBFrameCBTransmitterEmpty = xMBASCIITransmitFSM;
            pxMBPortCBTimer1Expired = xMBASCIITimerT1SExpired;
            pxMBPortCBTimer2Expired = NULL;
#else
            assert( eMode != MB_ASCII );
#endif
    }

    eStatus = peMBFrameInitCur( ucSlaveAddress, ulBaudRate, eParity );
    if( eStatus == MB_ENOERR )
    {
        if( xMBPortEventInit(  ) != TRUE )
        {
            eStatus = MB_EPORTERR;
        }
    }
    return eStatus;
}

eMBErrorCode
eMBEnable( void )
{
    eMBErrorCode eStatus = MB_ENOERR;

    if( peMBFrameStartCur != NULL )
    {
        /* Activate the protocol stack. */
        peMBFrameStartCur(  );
    }
    else
    {
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

eMBErrorCode
eMBPool(  )
{
    static UCHAR ucRcvAddress;
    static UCHAR ucFunctionCode;
    static USHORT usLength;
    static eMBException eException;

    int i;
    eMBErrorCode eStatus;
    eMBEventType eEvent;

    /* Check if there is a event available. If not return control to caller.
     * Otherwise we will handle the event. */
    if( xMBPortEventGet( &eEvent ) == TRUE )
    {
        switch ( eEvent )
        {
            case EV_READY:
                eStatus = MB_ENOERR;
                break;

            case EV_FRAME_RECEIVED:
                eStatus = peMBFrameReceiveCur( &ucRcvAddress, &ucMBFrame, &usLength );
                if( eStatus == MB_ENOERR )
                {
                    /* Check if the frame is for us. If not ignore the frame. */
                    if( ( ucRcvAddress == ucMBAddress ) || ( ucRcvAddress == MB_ADDRESS_BROADCAST ) )
                    {
                        ( void )xMBPortEventPost( EV_EXECUTE );
                    }
                }
                break;

            case EV_EXECUTE:
                ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
                eException = MB_EX_ILLEGAL_FUNCTION;
                for( i = 0; i < MB_FUNC_HANDLERS_MAX; i++ )
                {
                    /* No more function handlers registered. Abort. */
                    if( xFuncHandlers[i].ucFunctionCode == 0 )
                    {
                        break;
                    }
                    else if( xFuncHandlers[i].ucFunctionCode == ucFunctionCode )
                    {
                        eException = xFuncHandlers[i].pxHandler( ucMBFrame, &usLength );
                        break;
                    }
                }

                /* If the request was not sent to the broadcast address we 
                 * return a reply. */
                if( ucRcvAddress != MB_ADDRESS_BROADCAST )
                {
                    if( eException != MB_EX_NONE )
                    {
                        /* An exception occured. Build an error frame. */
                        usLength = 0;
                        ucMBFrame[usLength++] = ucFunctionCode | MB_FUNC_ERROR;
                        ucMBFrame[usLength++] = eException;
                    }
                    eStatus = peMBFrameSendCur( ucMBAddress, ucMBFrame, usLength );
                }
                break;

            case EV_FRAME_SENT:
                eStatus = MB_ENOERR;
                break;
        }
    }
    return eStatus;
}
