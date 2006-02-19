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
  * File: $Id: mb.h,v 1.1 2006/02/19 15:00:53 wolti Exp $
  */

#ifndef _MB_H
#define _MB_H

#include "port.h"

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    MB_RTU,
    MB_ASCII
} eMBMode;

typedef enum
{
    MB_PAR_NONE,
    MB_PAR_ODD,
    MB_PAR_EVEN
} eMBParity;

typedef enum
{
    MB_REG_DISCRETE = 0x00,
    MB_REG_COILS = 0x01,
    MB_REG_INPUT = 0x02,
    MB_REG_HOLDING = 0x03
} eMBRegister;

typedef enum
{
    MB_REG_READ,
    MB_REG_WRITE
} eMBRegisterMode;

typedef enum
{
    MB_ENOERR,                  /*!< no error. */
    MB_ENOSUCHREG,              /*!< illegal register addresss. */
    MB_EINVAL,                  /*!< illegal argument. */
    MB_EPORTERR,                /*!< porting layer error. */
    MB_ENORES,                  /*!< insufficient resources. */
    MB_EIO,                     /*!< I/O error. */
    MB_EILLSTATE,               /*!< protocol stack in illegal state. */
    MB_ETIMEDOUT                /*!< timeout error occured. */
} eMBErrorCode;

typedef enum
{
    EV_READY,                   /*!< Startup finished. */
    EV_FRAME_RECEIVED,          /*!< Frame received. */
    EV_EXECUTE,                 /*!< Execute function. */
    EV_FRAME_SENT,              /*!< Frame sent. */
} eMBEventType;

/* A function which is called whenever the Modbus protocol stack receives
 * a read or write request for the registered register range.
 *
 * \param eMode If eMode is eMBRegisterMode::MB_REG_WRITE the application 
 *   register values should be updated. If eMode is eMBRegisterMode::MB_REG_READ
 *   the Modbus application protocol stack wants to know the current values. 
 *
 * \param pusRegBuffer If eMode is MB_REG_READ the current register
 *   values must be written to the buffer pusRegBuffer. I.e the register
 *   with offset one should be written to pucRegBuffer[0] = high byte and to 
 *   pucRegBuffer[1] = low byte.<br>
 *   If eMode is MB_REG_WRITE the application register values should
 *   be updated. For each register starting at address usAddress two
 *   bytes are available in the buffer where the first byte is the high
 *   byte of the register value.
 *
 * \param usAddress The register start address.
 *
 * \param usNRegs Number of registers.
 *
 * \return eMBRegisterMode::MB_ENOERR is no error occurred. The other return
 *   values result in a Modbus Application Protocol exception. This should not 
 *   be used for application error reporting (See the section exceptions in the
 *   Modbus standard). The following other return codes are allowed:
 *   eMBErrorCode::MB_ETIMEDOUT If a timeout error occurred. This results
 *     in <b>SLAVE DEVICE BUSY</b> exception.
 *   eMBErrorCode::MB_EIO If an error occurred. This results in a 
 *     <b>SLAVE DEVICE FAILURE</b> exception.
 */
typedef
    eMBErrorCode ( *peMBRegHandler ) ( UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode );

/* ----------------------- Function prototypes ------------------------------*/
eMBErrorCode    eMBInit( eMBMode eMode, UCHAR ucSlaveAddress, ULONG eBaudRate, eMBParity eParity );

eMBErrorCode    eMBEnable( void );

eMBErrorCode    eMBPool( eMBEventType eEvent );

eMBErrorCode    eMBSetSlaveID( UCHAR const *pucSlaveID, USHORT usSlaveIDLen, BOOL xIsRunning );

eMBErrorCode
   eMBAddRegister( eMBRegister eRegister, USHORT usRegAddress, USHORT usNRegs, peMBRegHandler pxRegHandler );

#endif
