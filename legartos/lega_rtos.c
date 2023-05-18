/*
 * Copyright (c) 2022 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "lega_rtos_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "string.h"
#include "semphr.h"
#include "stdio.h"
#include "event_groups.h"
#include "alto_common.h"
/******************************************************
 *                      Macros
 ******************************************************/

#ifndef TIMER_THREAD_STACK_SIZE
#define TIMER_THREAD_STACK_SIZE      (1024 + 4*1024)
#endif
#define TIMER_QUEUE_LENGTH  5

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskBLOCKED_CHAR     ( 'B' )
#define tskREADY_CHAR       ( 'R' )
#define tskDELETED_CHAR     ( 'D' )
#define tskSUSPENDED_CHAR   ( 'S' )

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    event_handler_t function;
    void*           arg;
} timer_queue_message_t;

typedef struct
{
    event_handler_t function;
    void*           arg;
} lega_event_message_t;


/******************************************************
 *               Variables Definitions
 ******************************************************/

//static xTaskHandle  app_thread_handle;

//static lega_time_t lega_time_offset = 0;

uint32_t  max_syscall_int_prio;
uint32_t  ms_to_tick_ratio = 1;

lega_task_config_t task_cfg[LEGA_TASK_CONFIG_MAX] = {
#ifdef USE_SMALL_STACK
    {LEGA_WPA3_AUTH_TASK_PRIORITY, 2560},
    {LEGA_LWIFI_TASK_PRIORITY, 2560},
#else
    {LEGA_WPA3_AUTH_TASK_PRIORITY, 13312},
    {LEGA_LWIFI_TASK_PRIORITY, 2048},
#endif
    {LEGA_UWIFI_RX_TASK_PRIORITY, 4096},
    {LEGA_UWIFI_TASK_PRIORITY, 7168},
    {LEGA_LWIP_DHCP_TASK_PRIORITY, 1536},
    {LEGA_BLE_SCHEDULER_PRIORITY, 2048},
    {LEGA_CHIP_TASK_PRIORITY, 1024*8},
};

///////////////////////
FLASH_LWIFI_SEG SPEED_OPTIMIZATION OSBool lega_rtos_is_in_interrupt_context(void)
{
    rv_csr_t __msubm = __RV_CSR_READ(CSR_MSUBM);
    if(((__msubm>>6) & 0x3) == 1)
        return TRUE;
    else
        return  FALSE;
}

OSStatus  lega_rtos_get_chip_task_cfg(lega_task_config_t *cfg)
{
return lega_rtos_task_cfg_get(LEGA_TASK_CONFIG_CHIP, cfg);
}

/*******************************************************************
* Name: lega_task_cfg_init
* Description: this api get wifi task config
* input param: task index defined in lega_rtos.h
* output param: config data of wifi task
* return: OSSatus value
*******************************************************************/
OSStatus lega_rtos_task_cfg_get(uint32_t index, lega_task_config_t *cfg)
{
    if (!cfg || (index >= LEGA_TASK_CONFIG_MAX)) {
        return kGeneralErr;
    }
    cfg->task_priority = task_cfg[index].task_priority;
    cfg->stack_size = task_cfg[index].stack_size;

    return kNoErr;
}

//uint32_t lega_rtos_max_priorities = RTOS_HIGHEST_PRIORITY - RTOS_LOWEST_PRIORITY + 1;


/*******************************************************************
* Name: lega_rtos_create_thread
* Description: this api call freeRTOS function "xTaskCreate" to create task
* input param: task priority, stack_size, function, name
* output param: thread
* return: OSSatus value
*******************************************************************/
OSStatus lega_rtos_create_thread( lega_thread_t* thread,
                                  uint8_t priority,
                                  const char* name,
                                  lega_thread_function_t function,
                                  uint32_t stack_size,
                                  uint32_t arg )
{
      /* Limit priority to default lib priority */
    if ( priority > RTOS_HIGHEST_PRIORITY )
    {
        priority = RTOS_HIGHEST_PRIORITY;
    }

    if( pdPASS == xTaskCreate( (native_thread_t)function, \
                                 name, \
                                (unsigned short) (stack_size/sizeof( portSTACK_TYPE )), \
                                 (void *)arg, priority, (TaskHandle_t *)thread ) )
    {
        return kNoErr;
    }
    else
    {
        return kGeneralErr;
    }

}


/*******************************************************************
* Name: lega_rtos_delete_thread
* Description: this api calls FreeRTOS function "xTaskDelete" to delete a task
* input param: thread
* output param: none
* return:  OSStatus
*******************************************************************/
OSStatus lega_rtos_delete_thread( lega_thread_t* thread )
{
    if ( thread == NULL )
    {
        vTaskDelete( NULL );
    }
    else //if ( xTaskIsTaskFinished( *thread ) != pdTRUE )
    {
        vTaskDelete( *thread );
    }
    return kNoErr;
}


/*******************************************************************
* Name: lega_rtos_thread_join
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_thread_join( lega_thread_t* thread )
{
    lega_thread_t tmp =NULL;
    if(thread ==NULL)
        return kNoErr ;
    tmp = *thread;
    if ( tmp == NULL )
        return kNoErr;

//    while ( xTaskIsTaskFinished( tmp ) != pdTRUE )
//    {
    lega_rtos_delay_milliseconds( 10 );
//    }
    return kNoErr;

}


/*******************************************************************
* Name:lega_rtos_is_current_thread
* Description: judge if the thread is current thread
* input param: thread
* output param: none
* return: true or false
*******************************************************************/
OSBool lega_rtos_is_current_thread( lega_thread_t* thread )
{
    if ( xTaskGetCurrentTaskHandle( ) == *thread )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*******************************************************************
* Name:lega_rtos_get_current_thread
* Description: get current thread handle
* input param: none
* output param: none
* return: thread handle
*******************************************************************/
lega_thread_t* lega_rtos_get_current_thread( void )
{
    return (lega_thread_t *)xTaskGetCurrentTaskHandle();
}

/*******************************************************************
* Name:prvWriteNameToBuffer
* Description: write task name into pcb buffer
* input param: pcTaskName,
* output param: pcBuffer
* return:pcBuffer
*******************************************************************/
static char *prvWriteNameToBuffer( char *pcBuffer, const char *pcTaskName )
{
    long x;

    /* Start by copying the entire string. */
    strcpy( pcBuffer, pcTaskName );

    /* Pad the end of the string with spaces to ensure columns line up when
    printed out. */
    for( x = strlen( pcBuffer ); x < ( configMAX_TASK_NAME_LEN - 1 ); x++ )
    {
        pcBuffer[ x ] = ' ';
    }

    /* Terminate. */
    pcBuffer[ x ] = 0x00;

    /* Return the new end of string. */
    return &( pcBuffer[ x ] );
}


/*******************************************************************
* Name:lega_rtos_print_thread_status
* Description: print task status
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_print_thread_status( char* pcWriteBuffer, int xWriteBufferLen )
{
    TaskStatus_t *pxTaskStatusArray;
    unsigned portBASE_TYPE uxCurrentNumberOfTasks = uxTaskGetNumberOfTasks();
    volatile UBaseType_t uxArraySize, x;
    char cStatus;
    char pcTaskStatusStr[48];
    char *pcTaskStatusStrTmp;

    /* Make sure the write buffer does not contain a string. */
    *pcWriteBuffer = 0x00;

    /* Take a snapshot of the number of tasks in case it changes while this
    function is executing. */
    uxArraySize = uxCurrentNumberOfTasks;

    /* Allocate an array index for each task.  NOTE!  if
     configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
     equate to NULL. */
    pxTaskStatusArray = pvPortMalloc( uxCurrentNumberOfTasks * sizeof(TaskStatus_t) );

//    cmd_printf("%-12s Status     Prio    Stack   TCB\r\n", "Name");
//    cmd_printf("-------------------------------------------\r\n");

    printf("%-12s Status     Prio    Stack   TCB\r\n", "Name");
    printf("-------------------------------------------\r\n");

    xWriteBufferLen-=strlen(pcWriteBuffer);

    if ( pxTaskStatusArray != NULL )
    {
        /* Generate the (binary) data. */
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, NULL );

        /* Create a human readable table from the binary data. */
        for ( x = 0; x < uxArraySize; x++ )
        {
            switch ( pxTaskStatusArray[x].eCurrentState )
            {
                case eReady:
                    cStatus = tskREADY_CHAR;
                    break;

                case eBlocked:
                    cStatus = tskBLOCKED_CHAR;
                    break;

                case eSuspended:
                    cStatus = tskSUSPENDED_CHAR;
                    break;

                case eDeleted:
                    cStatus = tskDELETED_CHAR;
                    break;

                default: /* Should not get here, but it is included
                 to prevent static checking errors. */
                    cStatus = 0x00;
                    break;
            }

            /* Write the task name to the string, padding with spaces so it
             can be printed in tabular form more easily. */
            pcTaskStatusStrTmp = pcTaskStatusStr;
            pcTaskStatusStrTmp = prvWriteNameToBuffer( pcTaskStatusStrTmp, pxTaskStatusArray[x].pcTaskName );
            //pcWriteBuffer = prvWriteNameToBuffer( pcWriteBuffer, pxTaskStatusArray[x].pcTaskName );

            /* Write the rest of the string. */
            sprintf( pcTaskStatusStrTmp, "\t%c\t%u\t%u\t%u\r\n", cStatus,
                     ((unsigned int) pxTaskStatusArray[x].uxCurrentPriority),
                     (unsigned int) pxTaskStatusArray[x].usStackHighWaterMark,
                     (unsigned int) pxTaskStatusArray[x].xTaskNumber );

            if( xWriteBufferLen < strlen( pcTaskStatusStr ) )
            {
                for ( x = 0; x < xWriteBufferLen; x++ )
                {
                    *(pcWriteBuffer+x) = '.';
                }
                break;
            }
            else
            {
                strncpy( pcWriteBuffer, pcTaskStatusStr, xWriteBufferLen);
                xWriteBufferLen -= strlen( pcTaskStatusStr );
                pcWriteBuffer += strlen( pcWriteBuffer );
            }
        }

        /* Free the array again.  NOTE!  If configSUPPORT_DYNAMIC_ALLOCATION
         is 0 then vPortFree() will be #defined to nothing. */
        vPortFree( pxTaskStatusArray );
    }
    else
    {
        mtCOVERAGE_TEST_MARKER( );
    }

    return kNoErr;

}

/*******************************************************************
* Name:
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_check_stack( void )
{
     return kNoErr;
}

/*******************************************************************
* Name:
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_thread_force_awake( lega_thread_t* thread )
{
    xTaskAbortDelay(*thread);

    return kNoErr;
}

/*******************************************************************
* Name:
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_time_get_time(lega_time_t* time_ptr)
{
    return kNoErr;
}


/*******************************************************************
* Name:
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_time_set_time(lega_time_t* time_ptr)
{
    return kNoErr;
}

//////////////////////////////  semaphore  //////////////////////////////////////

/*******************************************************************
* Name:lega_rtos_init_semaphore
* Description: create a count semaphore
* input param: int value,
* output param: lega_semaphore_t* semaphore
* return: kNoErr or kGeneralErr;
*******************************************************************/
OSStatus lega_rtos_init_semaphore( lega_semaphore_t* semaphore, int value )
{
    *semaphore = xSemaphoreCreateCounting( (unsigned portBASE_TYPE)LEGA_SEMAPHORE_COUNT_MAX , (unsigned portBASE_TYPE) value );

    return ( *semaphore != NULL ) ? kNoErr : kGeneralErr;
}


/*******************************************************************
* Name:lega_rtos_get_semaphore
* Description: get semaphore value
* input param:semaphore, timeout_ms
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_get_semaphore( lega_semaphore_t* semaphore, uint32_t timeout_ms )
{
    signed portBASE_TYPE result;
    if(NULL == *semaphore)
    {
        printf("get sema is null, 0x%x\r\n",(unsigned int)semaphore);
    }

    if(lega_rtos_is_in_interrupt_context( ) == TRUE ) //need port
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        result = xSemaphoreTakeFromISR( *semaphore, &xHigherPriorityTaskWoken );

        //check_string( result == pdTRUE, "Unable to set semaphore" );

        /* If xSemaphoreTakeFromISR() unblocked a task, and the unblocked task has
         * a higher priority than the currently executing task, then
         * xHigherPriorityTaskWoken will have been set to pdTRUE and this ISR should
         * return directly to the higher priority unblocked task.
         */
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }else
    {
         result = xSemaphoreTake( *semaphore, (portTickType) ( timeout_ms / ms_to_tick_ratio ) );
    }
    return ( result == pdPASS )? kNoErr : kGeneralErr;
}


/*******************************************************************
* Name:
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_set_semaphore( lega_semaphore_t* semaphore )
{
    signed portBASE_TYPE result;

    if(NULL == *semaphore)
    {
        printf("set sema is null, 0x%x\r\n",(unsigned int)semaphore);
    }

    if ( lega_rtos_is_in_interrupt_context( ) == TRUE )  // need port
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        result = xSemaphoreGiveFromISR( *semaphore, &xHigherPriorityTaskWoken );

        //check_string( result == pdTRUE, "Unable to set semaphore" );

        /* If xSemaphoreGiveFromISR() unblocked a task, and the unblocked task has
         * a higher priority than the currently executing task, then
         * xHigherPriorityTaskWoken will have been set to pdTRUE and this ISR should
         * return directly to the higher priority unblocked task.
         */
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }
    else
    {
        result = xSemaphoreGive( *semaphore );
        //check_string( result == pdTRUE, "Unable to set semaphore" );
    }

    return ( result == pdPASS )? kNoErr : kGeneralErr;
}


/*******************************************************************
* Name:
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_deinit_semaphore( lega_semaphore_t* semaphore )
{
    if (*semaphore != NULL)
    {
        vQueueDelete( *semaphore );
        *semaphore = NULL;
    }
    return kNoErr;
}

/*******************************************************************
* Name:
* Description:
* input param:
* output param:
* return:
*******************************************************************/
OSBool lega_rtos_semaphore_pending_task_null( lega_semaphore_t* semaphore )
{
    return xQueueWaitingTaskEmpty((QueueHandle_t)(*semaphore));
}

//lega_cpsr_t lega_rtos_declare_critical(void)
//{
//    return 0;
//}

/*******************************************************************
* Name:lega_rtos_enter_critical
* Description: get system into critical area where there is no context switch
* input param: none
* output param: none
* return:none
*******************************************************************/
FLASH_LWIFI_SEG SPEED_OPTIMIZATION lega_cpsr_t _lega_rtos_enter_critical( void )
{
    if(lega_rtos_is_in_interrupt_context() == FALSE)
    {
        vPortEnterCritical();
    }
    return 0;
}

/*******************************************************************
* Name:lega_rtos_exit_critical
* Description: get system out of critical area
* input param: none
* output param:none
* return:none
*******************************************************************/
FLASH_LWIFI_SEG SPEED_OPTIMIZATION void _lega_rtos_exit_critical( lega_cpsr_t cpsr_store )
{
    if(lega_rtos_is_in_interrupt_context() == FALSE)
    {
        vPortExitCritical();
    }
}

#ifdef ALTO_BLE_NO_BLOCK
/** \brief BLE int level, init in ble main */
#define HIGH_INTCTL_LEVEL      4

static uint8_t CalcMaxSysCallMTH( uint8_t max_syscall_prio )
{
    uint8_t nlbits = __ECLIC_GetCfgNlbits();
    uint8_t intctlbits = __ECLIC_INTCTLBITS;
    uint8_t lvlbits, lfabits;
    uint8_t maxsyscallmth = 0;
    uint8_t temp;

    if (nlbits <= intctlbits) {
        lvlbits = nlbits;
    } else {
        lvlbits = intctlbits;
    }

    lfabits = 8 - lvlbits;

    temp = ((1<<lvlbits) - 1);
    if (max_syscall_prio > temp) {
        max_syscall_prio = temp;
    }

    maxsyscallmth = (max_syscall_prio << lfabits) | ((1<<lfabits) - 1);

    return maxsyscallmth;
}

void lega_enter_critical_expble(void)
{
    ECLIC_SetMth(CalcMaxSysCallMTH(HIGH_INTCTL_LEVEL - 1));
    __RWMB();
}

void lega_exit_critical_expble(void)
{
    ECLIC_SetMth(0);
    __RWMB();
}
#endif

/*******************************************************************
* Name:lega_rtos_init_mutex
* Description: create a mutext semaphore
* input param: none
* output param: mutex
* return: kNoErr or kGeneralErr
*******************************************************************/
OSStatus lega_rtos_init_mutex( lega_mutex_t* mutex )
{
    // check_string(mutex != NULL, "Bad args");

    /* Mutex uses priority inheritance */
    *mutex = xSemaphoreCreateMutex( );
    if ( *mutex == NULL )
    {
        return kGeneralErr;
    }

    return kNoErr;

}

/*******************************************************************
* Name:lega_rtos_lock_mutex
* Description: take a mutex semaphore
* input param: mutex
* output param: none
* return:
*******************************************************************/
OSStatus lega_rtos_lock_mutex( lega_mutex_t* mutex, uint32_t timeout_ms )
{
    signed portBASE_TYPE result;
    if(NULL == *mutex)
    {
        printf("lock mutex is null, 0x%x\r\n",(unsigned int)mutex);
    }

    if(lega_rtos_is_in_interrupt_context( ) == TRUE ) //need port
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        result = xSemaphoreTakeFromISR( *mutex, &xHigherPriorityTaskWoken );

        //check_string( result == pdTRUE, "Unable to set semaphore" );

        /* If xSemaphoreTakeFromISR() unblocked a task, and the unblocked task has
         * a higher priority than the currently executing task, then
         * xHigherPriorityTaskWoken will have been set to pdTRUE and this ISR should
         * return directly to the higher priority unblocked task.
         */
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }else
    {
         result = xSemaphoreTake( *mutex, (portTickType) ( timeout_ms / ms_to_tick_ratio ) );
    }
    return ( result == pdPASS )? kNoErr : kGeneralErr;
   //  check_string(mutex != NULL, "Bad args");
}


/*******************************************************************
* Name:lega_rtos_unlock_mutex
* Description: give a mutex semaphore
* input param:
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_unlock_mutex( lega_mutex_t* mutex )
{
    signed portBASE_TYPE result;
    if(NULL == *mutex)
    {
        printf("unlock mutex is null, 0x%x\r\n",(unsigned int)mutex);
    }

    if ( lega_rtos_is_in_interrupt_context( ) == TRUE )  // need port
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        result = xSemaphoreGiveFromISR( *mutex, &xHigherPriorityTaskWoken );

        //check_string( result == pdTRUE, "Unable to set semaphore" );

        /* If xSemaphoreGiveFromISR() unblocked a task, and the unblocked task has
         * a higher priority than the currently executing task, then
         * xHigherPriorityTaskWoken will have been set to pdTRUE and this ISR should
         * return directly to the higher priority unblocked task.
         */
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }
    else
    {
        result = xSemaphoreGive( *mutex );
        //check_string( result == pdTRUE, "Unable to set semaphore" );
    }

    return ( result == pdPASS )? kNoErr : kGeneralErr;
}

/*******************************************************************
* Name:lega_rtos_deinit_mutex
* Description: delete a mutex semaphore
* input param: mutex
* output param: none
* return:
*******************************************************************/
OSStatus lega_rtos_deinit_mutex( lega_mutex_t* mutex )
{
  //  check_string(mutex != NULL, "Bad args");

    vSemaphoreDelete( *mutex );
    *mutex = NULL;
    return kNoErr;
}

/*******************************************************************
* Name:lega_rtos_init_queue
* Description: create a queue
* input param: name, mesage size, number_of_message
* output param:
* return:
*******************************************************************/
OSStatus lega_rtos_init_queue( lega_queue_t* queue,
                               const char* name,
                               uint32_t message_size,
                               uint32_t number_of_messages )
{
    //UNUSED_PARAMETER(name);
    if ( ( *queue = xQueueCreate( number_of_messages, message_size ) ) == NULL )
    {
        return kGeneralErr;
    }

    return kNoErr;
}

/*******************************************************************
* Name: lega_rtos_push_to_queue
* Description: send a message to queue;
* input param: queue, message, timeout_ms
* output param: none
* return: kNoRrr,or kGeneralErr;
*******************************************************************/
FLASH_LWIFI_SEG SPEED_OPTIMIZATION OSStatus lega_rtos_push_to_queue( lega_queue_t* queue,
                                  void* message,
                                  uint32_t timeout_ms )
{
    signed portBASE_TYPE result;
    if(NULL == *queue)
    {
        printf("push to queue is null, 0x%x\r\n",(unsigned int)queue);
    }

    if ( lega_rtos_is_in_interrupt_context( ) == TRUE )  // need port
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        result = xQueueSendToBackFromISR( *queue, message, &xHigherPriorityTaskWoken );

        /* If xQueueSendToBackFromISR() unblocked a task, and the unblocked task has
         * a higher priority than the currently executing task, then
         * xHigherPriorityTaskWoken will have been set to pdTRUE and this ISR should
         * return directly to the higher priority unblocked task.
         */
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
        if (result != pdPASS)
            printf("OS_push_to_queue failed1\n");
    }
    else
    {
        result = xQueueSendToBack( *queue, message, (portTickType) ( timeout_ms / ms_to_tick_ratio ) );
        if (result != pdPASS)
            printf("OS_push_to_queue failed2\n");
    }

    return ( result == pdPASS )? kNoErr : kGeneralErr;
}

//xQueueSendToFrontFromISR or xQueueSendToFront

/*******************************************************************
* Name:lega_rtos_push_to_queue_front
* Description: send a message to a queue
* input param: queue, message, timeout
* output param: none
* return:
*******************************************************************/
OSStatus lega_rtos_push_to_queue_front( lega_queue_t* queue,
                                        void* message,
                                        uint32_t timeout_ms )
{
     signed portBASE_TYPE result;
     if(NULL == *queue)
     {
         printf("push to queue front is null, 0x%x\r\n",(unsigned int)queue);
     }

    if ( lega_rtos_is_in_interrupt_context( ) == TRUE )   // need port
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        result = xQueueSendToFrontFromISR( *queue, message, &xHigherPriorityTaskWoken );

        /* If xQueueSendToBackFromISR() unblocked a task, and the unblocked task has
         * a higher priority than the currently executing task, then
         * xHigherPriorityTaskWoken will have been set to pdTRUE and this ISR should
         * return directly to the higher priority unblocked task.
         */
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }
    else
    {
        result = xQueueSendToFront( *queue, message, (portTickType) ( timeout_ms / ms_to_tick_ratio ) );
    }

    return ( result == pdPASS )? kNoErr : kGeneralErr;
}


/*******************************************************************
* Name: lega_rtos_pop_from_queue
* Description: receive a message from a queue
* input param: queue, timeout_ms
* output param:message
* return:kNoErr or kGeneralErr;
*******************************************************************/
OSStatus lega_rtos_pop_from_queue( lega_queue_t* queue,
                                   void* message,
                                   uint32_t timeout_ms )
{
    signed portBASE_TYPE result;
    if(NULL == *queue)
    {
        printf("pop from queue is null, 0x%x\r\n",(unsigned int)queue);
    }

    if ( lega_rtos_is_in_interrupt_context( ) == TRUE )   // need port
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        result = xQueueReceiveFromISR( *queue, message, &xHigherPriorityTaskWoken );

        /* If xQueueReceiveFromISR() unblocked a task, and the unblocked task has
         * a higher priority than the currently executing task, then
         * xHigherPriorityTaskWoken will have been set to pdTRUE and this ISR should
         * return directly to the higher priority unblocked task.
         */
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }
    else
    {
        result = xQueueReceive( *queue, message, (portTickType) ( timeout_ms / ms_to_tick_ratio ) );
    }

    return ( result == pdPASS )? kNoErr : kGeneralErr;

}


/*******************************************************************
* Name:lega_rtos_deinit_queue
* Description: delete a queue
* input param: queue
* output param: none
* return: kNoErr
*******************************************************************/
OSStatus lega_rtos_deinit_queue( lega_queue_t* queue )
{
    vQueueDelete( *queue );
    *queue = NULL;
    return kNoErr;
}


/*******************************************************************
* Name:lega_rtos_is_queue_empty
* Description: get a queue status if it is empty or not
* input param: queue
* output param: none
* return:trur or false
*******************************************************************/
OSBool lega_rtos_is_queue_empty( lega_queue_t* queue ) //used in ISR
{
    signed portBASE_TYPE result;

    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    result = xQueueIsQueueEmptyFromISR( *queue );
    lega_rtos_exit_critical();

    return ( result != 0 ) ? TRUE : FALSE;
}

/*******************************************************************
* Name:lega_rtos_is_queue_full
* Description: get a queue status if it is full or not
* input param: queue
* output param: none
* return: true or false
*******************************************************************/
OSBool lega_rtos_is_queue_full( lega_queue_t* queue ) //used in ISR
{
    signed portBASE_TYPE result;

    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    result = xQueueIsQueueFullFromISR( *queue );
    lega_rtos_exit_critical();

    return ( result != 0 ) ? TRUE : FALSE;
}


/*******************************************************************
* Name:timer_callback
* Description:this function is to execute call function which is defined by user;
* input param: handle
* output param: none
* return: none
*******************************************************************/
static void timer_callback(xTimerHandle handle )
{
    lega_timer_t *timer;

    timer= (lega_timer_t *)pvTimerGetTimerID(handle);

    if(timer->function)
    {
        timer->function(timer->arg);

    }

}

/*******************************************************************
* Name:lega_rtos_is_timer_running
* Description: check if the timer is active or not
* input param: timer
* output param: none
* return: true or false
*******************************************************************/

OSBool lega_rtos_is_timer_running( lega_timer_t* timer )
{
    if (timer->handle)
        return ( xTimerIsTimerActive( timer->handle ) != 0 ) ? TRUE : FALSE;
    else
        return FALSE;
}


/*******************************************************************
* Name:lega_rtos_init_timer
* Description: create a timer
* input param:  time_ms, function, arg
* output param: timer
* return: kNoErr or kGeneralErr
*******************************************************************/
OSStatus lega_rtos_init_timer( lega_timer_t* timer,
                               uint32_t time_ms,
                               timer_handler_t function,
                               void* arg )
{
    // check_string(timer != NULL, "Bad args");

    timer->function = function;
    timer->arg      = arg;

    timer->handle = xTimerCreate( "t", (portTickType)( time_ms / ms_to_tick_ratio ), pdTRUE, timer,  timer_callback );
    if ( timer->handle == NULL )
    {
        printf("OS init timer fail");
        return kGeneralErr;
    }

    return kNoErr;
}

/*******************************************************************
* Name:lega_rtos_start_timer
* Description: start a timer thing
* input param: timer
* output param:none
* return:kNoRrr or kGeneralErr
*******************************************************************/

OSStatus lega_rtos_start_timer( lega_timer_t* timer )
{
    signed portBASE_TYPE result;

    if(timer->handle)
    {
        if ( lega_rtos_is_in_interrupt_context( ) == TRUE ) {
            signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            result = xTimerStartFromISR(timer->handle, &xHigherPriorityTaskWoken );
            portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
        } else
            result = xTimerStart( timer->handle, LEGA_WAIT_FOREVER );

        if ( result != pdPASS )
        {
            return kGeneralErr;
        }
    }

    return kNoErr;
}

/*******************************************************************
* Name:lega_rtos_stop_timer
* Description: this function is to stop timer function
* input param: timer
* output param: none
* return: kNoErr or kGeneralErr;
*******************************************************************/
OSStatus lega_rtos_stop_timer( lega_timer_t* timer )
{
    signed portBASE_TYPE result;

    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    if(timer->handle)
    {
        if ( lega_rtos_is_in_interrupt_context( ) == TRUE ) {
            signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            result = xTimerStopFromISR(timer->handle, &xHigherPriorityTaskWoken );
            portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
        } else
            result = xTimerStop( timer->handle, LEGA_WAIT_FOREVER );

        if ( result != pdPASS )
        {
            return kGeneralErr;
        }
    }
    lega_rtos_exit_critical();

    return kNoErr;
}

/*******************************************************************
* Name: lega_rtos_reload_timer
* Description: reset timer
* input param: tmer
* output param: none
* return: kNoeErr or kGeneralErr
*******************************************************************/
OSStatus lega_rtos_reload_timer( lega_timer_t* timer )
{
    signed portBASE_TYPE result;

    if(timer->handle)
    {
        if ( lega_rtos_is_in_interrupt_context( ) == TRUE ) {
            signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            result = xTimerResetFromISR(timer->handle, &xHigherPriorityTaskWoken );
            portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
        } else
            result = xTimerReset( timer->handle, LEGA_WAIT_FOREVER );

        if ( result != pdPASS )
        {
            return kGeneralErr;
        }
    }

    return kNoErr;
}

/*******************************************************************
* Name:lega_rtos_deinit_timer
* Description:  delete timer
* input param: timer
* output param: none
* return:kNoErr or kGeneralErr
*******************************************************************/

OSStatus lega_rtos_deinit_timer( lega_timer_t* timer )
{
    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    if(timer->handle)
    {
        if (lega_rtos_is_timer_running(timer))
            lega_rtos_stop_timer(timer);

        if ( xTimerDelete( timer->handle, LEGA_WAIT_FOREVER ) != pdPASS )
        {
            printf("OS deinit timer fail");
            return kGeneralErr;
        }
        timer->handle = NULL;
    }
    lega_rtos_exit_critical();
    return kNoErr;
}

//////////////  event group functions /////////////////////////////////
/// it is reserved for future
OSStatus lega_rtos_init_event_flags( lega_event_flags_t* event_flags )
{
    *event_flags = xEventGroupCreate();

    return kNoErr;
}

OSStatus lega_rtos_wait_for_event_flags( lega_event_flags_t* event_flags,
                                         uint32_t flags_to_wait_for,
                                         uint32_t* flags_set,
                                         OSBool clear_set_flags,
                                         lega_event_flags_wait_option_t wait_option,
                                         uint32_t timeout_ms )
{

    *flags_set = xEventGroupWaitBits(*event_flags, /* The event group being tested. */
                                flags_to_wait_for, /* The bits within the event group to wait for. */
                                clear_set_flags,         /*  should be cleared before returning. */
                                wait_option,        /* Wait for all the bits to be set, not needed for single bit. */
                                timeout_ms); /* Wait for maximum duration for bit to be set. With 1 ms tick,
                                                portMAX_DELAY will result in wait of 50 days*/
    return kNoErr;
}

OSStatus lega_rtos_set_event_flags( lega_event_flags_t* event_flags, uint32_t flags_to_set )
{
    portBASE_TYPE taskToWake = pdFALSE;

    if (lega_rtos_is_in_interrupt_context( ) == TRUE)
    {
        if (xEventGroupSetBitsFromISR(*event_flags, flags_to_set, &taskToWake) == pdPASS)
        {
            /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
                    switch should be requested.  The macro used is port specific and will
                    be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
                    the documentation page for the port being used. */
            portYIELD_FROM_ISR(taskToWake);
        }
    }
    else
    {
        xEventGroupSetBits(*event_flags, flags_to_set);
    }

    return kNoErr;
}

OSStatus lega_rtos_deinit_event_flags( lega_event_flags_t* event_flags )
{
    vEventGroupDelete(*event_flags);
    *event_flags = NULL;
    return  kNoErr;
}

/*******************************************************************
* Name:lega_rtos_suspend_thread
* Description: suspend a task
* input param: thread
* output param: none
* return: none
*******************************************************************/
void lega_rtos_suspend_thread(lega_thread_t* thread)
{
    if (thread == NULL)
        vTaskSuspend(NULL);
    else
        vTaskSuspend(*thread);

}
/*******************************************************************
* Name: lega_rtos_resume_thread
* Description: resume a task which is suspended
* input param: thread
* output param: none
* return: none
*******************************************************************/
void lega_rtos_resume_thread(lega_thread_t* thread)
{
    if (thread == NULL)
        vTaskResume(NULL);
    else
        vTaskResume(*thread);

}

/*******************************************************************
* Name:lega_rtos_get_time
* Description: get freeRTOS tick count
* input param: none
* output param: none
* return: tick count
*******************************************************************/
uint32_t lega_rtos_get_time( void )
{
    return (lega_time_t) ( xTaskGetTickCount( ) * ms_to_tick_ratio );
}

uint32_t lega_rtos_get_system_ticks(void)
{
    return xTaskGetTickCount();
}

/*******************************************************************
* Name: lega_rtos_get_system_version
* Description: return system version
* input param: none
* output param: none
* return: system version
*******************************************************************/
const char *lega_rtos_get_system_version()
{
    return "freertos";
}

/*******************************************************************
* Name: lega_rtos_delay_milliseconds
* Description: do a freeRTOS delay for current task
* input param: num_ms (time)
* output param: none
* return: kNoErr
*******************************************************************/
OSStatus lega_rtos_delay_milliseconds( uint32_t num_ms )
{
    uint32_t ticks;

    ticks = num_ms / ms_to_tick_ratio;
    if (ticks == 0)
        ticks = 1;

    vTaskDelay( (portTickType) ticks );

    return kNoErr;
}

/*******************************************************************
* Name: vApplicationStackOverflowHook
* Description: this is a callback function as a hook for printing stack overflow task
* input param: pxTask, pcTaskName
* output param: none
* return: none
*******************************************************************/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
   // char buf[256];

   // vTaskList(buf);
    printf("ERROR: Stack OverFlow task is %s", pcTaskName);


}

/*******************************************************************
* Name:vApplicationMallocFailedHook
* Description: this is a hook function for malloc mem failure
* input param:
* output param:
* return:
*******************************************************************/
void vApplicationMallocFailedHook( void )
{
    printf("ERROR: it happened malloc failure \n");
}


void vApplicationTickHook(void)
{
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* The RTOS tick hook function is enabled by setting configUSE_TICK_HOOK to
    1 in FreeRTOSConfig.h.

    "Give" the semaphore on every 500th tick interrupt. */

    /* If xHigherPriorityTaskWoken is pdTRUE then a context switch should
    normally be performed before leaving the interrupt (because during the
    execution of the interrupt a task of equal or higher priority than the
    running task was unblocked).  The syntax required to context switch from
    an interrupt is port dependent, so check the documentation of the port you
    are using.

    In this case, the function is running in the context of the tick interrupt,
    which will automatically check for the higher priority task to run anyway,
    so no further action is required. */
}
/*-----------------------------------------------------------*/

 __attribute__((weak)) void vApplicationIdleHook(void)
{
    // volatile size_t xFreeStackSpace;
    /* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
    FreeRTOSConfig.h.

    This function is called on each cycle of the idle task.  In this case it
    does nothing useful, other than report the amount of FreeRTOS heap that
    remains unallocated. */
    /* By now, the kernel has allocated everything it is going to, so
    if there is a lot of heap remaining unallocated then
    the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
    reduced accordingly. */
}

/*******************************************************************
* Name: rtos_suppress_and_sleep
* Description: this function is to get system sleep down for sime time
* input param:
* output param:
* return:
*******************************************************************/
void rtos_suppress_and_sleep( unsigned long sleep_ms )
{

}

int32_t lega_malloc_cnt = 0;
/*******************************************************************
* Name:lega_rtos_malloc
* Description: malloc a buffer
* input param: buffer size
* output param: none
* return:
*******************************************************************/
#if LEGA_DEBUG_MALLOC_TRACE
void *_lega_rtos_malloc( uint32_t xWantedSize,const char* function, int line)
#else
void *lega_rtos_malloc( uint32_t xWantedSize )
#endif
{
#if LEGA_DEBUG_MALLOC_TRACE
    void *p= _pvPortMalloc(xWantedSize,function,line);
#else
    void *p= pvPortMalloc(xWantedSize);
#endif

    if(p)
    {
        lega_malloc_cnt++;
    }
    else
    {

#if LEGA_DEBUG_MALLOC_TRACE
        printf("OS malloc %d fail %s %d\n",(int)xWantedSize,function,line);
#else
        printf("OS malloc %d fail\n",(int)xWantedSize);
#endif

        return p;
    }

    while(xWantedSize)
    {
        if(xWantedSize>=4)
        {
           xWantedSize-=4;
           * (int *)(p+ xWantedSize)=0;
        }
        else
        {
           xWantedSize--;
          * (char *)(p+ xWantedSize)=0;   // clear memory alloced
        }
    }

    return p;

}

void *lega_rtos_realloc(void *ptr, uint32_t size)
{
    return pvPortRealloc( ptr,size );
}

/*******************************************************************
* Name:lega_rtos_free
* Description: free a buffer memory
* input param: pv, pointer to the buffer
* output param: none
* return:
*******************************************************************/
#if LEGA_DEBUG_MALLOC_TRACE
void _lega_rtos_free(void *pv,const char* function, int line)
#else
void lega_rtos_free( void *pv )
#endif
{
    if(pv)
    {
        lega_malloc_cnt--;
    }
    else
    {
#if LEGA_DEBUG_MALLOC_TRACE
        printf("OS memfree null %s %d\n",function,line);
#else
        printf("OS memfreer null\n");
#endif
    }

#if LEGA_DEBUG_MALLOC_TRACE
    _vPortFree(pv,function,line);
#else
    vPortFree(pv);
#endif
}

#if LEGA_DEBUG_MALLOC_TRACE
void _lega_rtos_dump_malloc()
{
    dump_used_malloc();
}
#endif

#pragma GCC push_options
#pragma GCC optimize("O2")
void lega_intrpt_enter()
{

}
void lega_intrpt_exit()
{

}
#pragma GCC pop_options

/*******************************************************************
* Name:lega_rtos_int_disable
* Description: disable interrupt
* input param: none
* output param: none
* return:none
*******************************************************************/
FLASH_LWIFI_SEG SPEED_OPTIMIZATION uint32_t lega_rtos_int_disable( void )
{
    if(!lega_rtos_is_in_interrupt_context())
    {
        vPortRaiseBASEPRI();
    }

    return 0;
}

/*******************************************************************
* Name:lega_rtos_int_enable
* Description: enable interrupt
* input param: int_mask, int_mask to restore
* output param:none
* return:none
*******************************************************************/
FLASH_LWIFI_SEG SPEED_OPTIMIZATION void lega_rtos_int_enable( uint32_t int_mask )
{
    if(!lega_rtos_is_in_interrupt_context())
    {
        vPortSetBASEPRI(0);
    }
}

#if( configUSE_TICKLESS_IDLE == 1 )
extern void vPortReSetupTimer( void );
#endif
FLASH_LWIFI_SEG SPEED_OPTIMIZATION void lega_rtos_systick_reconfig(void)
{
    SysTimer_Stop();
    SysTick_ReConfig(SYSTEM_CORE_CLOCK/configTICK_RATE_HZ);

#if( configUSE_TICKLESS_IDLE == 1 )
    vPortReSetupTimer();
#endif
    SysTimer_Start();
}

FLASH_LWIFI_SEG SPEED_OPTIMIZATION int lega_rtos_running(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        return 1;
    } else {
        return 0;
    }
}

/*******************************************************************
* Name:lega_rtos_get_threadinfo
* Description: get rtos thread info
* input param: thread, thread id;info, buffer to store infomation
* output param:none
* return:OSStatus
*******************************************************************/
FLASH_LWIFI_SEG SPEED_OPTIMIZATION OSStatus lega_rtos_get_threadinfo(lega_thread_t* thread, lega_threadinfo_t * info)
{
    TaskStatus_t xTaskDetails;
    TaskHandle_t xHandle = NULL;

    if (thread != NULL) {
        xHandle = *((TaskHandle_t *)thread);
    } else {
        xHandle = xTaskGetCurrentTaskHandle();
    }
    if((NULL == xHandle) || (NULL == info))
        return kGeneralErr;
    vTaskGetInfo( xHandle,
                  &xTaskDetails,
                  0, // Include the high water mark in xTaskDetails.
                  eInvalid ); // Include the task state in xTaskDetails.
    info->stackTop = (uint32_t)xTaskDetails.pxStackBase;
    info->stackBottom = (uint32_t)xTaskDetails.pxStackEnd;

    return kNoErr;
}

uint32_t lega_rtos_get_free_heap_size(void)
{
    return (uint32_t)xPortGetFreeHeapSize();
}

uint32_t lega_rtos_get_total_size(void)
{
    extern void *heap_len;
    return (uint32_t)&heap_len;
}

uint32_t lega_rtos_get_minimum_free_heap_size(void)
{
    return (uint32_t)xPortGetMinimumEverFreeHeapSize();
}

void lega_rtos_set_timeout(lega_timeout_t *timeout)
{
    vTaskSetTimeOutState((TimeOut_t * const)timeout);
}

OSBool lega_rtos_check_timeout(lega_timeout_t *timeout,lega_tick_t *wait_ticks)
{
    if(xTaskCheckForTimeOut((TimeOut_t * const)timeout,wait_ticks) == pdTRUE)
        return TRUE;
    else
        return FALSE;
}

uint64_t ms_to_tick(uint32_t timeout_ms)
{
    return (portTickType) ( timeout_ms / ms_to_tick_ratio );
}
/***************************  end   *************************************/
