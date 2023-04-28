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
#ifdef CFG_PLF_RV32
#include "asr_rv32.h"
#endif
#include "asr_port.h"
#include "asr_alto_boot.h"

void asr_cfg_boot_type(void)
{
    uint32_t flag1 = RETENTION_SRAM->BOOT_CFG;
    uint32_t flag2 = REG_RD(AON_RST_CHECK_REG);
    if((RET_RAM_SOFT_RST_FLAG == flag1) && (AON_RST_CHECK_FLAG == (flag2 & AON_RST_CHECK_FLAG)))
    {
        RETENTION_SRAM->BOOT_TYPE = SOFTWARE_RST;
    }
    else if((RET_RAM_DS_RST_FLAG == flag1) && (AON_RST_CHECK_FLAG == (flag2 & AON_RST_CHECK_FLAG)))
    {
        RETENTION_SRAM->BOOT_CFG = RET_RAM_SOFT_RST_FLAG;
        RETENTION_SRAM->BOOT_TYPE = DEEP_SLEEP_RST;
    }
    else if((RET_RAM_SOFT_RST_FLAG == flag1) && (0 == (flag2 & AON_RST_CHECK_FLAG)))
    {
        REG_WR(AON_RST_CHECK_REG, flag2 | AON_RST_CHECK_FLAG);
        RETENTION_SRAM->BOOT_TYPE = HARDWARE_PIN_RST;
    }
    else if(0 == (flag2 & AON_RST_CHECK_FLAG))
    {
        REG_WR(AON_RST_CHECK_REG, flag2 | AON_RST_CHECK_FLAG);
        RETENTION_SRAM->BOOT_CFG = RET_RAM_SOFT_RST_FLAG;
        RETENTION_SRAM->BOOT_TYPE = PWR_ON_RST;
    }
    else
    {
        REG_WR(AON_RST_CHECK_REG, flag2 | AON_RST_CHECK_FLAG);
        RETENTION_SRAM->BOOT_CFG = RET_RAM_SOFT_RST_FLAG;
        RETENTION_SRAM->BOOT_TYPE = UNKNOWN_RST;
    }
}

void asr_set_ds_boot_type(void)
{
    RETENTION_SRAM->BOOT_CFG = RET_RAM_DS_RST_FLAG;
}

uint32_t asr_get_boot_type(void)
{
    return (RETENTION_SRAM->BOOT_TYPE);
}

void asr_cfg_boot_reset_type(void)
{
   if(RETENTION_SRAM->BOOT_TYPE == 0)
   {
      RETENTION_SRAM->BOOT_TYPE_RESET = PWR_ON_RST;
      RETENTION_SRAM->BOOT_TYPE = PWR_ON_RST;
      RETENTION_SRAM->BOOT_CFG = RET_RAM_SOFT_RST_FLAG;
   }
   else if(RETENTION_SRAM->BOOT_TYPE == SOFTWARE_RST)
   {
     if(RETENTION_SRAM->BOOT_TYPE_SW_RESET == SOFTWARE_RST)
       RETENTION_SRAM->BOOT_TYPE_RESET = SOFTWARE_RST;
    else if(RETENTION_SRAM->BOOT_TYPE_SW_RESET == WDG_RST)
       RETENTION_SRAM->BOOT_TYPE_RESET = WDG_RST;
    else
        RETENTION_SRAM->BOOT_TYPE_RESET = UNKNOWN_RST;
   }
   else  if(RETENTION_SRAM->BOOT_TYPE == HARDWARE_PIN_RST)
      RETENTION_SRAM->BOOT_TYPE_RESET = HARDWARE_PIN_RST;
   else
      RETENTION_SRAM->BOOT_TYPE_RESET = 0;

   RETENTION_SRAM->BOOT_TYPE_SW_RESET = 0;
}

DEBUG_SECTION void asr_set_sw_reset_boot_type(uint32_t sw_type)
{
    RETENTION_SRAM->BOOT_TYPE_SW_RESET = sw_type;
}

DEBUG_SECTION uint32_t asr_get_sw_reset_boot_type()
{
    return (RETENTION_SRAM->BOOT_TYPE_SW_RESET);
}

uint32_t asr_sys_reset_type_get(void)
{
    uint32_t reason = 0;
    uint32_t status = REG_RD(AON_CPU_RESET_STATUS);

    if(RETENTION_SRAM->BOOT_TYPE_RESET != 0)
      reason = RETENTION_SRAM->BOOT_TYPE_RESET;
    else
      reason = RETENTION_SRAM->BOOT_TYPE;

    printf("reset status is 0x%08lx, reason is 0x%lx\n", status, reason);
    if (status == RBK_CPU_REQ_RESET)
    {
        REG_WR(AON_CPU_RESET_STATUS, RBK_CPU_REQ_RESET);
        return (SOFTWARE_RST);     /*sw reset*/
    }
    else if (status == RBK_WDG_REQ_RESET)
    {
        REG_WR(AON_CPU_RESET_STATUS, RBK_WDG_REQ_RESET);
        return (WDG_RST);          /*watch dog*/
    }
    else
        return reason;
}