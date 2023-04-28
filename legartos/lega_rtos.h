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

#ifndef __LEGARTOS_H__
#define __LEGARTOS_H__

//#include "co_int.h"
#include "stddef.h"
#include <stdio.h>
#include <stdbool.h>
#include "lega_rtos_api.h"

/** @addtogroup LEGA_Core_APIs
  * @{
  */

/** @defgroup LEGA_RTOS LEGA RTOS Operations
  * @brief Provide management APIs for Thread, Mutex, Timer, Semaphore and FIFO
  * @{
  */

#define FreeRTOS_VERSION_MAJOR 9



#define LEGA_HARDWARE_IO_WORKER_THREAD     ( (lega_worker_thread_t*)&lega_hardware_io_worker_thread)
#define LEGA_NETWORKING_WORKER_THREAD      ( (lega_worker_thread_t*)&lega_worker_thread )
#define LEGA_WORKER_THREAD                 ( (lega_worker_thread_t*)&lega_worker_thread )

#define LEGA_NETWORK_WORKER_PRIORITY      (3)
#define LEGA_DEFAULT_WORKER_PRIORITY      (5)
#define LEGA_DEFAULT_LIBRARY_PRIORITY     (5)
#define LEGA_APPLICATION_PRIORITY         (7)

#define kNanosecondsPerSecond   1000000000UUL
#define kMicrosecondsPerSecond  1000000UL
#define kMillisecondsPerSecond  1000

typedef struct
{
    lega_thread_t thread;
    lega_queue_t  event_queue;
} lega_worker_thread_t;

typedef struct
{
    event_handler_t        function;
    void*                  arg;
    lega_timer_t           timer;
    lega_worker_thread_t*  thread;
} lega_timed_event_t;

extern lega_worker_thread_t lega_hardware_io_worker_thread;
extern lega_worker_thread_t lega_worker_thread;

/* Legacy definitions */
#define lega_thread_sleep                 lega_rtos_thread_sleep
#define lega_thread_msleep                lega_rtos_thread_msleep



/** @defgroup LEGA_RTOS_Thread LEGA RTOS Thread Management Functions
 *  @brief Provide thread creation, delete, suspend, resume, and other RTOS management API
 *  @verbatim
 *   LEGA thread priority table
 *
 * +----------+-----------------+
 * | Priority |      Thread     |
 * |----------|-----------------|
 * |     0    |      LEGA       |   Highest priority
 * |     1    |     Network     |
 * |     2    |                 |
 * |     3    | Network worker  |
 * |     4    |                 |
 * |     5    | Default Library |
 * |          | Default worker  |
 * |     6    |                 |
 * |     7    |   Application   |
 * |     8    |                 |
 * |     9    |      Idle       |   Lowest priority
 * +----------+-----------------+
 *  @endverbatim
 * @{
 */

/** @brief   Creates a worker thread
 *
 * Creates a worker thread
 * A worker thread is a thread in whose context timed and asynchronous events
 * execute.
 *
 * @param worker_thread    : a pointer to the worker thread to be created
 * @param priority         : thread priority
 * @param stack_size       : thread's stack size in number of bytes
 * @param event_queue_size : number of events can be pushed into the queue
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred
 */
OSStatus lega_rtos_create_worker_thread( lega_worker_thread_t* worker_thread, uint8_t priority, uint32_t stack_size, uint32_t event_queue_size );


/** @brief   Deletes a worker thread
 *
 * @param worker_thread : a pointer to the worker thread to be created
 *
 * @return    kNoErr : on success.
 * @return    kGeneralErr   : if an error occurred
 */
OSStatus lega_rtos_delete_worker_thread( lega_worker_thread_t* worker_thread );


/** @brief    Suspend all other thread
  *
  * @param    none
  *
  * @return   none
  */
void lega_rtos_suspend_all_thread(void);


/** @brief    Rresume all other thread
  *
  * @param    none
  *
  * @return   none
  */
long lega_rtos_resume_all_thread(void);


/** @brief    Sleeps until another thread has terminated
  *
  * @Details  Causes the current thread to sleep until the specified other thread
  *           has terminated. If the processor is heavily loaded with higher priority
  *           tasks, this thread may not wake until significantly after the thread termination.
  *
  * @param    thread : the handle of the other thread which will terminate
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_thread_join( lega_thread_t* thread );


/** @brief    Suspend current thread for a specific time
  *
  * @param    seconds : A time interval (Unit: seconds)
  *
  * @return   None.
  */
void lega_rtos_thread_sleep(uint32_t seconds);

/** @brief    Suspend current thread for a specific time
 *
 * @param     milliseconds : A time interval (Unit: millisecond)
 *
 * @return    None.
 */
void lega_rtos_thread_msleep(uint32_t milliseconds);







/** @defgroup LEGA_RTOS_EVENT LEGA RTOS Event Functions
  * @{
  */

/**
  * @brief    Sends an asynchronous event to the associated worker thread
  *
  * @param worker_thread :the worker thread in which context the callback should execute from
  * @param function      : the callback function to be called from the worker thread
  * @param arg           : the argument to be passed to the callback function
  *
  * @return    kNoErr        : on success.
  * @return    kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_send_asynchronous_event( lega_worker_thread_t* worker_thread, event_handler_t function, void* arg );

/** Requests a function be called at a regular interval
 *
 * This function registers a function that will be called at a regular
 * interval. Since this is based on the RTOS time-slice scheduling, the
 * accuracy is not high, and is affected by processor load.
 *
 * @param event_object  : pointer to a event handle which will be initialised
 * @param worker_thread : pointer to the worker thread in whose context the
 *                        callback function runs on
 * @param function      : the callback function that is to be called regularly
 * @param time_ms       : the time period between function calls in milliseconds
 * @param arg           : an argument that will be supplied to the function when
 *                        it is called
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred
 */
OSStatus lega_rtos_register_timed_event( lega_timed_event_t* event_object, lega_worker_thread_t* worker_thread, event_handler_t function, uint32_t time_ms, void* arg );


/** Removes a request for a regular function execution
 *
 * This function de-registers a function that has previously been set-up
 * with @ref lega_rtos_register_timed_event.
 *
 * @param event_object : the event handle used with @ref lega_rtos_register_timed_event
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred
 */
OSStatus lega_rtos_deregister_timed_event( lega_timed_event_t* event_object );


/**
  * @}
  */

















/** @brief    Initialize an endpoint for a RTOS event, a file descriptor
  *           will be created, can be used for select
  *
  * @param    event_handle : lega_semaphore_t, lega_mutex_t or lega_queue_t
  *
  * @retval   On success, a file descriptor for RTOS event is returned.
  *           On error, -1 is returned.
  */
int lega_rtos_init_event_fd(lega_event_t event_handle);

/** @brief    De-initialise an endpoint created from a RTOS event
  *
  * @param    fd : file descriptor for RTOS event
  *
  * @retval   0 for success. On error, -1 is returned.
  */
int lega_rtos_deinit_event_fd(int fd);


/**
  * @}
  */

#endif

/**
  * @}
  */

/**
  * @}
  */

