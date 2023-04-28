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
#include "asr_port.h"

int asr_get_pad_func_val(Pad_Num_Type pad_num,Pad_Func_Type pad_type)
{
    int pad_func_value=-1;
    switch(pad_type)
    {
        case PF_GPIO:
            switch(pad_num)
            {
                case PAD0:
                case PAD1:
                case PAD2:
                case PAD3: pad_func_value=0; break;
                case PAD4:
                case PAD5: pad_func_value=1; break;
                case PAD6:
                case PAD7:
                case PAD8:
                case PAD9:  pad_func_value=0; break;
                case PAD10: pad_func_value=2; break;
                case PAD11: pad_func_value=0; break;
                case PAD12:
                case PAD13: pad_func_value=1; break;
                case PAD14:
                case PAD15: pad_func_value=4; break;
                case PAD16:
                case PAD17:
                case PAD18:
                case PAD19:
                case PAD20:
                case PAD21:
                case PAD22:
                case PAD23: pad_func_value=0; break;
                default: break;
            }
            break;
        case PF_SWC:
            switch(pad_num)
            {
                case PAD0: pad_func_value=2;break;
                case PAD4: pad_func_value=0;break;
                default: break;
            }
            break;
        case PF_SWD:
            switch(pad_num)
            {
                case PAD1: pad_func_value=2;break;
                case PAD5: pad_func_value=0;break;
                default: break;
            }
            break;
        case PF_UART0:
            switch(pad_num)
            {
                case PAD0:
                case PAD1:
                case PAD16:
                case PAD17: pad_func_value=1; break;
                case PAD4:
                case PAD5:
                case PAD6:
                case PAD7:
                case PAD20:
                case PAD21: pad_func_value=3; break;
                default: break;
            }
            break;
        case PF_UART1:
            switch(pad_num)
            {
                case PAD2:
                case PAD3:
                case PAD18:
                case PAD19: pad_func_value=1; break;
                case PAD8:
                case PAD9: pad_func_value=4; break;
                case PAD14:
                case PAD15: pad_func_value=3; break;
                default: break;
            }
            break;
        case PF_UART2:
            switch(pad_num)
            {
                case PAD10:
                case PAD11:
                case PAD12:
                case PAD13:
                case PAD22:
                case PAD23: pad_func_value=3; break;
                default: break;
            }
            break;
        case PF_I2S:
            switch(pad_num)
            {
                case PAD5:
                case PAD7:
                case PAD8:
                case PAD9:
                case PAD10:
                case PAD11:
                case PAD12: pad_func_value=6; break;
                default: break;
            }
            break;
        case PF_PSRAM:
            switch(pad_num)
            {
                case PAD4:
                case PAD5:
                case PAD6:
                case PAD7:
                case PAD8:
                case PAD9: pad_func_value=7; break;
                case PAD16:
                case PAD17:
                case PAD18:
                case PAD19:
                case PAD20:
                case PAD21:
                case PAD22: pad_func_value=6; break;
                default: break;
            }
            break;
        case PF_SPI0:
            switch(pad_num)
            {
                case PAD6:
                case PAD7:
                case PAD8:
                case PAD9: pad_func_value=1; break;
                default: break;
            }
            break;
        case PF_SPI1:
            switch(pad_num)
            {
                case PAD0:
                case PAD1:
                case PAD2:
                case PAD3:
                case PAD16:
                case PAD17:
                case PAD18:
                case PAD19: pad_func_value=3; break;
                default: break;
            }
            break;
        case PF_SPI2:
            switch(pad_num)
            {
                case PAD10:
                case PAD11: pad_func_value=4; break;
                case PAD12:
                case PAD13: pad_func_value=7; break;
                case PAD14:
                case PAD15: pad_func_value=2; break;
                default: break;
            }
            break;
        case PF_PWM:
            switch(pad_num)
            {
                case PAD0:
                case PAD1:
                case PAD4:
                case PAD5:
                case PAD6:
                case PAD7: pad_func_value=4; break;

                case PAD10:
                case PAD11:
                case PAD14:
                case PAD15: pad_func_value=1; break;
                default: break;
            }
            break;
        case PF_I2C0:
            switch(pad_num)
            {
                case PAD2:
                case PAD3: pad_func_value=4; break;
                case PAD20:
                case PAD21: pad_func_value=1; break;
                default: break;
            }
            break;
        case PF_I2C1:
            switch(pad_num)
            {
                case PAD8:
                case PAD9: pad_func_value=3; break;
                case PAD22:
                case PAD23: pad_func_value=1; break;
                default: break;
            }
            break;

        default:    break;
    }
    return pad_func_value;
}

#ifdef ALIOS_SUPPORT
void asr_set_uart_pinmux(uint8_t uart_idx,uint8_t hw_flow_control)
{
    switch(uart_idx)
    {
        case 0:
            break;
        case 1:
            break;
        default:
            return;
    }
}
#endif
