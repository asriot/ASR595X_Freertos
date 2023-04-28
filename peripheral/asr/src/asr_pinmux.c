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
#include "asr_pinmux.h"

#include "asr_port.h"

Pad_Config_State asr_pinmux_config(Pad_Num_Type pad_num, Pad_Func_Type pad_func)
{
    uint32_t pad_reg_addr = PINMUX_REG_BASE + 4*(pad_num>>3); // pinmux register address of pad pad_num
    uint32_t reg_offset = (pad_num%8)*4;  // offset from pad_reg_addr in bits
    int pad_func_value = asr_get_pad_func_val(pad_num,pad_func);

    if(pad_func_value<0) return Config_Fail;
    *(volatile uint32_t *)(pad_reg_addr) &= ~(0xf<<reg_offset); // mask
    *(volatile uint32_t *)(pad_reg_addr) |= (pad_func_value<<reg_offset); // set pinmux value

    return Config_Success;
}

void asr_pad_config(Pad_Num_Type pad_num, Pad_Pull_Type pull_type)
{
    port_pad_config(pad_num, pull_type);
}
