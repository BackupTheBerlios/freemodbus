/*
 * FreeRTOS Modbus Library demo application
 * Copyright (C) 2006 Christian Walter <cwalter@s-can.at>
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
 * File: $Id: simple.c,v 1.1 2006/02/19 15:00:46 wolti Exp $
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

/* ----------------------- Defines ------------------------------------------*/
#define xArrNElems( x ) ( sizeof( x ) / ( sizeof( x[ 0 ] ) ) )

/* ----------------------- Static variables ---------------------------------*/
static unsigned portSHORT usRegInputStart = 1000;
static unsigned portSHORT usRegInputBuf[4];

xQueueHandle    xMBPortQueueHdl;

/* ----------------------- Static functions ---------------------------------*/
static void     vInitTask( void *pvParameters );
static void     vMeasureTask( void *pvParameters );

static eMBErrorCode
prveInputRegister( unsigned portCHAR * pusRegBuffer, unsigned portSHORT usAddress,
                   unsigned portSHORT usNRegs, eMBRegisterMode eMode );

/* ----------------------- Start implementation -----------------------------*/
int
main( void )
{
    EIC_Init(  );
    EIC_IRQConfig( ENABLE );

    ( void )xTaskCreate( vInitTask, NULL, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
    vTaskStartScheduler(  );

    return 0;
}

static void
vInitTask( void *pvParameters )
{
    const unsigned portCHAR ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
    eMBErrorCode    eStatus;
    eMBEventType    eEvent;

    /* The FreeRTOS port uses a FreeRTOS queue for the event implementation. */
    xMBPortQueueHdl = xQueueCreate( 1, sizeof( eMBEventType ) );
    assert( xMBPortQueueHdl != NULL );

    /* Select either ASCII or RTU Mode. */
    eStatus = eMBInit( MB_ASCII, 0x0A, 9600, MB_PAR_EVEN );
    assert( eStatus == MB_ENOERR );
    //eStatus = eMBInit(MB_RTU, 0x0A, 9600, MB_PAR_EVEN);
    //assert( eStatus == MB_ENOERR );

    /* Configure the slave id of the device. */
    eStatus = eMBSetSlaveID( ucSlaveID, 3, pdTRUE );
    assert( eStatus == MB_ENOERR );

    /* Register a memory block for Input registers. */
    eStatus = eMBAddRegister( MB_REG_INPUT, usRegInputStart, xArrNElems( usRegInputBuf ), prveInputRegister );
    assert( eStatus == MB_ENOERR );


    /* Enable the Modbus Protocol Stack. */
    eStatus = eMBEnable(  );

    for( ;; )
    {
        /* The Modbus RTU or ASCII frames are sent and received in interrupt 
         * driven state machines because of the strict timing requirements.
         * If a complete frame has been received or sent a event is posted
         * to a queue (The actual implementation depends on the port. See
         * xMBPortEventPost(  ).
         *
         * This allows the protocol stack to transfer handling of the
         * Modbus Application Protocol into a user task and therefore reduces
         * the time spent in interrupts. The minimum time intervall required
         * for calling eMBPool(  ) is given by the configured Modbus timeouts.
         */
        if( xQueueReceive( xMBPortQueueHdl, &eEvent, portMAX_DELAY ) == pdTRUE )
        {
            eMBPool( eEvent );
        }

        /* Application specific actions. Here we simply count the number
         * of poll cycles. */
        usRegInputBuf[0]++;
    }
}

static eMBErrorCode
prveInputRegister( unsigned portCHAR * pusRegBuffer, unsigned portSHORT usAddress,
                   unsigned portSHORT usNRegs, eMBRegisterMode eMode )
{
    int             iRegIndex = usAddress - usRegInputStart;

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
