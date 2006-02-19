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
  * File: $Id: mbfunc.h,v 1.2 2006/02/19 17:15:09 wolti Exp $
  */

#ifndef _MB_FUNC_H
#define _MB_FUNC_H

eMBException eMBFuncReportSlaveID( UCHAR *pucFrame, USHORT * usLen );

#if MB_FUNC_DATA_INPUT_BLK_MAX > 0
eMBException    eMBFuncReadInputRegister( UCHAR *pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_DATA_HOLDING_BLK_MAX > 0
eMBException    eMBFuncReadHoldingRegister( UCHAR *pucFrame, USHORT * usLen );

eMBException    eMBFuncWriteRegister( UCHAR *pucFrame, USHORT * usLen );
#endif

#endif
