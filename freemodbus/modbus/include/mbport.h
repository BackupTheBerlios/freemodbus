 /*
  * FreeRTOS Modbus Libary: A Modbus serial implementation for FreeRTOS
  * Copyright (C) 2006 Christian Walter <cwalter@s-can.at>
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
  * File: $Id: mbport.h,v 1.1 2006/02/19 15:00:53 wolti Exp $
  */

#ifndef _MB_PORT_H
#define _MB_PORT_H

/* ----------------------- Supporting functions -----------------------------*/
BOOL            xMBPortEventPost( eMBEventType eEvent );

/* ----------------------- Serial port functions ----------------------------*/

BOOL            xMBPortSerialInit( ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity );

void            vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable );

inline BOOL     xMBPortSerialGetByte( CHAR *pucByte );

inline BOOL     xMBPortSerialPutByte( CHAR ucByte );

/* ----------------------- Timers functions ---------------------------------*/
BOOL            xMBPortTimersInit( USHORT usTim1Timerout100us, USHORT usTim2Timerout100us );

inline void     vMBPortTimersEnable(  );

inline void     vMBPortTimersDisable(  );

/* ----------------------- Callback for the protocol stack ------------------*/

/*!
 * \brief Callback function for the porting layer when a new byte is
 *   available.
 *
 * Depending upon the mode this callback function is used by the RTU or
 * ASCII transmission layers. In any case a call to xMBPortSerialGetByte()
 * must immediately return a new character.
 * 
 * \return <code>TRUE</code> if a event was posted to the queue because
 *   a new byte was received. The port implementation should wake up the
 *   tasks which are currently blocked on the eventqueue.
 */
BOOL            ( *pxMBFrameCBByteReceived ) ( void );

BOOL            ( *pxMBFrameCBTransmitterEmpty ) ( void );

BOOL            ( *pxMBPortCBTimer1Expired ) ( void );

BOOL            ( *pxMBPortCBTimer2Expired ) ( void );

#endif
