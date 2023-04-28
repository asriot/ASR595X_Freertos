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
#include <sys/stat.h>
#include <reent.h>
#ifdef CFG_PLF_RV32
#include "asr_rv32.h"
#endif
#include "printf_uart.h"
#include <errno.h>
#include "stdbool.h"
#include <sys/time.h>
#ifdef LEGA_RTOS_SUPPORT
#include "lega_rtos_api.h"

#ifndef ALIOS_SUPPORT
int __errno;
#endif

void * __dso_handle = 0;

void *_malloc_r(struct _reent *ptr, size_t size)
{
    void *mem;

    mem = lega_rtos_malloc(size);

    return mem;
}

void *_realloc_r(struct _reent *ptr, void *old, size_t newlen)
{
    return 0;
}

void *_calloc_r(struct _reent *ptr, size_t size, size_t len)
{
    void *mem;

    mem = lega_rtos_malloc(size * len);

    return mem;
}

void _free_r(struct _reent *ptr, void *addr)
{
    if(addr == NULL)
    {
        return;
    }

    lega_rtos_free(addr);
}
#endif
/////////////////////////////////////
#ifndef ALIOS_SUPPORT
_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t nbytes)
{
    const char *tmp = buf;
    int         i   = 0;

    if (buf == NULL)
    {
        return 0;
    }

    if ((fd == 1) || (fd == 2))
    {
        for (i = 0; i < nbytes; i++)
        {
            uart_put_char(*tmp);
            tmp++;
        }
        return nbytes;
    }
    else
    {
        return -1;
    }
}
#endif
 ///////////////////////////////////
 int _isatty (int fd)
 {
     return 1;
 }
 /////////////////////////////////
 int _lseek(int i, int k, int j)
 {
      return -1;
 }
 ////////////////////////////////
 int _read (int fd, char *pBuffer, int size)
 {
     return size;
 }
 ////////////////////////////////////
 int _fstat (int fd, struct stat *pStat)
 {
      pStat->st_mode = S_IFCHR;
      return 0;
 }
 ////////////////////////////////////
 int _close(int i)
 {
      return -1;
 }

caddr_t _sbrk(int increment)
{
    extern char end asm("itcm_heap_start");
    extern char pStack asm("itcm_heap_end");

    static char *s_pHeapEnd;

    if (!s_pHeapEnd)
        s_pHeapEnd = &end;

    if (s_pHeapEnd + increment > &pStack)
        return (caddr_t)-1;

    char *pOldHeapEnd = s_pHeapEnd;
    s_pHeapEnd += increment;
    return (caddr_t)pOldHeapEnd;
}


int _gettimeofday(struct timeval *tp, void *tzp)
{
    uint64_t cycles;

    cycles = __get_rv_cycle();

    tp->tv_sec = cycles / SystemCoreClock;
    tp->tv_usec = (cycles % SystemCoreClock) * 1000000 / SystemCoreClock;
    return 0;
}

#if __GNUC__ >= 10

int _kill(pid_t a, int b)
{
    return 0;
}

pid_t _getpid(void)
{
    return 0;
}

const char __locale_ctype_ptr[] = "ASR";

#endif //__GNUC__

bool __atomic_compare_exchange_1(volatile void * pulDestination, void * ulComparand, unsigned char desired, bool weak,
                                            int success_memorder, int failure_memorder)
{
    bool ulReturnValue;
    if (*(unsigned char *) pulDestination == *(unsigned char *) ulComparand)
    {
        *(unsigned char *) pulDestination = desired;
        ulReturnValue                     = true;
    }
    else
    {
        *(unsigned char *) ulComparand = *(unsigned char *) pulDestination;
        ulReturnValue                  = false;
    }
    return ulReturnValue;
}

#ifdef OS_NOT_SUPPORT
#include <stdlib.h>
uint8_t *lega_rtos_malloc(uint32_t xWantedSize)
{
    return malloc(xWantedSize);
}

void lega_rtos_free(void *pv)
{
    free(pv);
}

int lega_rtos_lock_mutex(void *mux,int timeout)
{
    return 0;
}

int lega_rtos_unlock_mutex(void *mux)
{
    return 0;
}

int lega_rtos_init_mutex(void *mux)
{
    return 0;
}

int lega_rtos_deinit_mutex(void *mux)
{
    return 0;
}
#endif //OS_NOT_SUPPORT
 /************end *************************/
