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
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include "asr_adc.h"
#include "asr_common.h"
#include "asr_rf_spi.h"
#include "asr_gpio.h"
#include "asr_port_peripheral.h"


#if defined ADC_ENABLE

void AUX_ADC_IRQHandler(void)
{
     AUX_ADC_IRQHandler_Dummy();
}

int32_t asr_adc_init(asr_adc_dev_t *adc_config)
{
    return port_adc_init(adc_config);
}

int32_t asr_adc_get(asr_adc_dev_t *adc_config)
{
    return port_adc_get(adc_config);
}
int32_t asr_tempr_get(asr_adc_dev_t *adc_config)
{
    return port_tempr_get(adc_config);
}

int32_t asr_adc_finalize(asr_adc_dev_t *adc_config)
{
    return port_adc_finalize(adc_config);
}

#endif
