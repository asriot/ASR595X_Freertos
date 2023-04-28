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
#ifndef __ASR_RF_SPI_H__
#define __ASR_RF_SPI_H__

#include "asr_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_COMMAND             SPI_CTRL_REG_BASE
#define SPI_RDATA               SPI_CTRL_REG_BASE

void spi_mst_write(uint16_t addr, uint16_t data );
uint16_t spi_mst_read(uint16_t addr);
uint16_t spi_sw_protect_read(uint16_t addr);
void spi_sw_protect_write(uint16_t addr, uint16_t data);

void rf_set_reg_bit(uint16_t reg, uint8_t start_bit, uint8_t len, uint16_t src_val);
uint16_t rf_get_reg_bit(uint16_t reg, uint8_t start_bit, uint8_t len);

void spi_mst_write_low(uint16_t addr, uint16_t data );
uint16_t spi_mst_read_low(uint16_t addr);

#ifdef __cplusplus
}
#endif

#endif //__ASR_RF_SPI_H__
