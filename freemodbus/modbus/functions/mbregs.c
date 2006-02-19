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
  */

/* ----------------------- System includes ----------------------------------*/
#include "assert.h"
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"
#include "mbregs.h"

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    xRegBuffer     *xRegBuffers;
    USHORT          xRegBufferCnt;
    USHORT          xRegBufferMax;
} xRegMapping;

/* ----------------------- Static variables ---------------------------------*/

static xRegBuffer xRegDisc[MB_FUNC_DATA_DISCRETE_BLK_MAX];
static xRegBuffer xRegCoils[MB_FUNC_DATA_COILS_BLK_MAX];
static xRegBuffer xRegInput[MB_FUNC_DATA_INPUT_BLK_MAX];
static xRegBuffer xRegHolding[MB_FUNC_DATA_HOLDING_BLK_MAX];

static xRegMapping xMBRegMapping[] = {
    /* register mapping for discrete input registers. */
    {&xRegDisc[0], 0, MB_FUNC_DATA_DISCRETE_BLK_MAX},

    /* register mapping for coil registers.  */
    {&xRegCoils[0], 0, MB_FUNC_DATA_COILS_BLK_MAX},

    /* register mapping for input registers. */
    {&xRegInput[0], 0, MB_FUNC_DATA_INPUT_BLK_MAX},

    /* register mapping for holding regisers. */
    {&xRegHolding[0], 0, MB_FUNC_DATA_HOLDING_BLK_MAX}
};

/* ----------------------- Static functions ---------------------------------*/

xRegBuffer     *prvpxMBRegLookupBuffer( eMBRegister eRegister, USHORT usAddress );

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBAddRegister( eMBRegister eRegister, USHORT usRegAddress, USHORT usNRegs, peMBRegHandler pxRegHandler )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    xRegBuffer     *pxRegBuffer;

    if( ( usRegAddress == 0 ) || ( pxRegHandler == NULL ) || ( usNRegs == 0 ) )
        return MB_EINVAL;

    pxRegBuffer = &xMBRegMapping[eRegister].xRegBuffers[0];
    if( xMBRegMapping[eRegister].xRegBufferCnt < xMBRegMapping[eRegister].xRegBufferMax )
    {
        pxRegBuffer += xMBRegMapping[eRegister].xRegBufferCnt;
        pxRegBuffer->usStartAddress = usRegAddress;
        pxRegBuffer->usEndAddress = usRegAddress + usNRegs - 1;
        pxRegBuffer->pxRegHandler = pxRegHandler;
        xMBRegMapping[eRegister].xRegBufferCnt++;
    }
    else
    {
        eStatus = MB_ENORES;
    }

    return eStatus;
}

xRegBuffer     *
prvpxMBRegLookupBuffer( eMBRegister eRegister, USHORT usAddress )
{
    xRegBuffer     *pxRegBuffer;
    USHORT          pxRegBufferPos;
    USHORT          pxRegBufferCnt;

    pxRegBuffer = &xMBRegMapping[eRegister].xRegBuffers[0];
    pxRegBufferCnt = xMBRegMapping[eRegister].xRegBufferCnt;

    for( pxRegBufferPos = 0; pxRegBufferPos < pxRegBufferCnt; pxRegBufferPos++ )
    {
        if( usAddress >= pxRegBuffer[pxRegBufferPos].usStartAddress &&
            usAddress <= pxRegBuffer[pxRegBufferPos].usEndAddress )
        {
            break;
        }
    }

    return pxRegBufferPos == pxRegBufferCnt ? NULL : &pxRegBuffer[pxRegBufferPos];
}
