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

#ifndef _ALTO_MATTER_H_
#define _ALTO_MATTER_H_
#include <stdint.h>
#define MATTER_CERT_DAC_PRIKEY_LEN    32

int32_t asr_factory_decrypt_dac_prvkey(uint8_t *pDacCipher, uint8_t ucDacKeyLen, uint8_t *pRdBuf, uint32_t *pOutLen);
#endif
