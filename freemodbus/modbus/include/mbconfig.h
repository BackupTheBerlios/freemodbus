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
  * File: $Id: mbconfig.h,v 1.1 2006/02/19 15:00:53 wolti Exp $
  */

#ifndef _MB_CONFIG_H
#define _MB_CONFIG_H

/* ----------------------- Defines ------------------------------------------*/
#define MB_ASCII_ENABLED                        (  1 )
#define MB_ASCII_TIMEOUT_SEC                    (  1 )

/*! 
 * Maximum number of supported Modbus functions. Note that at least some
 * of the handlers are required by the protocol stack itself.
 */
#define MB_FUNC_HANDLERS_MAX                    ( 16 )

/*! Number of bytes which should be allocated for the report slave id
 *  command.
 */
#define MB_FUNC_OTHER_REP_SLAVEID_BUF           ( 32 )

#define MB_FUNC_DATA_DISCRETE_BLK_MAX           (  0 )

#define MB_FUNC_DATA_COILS_BLK_MAX              (  0 )

#define MB_FUNC_DATA_INPUT_BLK_MAX              ( 16 )

#define MB_FUNC_DATA_HOLDING_BLK_MAX            ( 16 )

#endif
