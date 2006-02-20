/*
 * FreeRTOS Modbus Library demo application
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
 * File: $Id: simple2.c,v 1.2 2006/02/20 18:15:53 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "assert.h"

/* ----------------------- Platform includes --------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ----------------------- STR71X includes ----------------------------------*/
#include "eic.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"

/* ----------------------- Static variables ---------------------------------*/
static unsigned portSHORT   usRegInputBuf[4];

/* ----------------------- Static functions ---------------------------------*/
static void     vModbusTask( void *pvParameters );

static eMBErrorCode
prveInputRegister( unsigned portCHAR * pusRegBuffer, unsigned portSHORT usAddress,
                   unsigned portSHORT usNRegs, eMBRegisterMode eMode );

/* ----------------------- Start implementation -----------------------------*/
int
main( void )
{
    EIC_Init(  );
    EIC_IRQConfig( ENABLE );

    ( void )xTaskCreate( vModbusTask, NULL, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );

    vTaskStartScheduler(  );
    return 0;
}

static void
vModbusTask( void *pvParameters )
{
    portTickType xLastWakeTime;
    eMBEventType    eEvent;

    /* Select either ASCII or RTU Mode. */
    eMBInit( MB_RTU, 0x0A, 38400, MB_PAR_EVEN );
    /* Register a memory block for Input registers. */
    eMBAddRegister( MB_REG_INPUT, 1000, 4, prveInputRegister );
    /* Enable the Modbus Protocol Stack. */
    eMBEnable(  );
    for( ;; )
    {
        /* Call the main polling loop of the Modbus protocol stack. */
        eMBPool(  );
        
        /* Application specific actions. Count the number of poll cycles. */
        usRegInputBuf[0]++;

        /* Hold the current FreeRTOS ticks. */
        xLastWakeTime =  xTaskGetTickCount();
        usRegInputBuf[1] = ( unsigned portSHORT )( xLastWakeTime >> 16UL );
        usRegInputBuf[2] = ( unsigned portSHORT )( xLastWakeTime & 0xFFFFUL );

        /* The constant value. */
        usRegInputBuf[3] = 33;
    }
}

static eMBErrorCode
prveInputRegister( unsigned portCHAR * pusRegBuffer, unsigned portSHORT usAddress,
                   unsigned portSHORT usNRegs, eMBRegisterMode eMode )
{
    int             iRegIndex = usAddress - 1000;
    if( eMode == MB_REG_READ )
    {
        while( usNRegs > 0 )
        {
            *pusRegBuffer++ = usRegInputBuf[iRegIndex] >> 8;
            *pusRegBuffer++ = usRegInputBuf[iRegIndex] & 0xFF;
            iRegIndex++;
            usNRegs--;
        }
    }
    return MB_ENOERR;
}

void
__assert( const char *pcFile, const char *pcLine, int iLineNumber )
{
    portENTER_CRITICAL(  );
    while( 1 );
}
