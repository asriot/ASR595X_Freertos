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
#ifndef __ASR_I2C_H__
#define __ASR_I2C_H__

#include <errno.h>
#include "asr_port.h"
#include "asr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    FUGUE I2C BEGIN
*/

typedef struct
{
    __IO uint32_t  CR; //0x00
    __IO uint32_t  SR;
    __IO uint32_t  SAR; //0x08
    __IO uint32_t  DBR;
    __IO uint32_t  LCR;
    __IO uint32_t  WCR;  //0x14
    __IO uint32_t  RST_CYCL; //0x18
    __I  uint32_t  BMR;  //0x1c
    __IO uint32_t  WFIFO; //0x20
    __IO uint32_t  WFIFO_WPTR; //0x24
    __IO uint32_t  WFIFO_RPTR; //0x28
    __IO uint32_t  RFIFO; //0x2c
    __IO uint32_t  RFIFO_WPTR;
    __IO uint32_t  RFIFO_RPTR;  //0x34
    __IO uint32_t  RESV[2];       //0x38 0x3C
    __I uint32_t   WFIFO_STATUS;  //0x40
    __I uint32_t   RFIFO_STATUS;  //0x44
} I2C_TypeDef;

#define I2C0                       ((I2C_TypeDef *)I2C0_BASE)
#define I2C1                       ((I2C_TypeDef *)I2C1_BASE)

/*
    FUGUE I2C END
*/

#define I2C_SCL_STATUS_BIT  (1)
#define I2C_SDA_STATUS_BIT  (0)

// TWSI_NEW
#define I2C_INTERRUPT_RX_FIFO_OVERRUN_EN   (1 << 31)
#define I2C_INTERRUPT_RX_FIFO_FULL_EN      (1 << 30)
#define I2C_INTERRUPT_TX_FIFO_EMPTY_EN     (1 << 28)
#define I2C_INTERRUPT_RX_FIFO_HALF_FULL_EN (1 << 29)
#define I2C_INTERRUPT_TRANS_DONE_EN        (1 << 27)
#define I2C_INTERRUPT_SLAVE_ADDR_DET_EN    (1 << 23)
#define I2C_INTERRUPT_ARBT_LOSS_DET_EN     (1 << 18)
#define I2C_INTERRUPT_MASTER_STOP_DET_EN   (1 << 25)
#define I2C_INTERRUPT_SLAVE_STOP_DET_EN    (1 << 24)
#define I2C_INTERRUPT_BUS_ERROR_DET_EN     (1 << 22)
#define I2C_INTERRUPT_RX_BUFER_FULL_EN     (1 << 20)
#define I2C_INTERRUPT_TX_BUFFER_EMPTY_EN   (1 << 19)

#define I2C_STATUS_RX_FIFO_OVERRUN      (1 << 31)
#define I2C_STATUS_RX_FIFO_FULL         (1 << 30)
#define I2C_STATUS_TX_FIFO_EMPTY        (1 << 28)
#define I2C_STATUS_TX_FIFO_FULL         (0xffff)
#define I2C_STATUS_RX_FIFO_HALF_FULL    (1 << 29)
#define I2C_STATUS_TRANS_DONE           (1 << 27)
#define I2C_STATUS_MASTER_STOP_DET      (1 << 26)
#define I2C_STATUS_SLAVE_STOP_DET       (1 << 24)
#define I2C_STATUS_SLAVE_ADDR_DET       (1 << 23)
#define I2C_STATUS_BUS_ERROR_DET        (1 << 22)
#define I2C_STATUS_RX_BUFER_FULL        (1 << 20)
#define I2C_STATUS_TX_BUFFER_EMPTY      (1 << 19)
#define I2C_STATUS_UNIT_BUSY            (1 << 15)
#define I2C_STATUS_BUS_BUSY             (1 << 16)
#define I2C_STATUS_ACK_NACK             (1 << 14)
#define I2C_STATUS_RW_MODE              (1 << 13)

#define I2C_TRANS_BEGIN                 (1 << 4)
#define i2c_mst_fifo_mode_EN                (1 << 5)
#define i2c_mst_fifo_mode_DIS               (0)

#define I2C_MODE_STANDARD           (0)
#define I2C_MODE_FAST               (1)
#define I2C_MODE_HIGH_SPEED_0       (2)    // not supported in asr
#define I2C_MODE_HIGH_SPEED_1       (3)    // not supported in asr
#define I2C_MODE_SET_POS            (8)
#define I2C_MODE_SET_MASK           (3 << I2C_MODE_SET_POS)
#define I2C_HS_MASTER_CODE          (0x0A)

#define I2C_UNIT_RESET              (1 << 10)
#define I2C_DMA_ENABLE              (1 << 7)
#define I2C_UNIT_ENABLE             (1 << 14)
#define I2C_SCL_ENABLE              (1 << 13)
#define I2C_DMA_DISABLE             (0)

/* fifo mode control */
#define I2C_TB                      (1 << 11)
#define I2C_SEND_NACK               (1 << 10)
#define I2C_SEND_STOP               (1 << 9)
#define I2C_SEND_START              (1 << 8)
#define I2C_MASTER_ABORT            (1 << 12)

/* non fifo mode control */
#define I2C_CR_TB                   (I2C_TB >> 8)
#define I2C_CR_SEND_NACK            (I2C_SEND_NACK >> 8)
#define I2C_CR_SEND_STOP            (I2C_SEND_STOP >> 8)
#define I2C_CR_SEND_START           (I2C_SEND_START >> 8)

#define I2C_MODE   I2C_MASTER//I2C_SLAVE
#define I2C_MASTER                  (0)
#define I2C_SLAVE                   (1)
#define I2C_SLAVE_ADDR              0x55

#define I2C_READ                    (1)
#define I2C_WRITE                   (0)

#define I2C_TX_FIFO_DEPTH           (8)
#define I2C_RX_FIFO_DEPTH           (16)

#define I2C_MEM_ADDR_SIZE_8         (0)
#define I2C_MEM_ADDR_SIZE_16        (1)
#define I2C_MEM_ADDR_SIZE_32        (3)

#define DUMMY_BYTE                  (0x5a)
#define MAX_RX_SIZE                 (100)
#define TIME_OUT                    (1000)
#define I2C_WAIT_FOREVER            (0xffffffff)

#define I2C_CLK                     (system_peri_clk)//(52000000)
#define I2C_STANDARD_SPEED          (100000)
#define I2C_FAST_SPEED              (400000)
#define I2C_HIGH_SPEED              (1700000)

// error number
#define I2C_SUCCESS                 0 // success
#define EI2CNUMERR                  1 // I2C port number error
#define ETIMEOUT                    2 // I2C timeout
#define EBUSERR                     3 // I2C bus error


typedef struct
{
    uint32_t i2c_speed_mode;
    uint32_t i2c_role;
    uint32_t i2c_mst_fifo_mode;
    uint32_t i2c_slave_addr;
    uint32_t i2c_dma_function;
} asr_i2c_config_t;


typedef struct
{
    uint8_t      port;    /* i2c port */
    asr_i2c_config_t config;  /* i2c config */
    void        *priv;    /* priv data */
} asr_i2c_dev_t;

__STATIC_INLINE void asr_i2c_write_byte_cmd(I2C_TypeDef * I2Cx, uint8_t data)
{
    I2Cx->WFIFO = data | I2C_WRITE | I2C_TB;
}

__STATIC_INLINE void asr_i2c_read_byte_cmd(I2C_TypeDef * I2Cx)
{
    I2Cx->WFIFO = I2C_TB;
}

/* read one byte from fifo or buffer register */
__STATIC_INLINE uint8_t asr_i2c_receive_byte(I2C_TypeDef * I2Cx)
{
    if(I2Cx->CR & i2c_mst_fifo_mode_EN)
        return I2Cx->RFIFO;
    else
        return I2Cx->DBR;
}

/* write one byte to fifo or buffer reigster */
__STATIC_INLINE void asr_i2c_write_byte(I2C_TypeDef * I2Cx, uint16_t data)
{
    // data = data_to_write | any_conrol_bit
    if(I2Cx->CR & i2c_mst_fifo_mode_EN)
        I2Cx->WFIFO = data;
    else
        I2Cx->DBR = data;
}

__STATIC_INLINE void asr_i2c_clear_interrupt(I2C_TypeDef * I2Cx, uint32_t I2C_INTR)
{
    I2Cx->SR |= I2C_INTR;
}

/* I2C needs to set TB for transmitting and receiving a byte
   this function is mainly for when I2C is used as a slave
*/
__STATIC_INLINE void i2c_set_tb(I2C_TypeDef * I2Cx)
{
    I2Cx->CR |= I2C_CR_TB;
}

int8_t asr_i2c_unit_reset( I2C_TypeDef * I2Cx);

ITstatus asr_i2c_get_flag_status(I2C_TypeDef * I2Cx, uint32_t I2C_flag);

void asr_i2c_config_struct_init(asr_i2c_dev_t * i2c);
int8_t asr_i2c_init(asr_i2c_dev_t * i2c);
int8_t asr_i2c_master_write_data(asr_i2c_dev_t * i2c, uint8_t slave_addr, uint8_t* pwdata, uint32_t wlen);
int8_t asr_i2c_master_read_data(asr_i2c_dev_t * i2c, uint8_t slave_addr,uint8_t *data, uint32_t size);
int8_t asr_i2c_master_repeated_write_read(I2C_TypeDef * I2Cx, uint8_t slave_addr, const uint8_t * pwdata,uint8_t * rdata,uint32_t wlen, uint32_t rlen);
int32_t asr_i2c_finalize(asr_i2c_dev_t *i2c);

#ifdef __cplusplus
}
#endif

#endif /* __ASR_I2C_H */
