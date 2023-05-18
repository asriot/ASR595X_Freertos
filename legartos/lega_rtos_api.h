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

 #ifndef _LEGA_RTOS_API_H_
 #define _LEGA_RTOS_API_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define     RTOS_HIGHEST_PRIORITY       (configMAX_PRIORITIES)
typedef enum
{
    WAIT_FOR_ANY_EVENT,
    WAIT_FOR_ALL_EVENTS,
} lega_event_flags_wait_option_t;


#define LEGA_DEBUG_MALLOC_TRACE     0 //customer should not change
#define LEGA_NEVER_TIMEOUT   (0xFFFFFFFF)
#define LEGA_WAIT_FOREVER    (0xFFFFFFFF)
#define LEGA_NO_WAIT         (0)

#define LEGA_SEMAPHORE_COUNT_MAX (0xFFFF)

#define LEGA_TASK_CONFIG_MAX           32
#define LEGA_TASK_CONFIG_WPA3_AUTH      0
#define LEGA_TASK_CONFIG_LWIFI          1
#define LEGA_TASK_CONFIG_UWIFI_RX       2
#define LEGA_TASK_CONFIG_UWIFI          3
#define LEGA_TASK_CONFIG_DHCPS          4
#define LEGA_TASK_CONFIG_BLE            5
#define LEGA_TASK_CONFIG_CHIP           6

#define     LEGA_WPA3_AUTH_TASK_PRIORITY             (28)
#define     LEGA_LWIFI_TASK_PRIORITY                 (27)
#define     LEGA_UWIFI_RX_TASK_PRIORITY              (28)
#define     LEGA_UWIFI_TASK_PRIORITY                 (26)
#define     LEGA_LWIP_DHCP_TASK_PRIORITY             (29)
#define     LEGA_BLE_SCHEDULER_PRIORITY              (29)
#define     LEGA_CHIP_TASK_PRIORITY                  (2)


#ifdef ALIOS_SUPPORT
#define CONFIG_KV_BUFFER_SIZE KV_CONFIG_TOTAL_SIZE
#elif defined(HARMONYOS_SUPPORT)
#define CONFIG_KV_BUFFER_SIZE 0x8000
#else
#define CONFIG_KV_BUFFER_SIZE 0
#endif //ALIOS_SUPPORT

#define lega_cpsr_t uint32_t

typedef enum
{
    kNoErr=0,
    kGeneralErr,
    kTimeoutErr,

}OSStatus;

typedef enum
{
  FALSE=0,
  TRUE=1,
}OSBool;

typedef void * lega_event_flags_t;
typedef void * lega_semaphore_t;
typedef void * lega_mutex_t;
typedef void * lega_thread_t;
typedef void  (*native_thread_t) (void *) ;

typedef void * lega_queue_t;
typedef void * lega_event_t;// LEGA OS event: lega_semaphore_t, lega_mutex_t or lega_queue_t
typedef void (*timer_handler_t)( void* arg );
typedef OSStatus (*event_handler_t)( void* arg );

//typedef void* xTimerHandle;
typedef uint32_t lega_time_t;
//typedef void* xTaskHandle;

typedef uint64_t lega_tick_t;
typedef void * lega_timeout_t;

typedef struct
{
    void *          handle;
    timer_handler_t function;
    void *          arg;
}lega_timer_t;


typedef struct
{
    uint8_t        task_priority;
    uint32_t       stack_size;
} lega_task_config_t;

typedef struct {
    uint32_t stackTop;
    uint32_t stackBottom;
    // add more info here
} lega_threadinfo_t;

typedef uint32_t lega_thread_arg_t;
typedef void (*lega_thread_function_t)( lega_thread_arg_t arg );



/** @defgroup LEGA_RTOS LEGA RTOS Operations
  * @{
  */
#define lega_rtos_declare_critical() lega_cpsr_t critical_cpsr
//void lega_rtos_declare_critical(void);

/** @brief Enter a critical session, all interrupts are disabled
  *
  * @return    none
  */
lega_cpsr_t _lega_rtos_enter_critical( void );
#define lega_rtos_enter_critical()           \
    do {                                     \
        critical_cpsr = _lega_rtos_enter_critical();  \
    } while (0)
/** @brief Exit a critical session, all interrupts are enabled
  *
  * @return    none
  */
//void lega_rtos_exit_critical( void );
void _lega_rtos_exit_critical( lega_cpsr_t cpsr_store);
#define lega_rtos_exit_critical()        \
    do {                                 \
         _lega_rtos_exit_critical(critical_cpsr); \
    } while (0)
/**
  * @}
  */


/** @defgroup LEGA_RTOS_Thread LEGA RTOS Thread Management Functions
 *  @brief Provide thread creation, delete, suspend, resume, and other RTOS management API
  * @{
 */

/** @brief Creates and starts a new thread
  *
  * @param thread     : Pointer to variable that will receive the thread handle (can be null)
  * @param priority   : A priority number.
  * @param name       : a text name for the thread (can be null)
  * @param function   : the main thread function
  * @param stack_size : stack size for this thread
  * @param arg        : argument which will be passed to thread function
  *
  * @return    kNoErr          : on success.
  * @return    kGeneralErr     : if an error occurred
  */
OSStatus lega_rtos_create_thread( lega_thread_t* thread, uint8_t priority, const char* name, lega_thread_function_t function, uint32_t stack_size, lega_thread_arg_t arg );

/** @brief   Deletes a terminated thread
  *
  * @param   thread     : the handle of the thread to delete, , NULL is the current thread
  *
  * @return  kNoErr        : on success.
  * @return  kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_delete_thread( lega_thread_t* thread );

/** @brief    Suspend a thread
  *
  * @param    thread     : the handle of the thread to suspend, NULL is the current thread
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
void lega_rtos_suspend_thread(lega_thread_t* thread);

/** @brief    Checks if a thread is the current thread
  *
  * @Details  Checks if a specified thread is the currently running thread
  *
  * @param    thread : the handle of the other thread against which the current thread
  *                    will be compared
  *
  * @return   true   : specified thread is the current thread
  * @return   false  : specified thread is not currently running
  */
OSBool lega_rtos_is_current_thread( lega_thread_t* thread );

/** @brief    Get current thread handler
  *
  * @return   Current LEGA RTOS thread handler
  */
lega_thread_t* lega_rtos_get_current_thread( void );

/** @brief    Suspend current thread for a specific time, at lease delay 1 tick
 *
 * @param     num_ms : A time interval (Unit: millisecond)
 *
 * @return    kNoErr.
 */
OSStatus lega_rtos_delay_milliseconds( uint32_t num_ms );

/** @brief    Forcibly wakes another thread
  *
  * @Details  Causes the specified thread to wake from suspension. This will usually
  *           cause an error or timeout in that thread, since the task it was waiting on
  *           is not complete.
  *
  * @param    thread : the handle of the other thread which will be woken
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_thread_force_awake( lega_thread_t* thread );

/** @brief    Print Thread status into buffer
  *
  * @param    buffer, point to buffer to store thread status
  * @param    length, length of the buffer
  *
  * @return   none
  */
OSStatus lega_rtos_print_thread_status( char* buffer, int length );
/**
  * @}
  */

  /** @defgroup LEGA_RTOS_SEM LEGA RTOS Semaphore Functions
  * @brief Provide management APIs for semaphore such as init,set,get and dinit.
  * @{
  */

/** @brief    Initialises a counting semaphore and set count to 0
  *
  * @param    semaphore : a pointer to the semaphore handle to be initialised
  * @param    count     : the init value of this semaphore
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_init_semaphore( lega_semaphore_t* semaphore, int value );

/** @brief    Set (post/put/increment) a semaphore
  *
  * @param    semaphore : a pointer to the semaphore handle to be set
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_set_semaphore( lega_semaphore_t* semaphore );

/** @brief    Get (wait/decrement) a semaphore
  *
  * @Details  Attempts to get (wait/decrement) a semaphore. If semaphore is at zero already,
  *           then the calling thread will be suspended until another thread sets the
  *           semaphore with @ref lega_rtos_set_semaphore
  *
  * @param    semaphore : a pointer to the semaphore handle
  * @param    timeout_ms: the number of milliseconds to wait before returning
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_get_semaphore( lega_semaphore_t* semaphore, uint32_t timeout_ms );

/** @brief    De-initialise a semaphore
  *
  * @Details  Deletes a semaphore created with @ref lega_rtos_init_semaphore
  *
  * @param    semaphore : a pointer to the semaphore handle
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_deinit_semaphore( lega_semaphore_t* semaphore );

/** @brief    if the task num pending by this semaphore is 0
  *
  * @Details  if the task num pending by this semaphore is 0
  *
  * @param    semaphore : a pointer to the semaphore handle
  *
  * @return   TRUE    : pending task num is 0
  * @return   FALSE   : pending task num is not 0
  */
OSBool lega_rtos_semaphore_pending_task_null( lega_semaphore_t* semaphore );

/** @defgroup LEGA_RTOS_MUTEX LEGA RTOS Mutex Functions
  * @brief Provide management APIs for Mutex such as init,lock,unlock and dinit.
  * @{
  */

/** @brief    Initialises a mutex
  *
  * @Details  A mutex is different to a semaphore in that a thread that already holds
  *           the lock on the mutex can request the lock again (nested) without causing
  *           it to be suspended.
  *
  * @param    mutex : a pointer to the mutex handle to be initialised
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_init_mutex( lega_mutex_t* mutex );

/** @brief    Obtains the lock on a mutex
  *
  * @Details  Attempts to obtain the lock on a mutex. If the lock is already held
  *           by another thead, the calling thread will be suspended until the mutex
  *           lock is released by the other thread.
  *
  * @param    mutex : a pointer to the mutex handle to be locked
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_lock_mutex( lega_mutex_t* mutex, uint32_t timeout_ms );

/** @brief    Releases the lock on a mutex
  *
  * @Details  Releases a currently held lock on a mutex. If another thread
  *           is waiting on the mutex lock, then it will be resumed.
  *
  * @param    mutex : a pointer to the mutex handle to be unlocked
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_unlock_mutex( lega_mutex_t* mutex );

/** @brief    De-initialise a mutex
  *
  * @Details  Deletes a mutex created with @ref lega_rtos_init_mutex
  *
  * @param    mutex : a pointer to the mutex handle
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_deinit_mutex( lega_mutex_t* mutex );

/**
  * @}
  */


/** @defgroup LEGA_RTOS_QUEUE LEGA RTOS FIFO Queue Functions
  * @brief Provide management APIs for FIFO such as init,push,pop and dinit.
  * @{
  */

/** @brief    Initialises a FIFO queue
  *
  * @param    queue : a pointer to the queue handle to be initialised
  * @param    name  : a text string name for the queue (NULL is allowed)
  * @param    message_size : size in bytes of objects that will be held in the queue
  * @param    number_of_messages : depth of the queue - i.e. max number of objects in the queue
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_init_queue( lega_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages );

/** @brief    Pushes an object onto a queue
  *
  * @param    queue : a pointer to the queue handle
  * @param    message : the object to be added to the queue. Size is assumed to be
  *                  the size specified in @ref lega_rtos_init_queue
  * @param    timeout_ms: the number of milliseconds to wait before returning
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error or timeout occurred
  */
OSStatus lega_rtos_push_to_queue( lega_queue_t* queue, void* message, uint32_t timeout_ms );

/** @brief    Pops an object off a queue
  *
  * @param    queue : a pointer to the queue handle
  * @param    message : pointer to a buffer that will receive the object being
  *                     popped off the queue. Size is assumed to be
  *                     the size specified in @ref lega_rtos_init_queue , hence
  *                     you must ensure the buffer is long enough or memory
  *                     corruption will result
  * @param    timeout_ms: the number of milliseconds to wait before returning
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error or timeout occurred
  */
OSStatus lega_rtos_pop_from_queue( lega_queue_t* queue, void* message, uint32_t timeout_ms );

/** @brief    De-initialise a queue created with @ref lega_rtos_init_queue
  *
  * @param    queue : a pointer to the queue handle
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_deinit_queue( lega_queue_t* queue );

/** @brief    Check if a queue is empty, only used in_interrupt_context
  *
  * @param    queue : a pointer to the queue handle
  *
  * @return   true  : queue is empty.
  * @return   false : queue is not empty.
  */
OSBool lega_rtos_is_queue_empty( lega_queue_t* queue );

/** @brief    Check if a queue is full, only used in_interrupt_context
  *
  * @param    queue : a pointer to the queue handle
  *
  * @return   true  : queue is empty.
  * @return   false : queue is not empty.
  */
OSBool lega_rtos_is_queue_full( lega_queue_t* queue );
/**
  * @}
  */


/** @defgroup LEGA_RTOS_TIMER LEGA RTOS Timer Functions
  * @brief Provide management APIs for timer such as init,start,stop,reload and dinit.
  * @{
  */

/**
  * @brief    Gets time in miiliseconds since RTOS start
  *
  * @note:    Since this is only 32 bits, it will roll over every 49 days, 17 hours.
  *
  * @returns  Time in milliseconds since RTOS started.
  */
uint32_t lega_rtos_get_time(void);

uint32_t lega_rtos_get_system_ticks(void);
/**
  * @brief    Return system version
  *
  * @returns  system version
  */
const char *lega_rtos_get_system_version();

/**
  * @brief     Initialize a RTOS timer
  *
  * @note      Timer does not start running until @ref lega_start_timer is called
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @param     time_ms  : Timer period in milliseconds
  * @param     function : the callback handler function that is called each time the
  *                       timer expires
  * @param     arg      : an argument that will be passed to the callback function
  *
  * @return    kNoErr        : on success.
  * @return    kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_init_timer( lega_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg );

/** @brief    Starts a RTOS timer running
  *
  * @note     Timer must have been previously initialised with @ref lega_rtos_init_timer
  *
  * @param    timer    : a pointer to the timer handle to start
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_start_timer( lega_timer_t* timer );

/** @brief    Stops a running RTOS timer
  *
  * @note     Timer must have been previously started with @ref lega_rtos_init_timer
  *
  * @param    timer    : a pointer to the timer handle to stop
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_stop_timer( lega_timer_t* timer );

/** @brief    Reloads a RTOS timer that has expired
  *
  * @note     This is usually called in the timer callback handler, to
  *           reschedule the timer for the next period.
  *
  * @param    timer    : a pointer to the timer handle to reload
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_reload_timer( lega_timer_t* timer );

/** @brief    De-initialise a RTOS timer
  *
  * @note     Deletes a RTOS timer created with @ref lega_rtos_init_timer
  *
  * @param    timer : a pointer to the RTOS timer handle
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus lega_rtos_deinit_timer( lega_timer_t* timer );

/** @brief    Check if an RTOS timer is running
  *
  * @param    timer : a pointer to the RTOS timer handle
  *
  * @return   true        : if running.
  * @return   false       : if not running
  */
OSBool lega_rtos_is_timer_running( lega_timer_t* timer );

/**
  * @}
  */
OSStatus lega_rtos_init_event_flags( lega_event_flags_t* event_flags );

OSStatus lega_rtos_wait_for_event_flags( lega_event_flags_t* event_flags,
                                         uint32_t flags_to_wait_for,
                                         uint32_t* flags_set,
                                         OSBool clear_set_flags,
                                         lega_event_flags_wait_option_t wait_option,
                                         uint32_t timeout_ms );

OSStatus lega_rtos_set_event_flags( lega_event_flags_t* event_flags, uint32_t flags_to_set );

OSStatus lega_rtos_deinit_event_flags( lega_event_flags_t* event_flags );

  /** @brief    judge whether in interrupt context or not
  *
  * @param
  *
  * @retval   true: in interrupt context. false: not in interrupt context.
  */
OSBool lega_rtos_is_in_interrupt_context(void);
OSStatus lega_rtos_task_cfg_get(uint32_t index, lega_task_config_t *cfg);
OSStatus lega_rtos_get_chip_task_cfg(lega_task_config_t *cfg);

#if !LEGA_DEBUG_MALLOC_TRACE
/**
 * @ malloc and free
 */
void *lega_rtos_malloc( uint32_t xWantedSize );
void lega_rtos_free( void *pv );
void *lega_rtos_realloc(void *ptr, uint32_t size);
#else
/**
 * @ used to debug memory leak
 */
#undef lega_rtos_malloc
#define lega_rtos_malloc(s) _lega_rtos_malloc((s),__FUNCTION__,__LINE__)
#undef lega_rtos_free
#define lega_rtos_free(v) _lega_rtos_free((v),__FUNCTION__,__LINE__)
#define lega_rtos_dump_malloc _lega_rtos_dump_malloc
void _lega_rtos_dump_malloc();
void _lega_rtos_free(void *pv,const char* function, int line);
void *_lega_rtos_malloc( uint32_t xWantedSize,const char* function, int line);
void *lega_rtos_realloc(void *ptr, uint32_t size);
#endif

void lega_intrpt_enter();
void lega_intrpt_exit();
uint32_t lega_rtos_int_disable( void );
void lega_rtos_int_enable( uint32_t int_mask );
void lega_rtos_systick_reconfig(void);
int lega_rtos_running(void);
OSStatus lega_rtos_get_threadinfo(lega_thread_t* thread, lega_threadinfo_t * info);

uint32_t lega_rtos_get_free_heap_size(void);
uint32_t lega_rtos_get_total_size(void);
uint32_t lega_rtos_get_minimum_free_heap_size(void);

void lega_rtos_set_timeout(lega_timeout_t *timeout);
OSBool lega_rtos_check_timeout(lega_timeout_t *timeout,lega_tick_t *wait_ticks);
uint64_t ms_to_tick(uint32_t timeout_ms);
#ifdef __cplusplus
}
#endif

#endif
