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
#ifndef __ASR_PIN_MUX_H
#define __ASR_PIN_MUX_H

#include "asr_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    Config_Success,
    Config_Fail
}Pad_Config_State;

typedef enum
{
    PULL_DEFAULT = 0x00,
    PULL_UP,
    PULL_DOWN,
    PULL_NONE
}Pad_Pull_Type;

typedef enum
{
    DS1DS0_00 = 0x00,
    DS1DS0_01,
    DS1DS0_10,
    DS1DS0_11
}Pad_DS_Type;

Pad_Config_State asr_pinmux_config(Pad_Num_Type pad_num, Pad_Func_Type pad_func);
void asr_pad_config(Pad_Num_Type pad_num, Pad_Pull_Type pull_type);
void port_pad_config(Pad_Num_Type pad_num, Pad_Pull_Type pull_type);
void asr_pad_drive_strength(Pad_Num_Type pad_num, Pad_DS_Type pad_ds_type);

#ifdef __cplusplus
}
#endif

#endif