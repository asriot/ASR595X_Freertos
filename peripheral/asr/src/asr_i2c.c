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

#include "asr_i2c.h"
#include "asr_port.h"
#include "asr_pinmux.h"

#include "asr_port_peripheral.h"

int8_t asr_i2c_unit_reset( I2C_TypeDef * I2Cx)
{
    I2Cx->CR &= I2C_UNIT_RESET; //clear rest of CR
    I2Cx->CR |= I2C_UNIT_RESET;  //set RESET bit
    I2Cx->SR = 0;
    I2Cx->CR &= ~I2C_UNIT_RESET; // clear RESET bit
    return 0;
}

void asr_i2c_config_struct_init(asr_i2c_dev_t * i2c)
{
    i2c->port = I2C0_INDEX;
    i2c->config.i2c_mst_fifo_mode = i2c_mst_fifo_mode_EN;
    i2c->config.i2c_role = I2C_MODE;                 // default role = master
    i2c->config.i2c_speed_mode = I2C_MODE_STANDARD;    // default speed = standard mode  I2C_MODE_FAST; //
    i2c->config.i2c_slave_addr = I2C_SLAVE_ADDR;       // default slave addr = 0x5a
    i2c->config.i2c_dma_function = I2C_DMA_DISABLE;
}

void asr_i2c_interrupt_config(I2C_TypeDef * I2Cx, uint32_t I2C_interrupt_en, int8_t new_state)
{
    if(new_state == ENABLE)
    {
        I2Cx->CR |= I2C_interrupt_en;
    }
    else
    {
        I2Cx->CR &= ~I2C_interrupt_en;
    }
}

ITstatus asr_i2c_get_flag_status(I2C_TypeDef * I2Cx, uint32_t I2C_flag)
{
    if(I2Cx->SR & I2C_flag)
        return SET;
    else
        return RESET;
}

static int8_t asr_i2c_check_flag(I2C_TypeDef * I2Cx, uint32_t flag)
{
    uint16_t timeout = 0;

    while( !asr_i2c_get_flag_status(I2Cx, flag) )
    {
        if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_ERROR_DET) )
            return EBUSERR; //bus error
        timeout++;
        if(timeout > 2000)
            return ETIMEOUT; // timeout
    }
    return 0;
}

/**
 * Initialises an I2C interface
 * Prepares an I2C hardware interface for communication as a master or slave
 *
 * @param[in]  i2c  the device for which the i2c port should be initialised
 *
 * @return  0 : on success, EIO : if an error occurred during initialisation
 */
int8_t asr_i2c_init(asr_i2c_dev_t * i2c)
{
    I2C_TypeDef * I2Cx;

    if(NULL == i2c)
      {
          return -EI2CNUMERR;
      }

      if(I2C0_INDEX == i2c->port) // I2C0_INDEX
      {
          I2Cx = I2C0;
      }
      else if(I2C1_INDEX == i2c->port) // I2C1_INDEX
      {
          I2Cx = I2C1;
      }
      else
      {
          return -EI2CNUMERR;
      }

    // I2C clock enable
    port_i2c_init_clk(i2c->port);

    /* reset unit */
    asr_i2c_unit_reset(I2Cx);

    if (i2c->config.i2c_role == I2C_MASTER)
    {
        I2Cx->SAR = i2c->config.i2c_slave_addr; // set unit address as slave

        I2Cx->CR &= ~I2C_MODE_SET_MASK; // reset speed mode to 0
        I2Cx->CR |= (i2c->config.i2c_speed_mode << I2C_MODE_SET_POS); // set speed mode
        I2Cx->LCR = 0;
        I2Cx->LCR = (((I2C_CLK/I2C_STANDARD_SPEED - 8)/2) | (((I2C_CLK/I2C_FAST_SPEED - 8)/2 - 1)<<9) | (((I2C_CLK/I2C_HIGH_SPEED - 9)/2)<<18) | (((I2C_CLK/I2C_HIGH_SPEED - 9)/2)<<27));  // set divider
        I2Cx->WCR = (((I2C_CLK/I2C_FAST_SPEED - 8)/2 - 1)/3);
        I2Cx->CR |= i2c->config.i2c_mst_fifo_mode;  // set FIFO mode

        I2Cx->CR |= I2C_UNIT_ENABLE | I2C_SCL_ENABLE; // scl driving enable & unit enable
        I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
        /*asr_i2c_interrupt_config(I2Cx, I2C_INTERRUPT_SLAVE_ADDR_DET_EN | I2C_INTERRUPT_TRANS_DONE_EN | I2C_INTERRUPT_RX_FIFO_FULL_EN | I2C_INTERRUPT_BUS_ERROR_DET_EN \
                                          | I2C_INTERRUPT_MASTER_STOP_DET_EN | I2C_INTERRUPT_RX_FIFO_FULL_EN | I2C_INTERRUPT_RX_FIFO_HALF_FULL_EN, ENABLE);//master*/
    }
    else
    {
        /* i2c as slave */
        I2Cx->SAR = i2c->config.i2c_slave_addr; // set unit address as slave
        I2Cx->LCR = 0;
        I2Cx->LCR = (((I2C_CLK/I2C_STANDARD_SPEED - 8)/2) | (((I2C_CLK/I2C_FAST_SPEED - 8)/2 - 1)<<9) | (((I2C_CLK/I2C_HIGH_SPEED - 9)/2)<<18) | (((I2C_CLK/I2C_HIGH_SPEED - 9)/2)<<27));  // set divider
        I2Cx->WCR = (((I2C_CLK/I2C_FAST_SPEED - 8)/2 - 1)/3);
        I2Cx->CR &= ~I2C_MODE_SET_MASK; // reset speed mode to 0
        I2Cx->CR |= (i2c->config.i2c_speed_mode << I2C_MODE_SET_POS); // set speed mode
//            I2Cx->CR |= i2c->i2c_mst_fifo_mode;  // FIFO mode is not for slave mode, so this has no effect
        I2Cx->CR |= I2C_INTERRUPT_SLAVE_ADDR_DET_EN|I2C_INTERRUPT_RX_FIFO_FULL_EN|I2C_INTERRUPT_RX_BUFER_FULL_EN|I2C_INTERRUPT_SLAVE_STOP_DET_EN|I2C_INTERRUPT_TRANS_DONE_EN|I2C_INTERRUPT_TX_BUFFER_EMPTY_EN; //master read
        I2Cx->CR |= I2C_UNIT_ENABLE; // unit enable
        asr_i2c_interrupt_config(I2Cx, I2C_INTERRUPT_SLAVE_ADDR_DET_EN | I2C_INTERRUPT_RX_FIFO_FULL_EN | I2C_INTERRUPT_RX_BUFER_FULL_EN | I2C_INTERRUPT_SLAVE_STOP_DET_EN \
                                           | I2C_INTERRUPT_TRANS_DONE_EN | I2C_INTERRUPT_TX_BUFFER_EMPTY_EN, ENABLE);   //slave

    }
    if(i2c->config.i2c_dma_function == I2C_DMA_ENABLE)
    {
        I2Cx->CR |= I2C_DMA_ENABLE;
    }
    //i2c IRQ enable
    port_i2c_enable_irq(i2c->port);

    /* check the bus busy after unit enable */
    if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_BUSY) )
        return -1;
    else
        return 0;
}

int8_t asr_i2c_master_write_data(asr_i2c_dev_t *i2c, uint8_t slave_addr, uint8_t* pwdata, uint32_t wlen)
{
    I2C_TypeDef * I2Cx;
    uint8_t temp = 0;
    int8_t ret = 0;
    if(NULL == i2c)
      {
          return -EI2CNUMERR;
      }

      if(I2C0_INDEX == i2c->port) // I2C0_INDEX
      {
          I2Cx = I2C0;
      }
      else if(I2C1_INDEX == i2c->port) // I2C1_INDEX
      {
          I2Cx = I2C1;
      }
      else
      {
          return -EI2CNUMERR;
      }

    I2Cx->CR &= (~I2C_CR_SEND_STOP);
    I2Cx->CR &= (~I2C_CR_SEND_NACK);
    // set TXBEGIN bit before starting another transfer
    I2Cx->CR |= I2C_TRANS_BEGIN;
    // send slave address first
    I2Cx->WFIFO = (slave_addr << 1) | I2C_WRITE | I2C_SEND_START | I2C_TB;
    I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY | I2C_STATUS_TRANS_DONE;
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
    if(ret)
        return ret;
    // send write cmd
    while(1)
    {
        if(wlen > I2C_TX_FIFO_DEPTH)
        {
            //send 8 bytes
            for(temp = 0; temp < I2C_TX_FIFO_DEPTH; temp++)
            {
                if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_ERROR_DET) )
                    return -1; //bus error
                else
                    I2Cx->WFIFO = (*pwdata++) | I2C_TB;
                I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
                ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
                if(ret)
                    return ret;
            }
            wlen -= I2C_TX_FIFO_DEPTH;
        }
        else
        {
            //send remaining bytes
            for(temp = 0; temp < wlen; temp++)
            {
                if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_ERROR_DET) )
                    return -1; //bus error
                else
                {
                    if(temp == wlen-1)
                    {
                        I2Cx->WFIFO = (*pwdata) | I2C_SEND_STOP | I2C_TB;
                        ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TRANS_DONE);
                        if(ret)
                            return ret;
                        return 0; // success
                    }
                    else
                    {
                        I2Cx->WFIFO = (*pwdata++) | I2C_TB;
                        I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
                        ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
                        if(ret)
                            return ret;
                    }
                }
            }
        }
    }
    return 0;
}

int8_t asr_i2c_master_read_data(asr_i2c_dev_t *i2c, uint8_t slave_addr, uint8_t *data, uint32_t size)
{
    I2C_TypeDef * I2Cx;
    uint8_t i = 0;
    int8_t ret = 0;
    uint16_t timeout = 0;
    if(NULL == i2c)
      {
          return -EI2CNUMERR;
      }

      if(I2C0_INDEX == i2c->port) // I2C0_INDEX
      {
          I2Cx = I2C0;
      }
      else if(I2C1_INDEX == i2c->port) // I2C1_INDEX
      {
          I2Cx = I2C1;
      }
      else
      {
          return -EI2CNUMERR;
      }

    I2Cx->CR &= (~I2C_CR_SEND_STOP);
    I2Cx->CR &= (~I2C_CR_SEND_NACK);
    // set TXBEGIN bit before starting another transfer
    I2Cx->CR |= I2C_TRANS_BEGIN;

    // send slave address first
    I2Cx->WFIFO = (slave_addr << 1) | I2C_READ | I2C_SEND_START | I2C_TB;
    I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY | I2C_STATUS_TRANS_DONE;
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
    if(ret)
        return ret;
    while(i < size)
    {
        if((size - 1) == i)
        {
            I2Cx->WFIFO = I2C_SEND_STOP | I2C_SEND_NACK | I2C_TB;
        }
        else
        {
            I2Cx->WFIFO = I2C_TB;
        }

        I2Cx->SR = I2C_STATUS_RX_FIFO_FULL;
        timeout = 0;
        while(!(I2Cx->RFIFO_STATUS & 0xF0))
        {
            if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_ERROR_DET) )
                return EBUSERR; //bus error
            timeout++;
            if(timeout > 2000)
                return ETIMEOUT; // timeout
        }
        *(data + i) = I2Cx->RFIFO & 0xFF;
        I2Cx->SR = I2C_STATUS_RX_FIFO_HALF_FULL;
        i++;
    }
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TRANS_DONE);
    if(ret)
        return ret;
    return 0;
}

int8_t asr_i2c_master_repeated_write_read(I2C_TypeDef * I2Cx, uint8_t slave_addr, const uint8_t * pwdata,uint8_t * rdata,uint32_t wlen, uint32_t rlen)
{
    /**** master write ****/

    uint8_t temp = 0, i = 0;
    uint8_t timeout = 0;
    int8_t ret = 0;
    I2Cx->CR &= (~I2C_CR_SEND_STOP);
    I2Cx->CR &= (~I2C_CR_SEND_NACK);
    // set TXBEGIN bit before starting another transfer
    I2Cx->CR |= I2C_TRANS_BEGIN;

    // send slave address first
    I2Cx->WFIFO = (slave_addr << 1) | I2C_WRITE | I2C_SEND_START | I2C_TB;
    I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY | I2C_STATUS_TRANS_DONE;
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
    if(ret)
        return ret;
    // send write cmd
    while(1)
    {

        if(wlen > I2C_TX_FIFO_DEPTH)
        {
            //send 8 bytes
            for(temp = 0; temp < I2C_TX_FIFO_DEPTH; temp++)
            {
                if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_ERROR_DET) )
                    return -1; //bus error
                else
                    I2Cx->WFIFO = (*pwdata++) | I2C_TB;
                I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
                ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
                if(ret)
                    return ret;
            }
            wlen -= I2C_TX_FIFO_DEPTH;
        }
        else
        {
            //send remaining bytes
            for(temp = 0; temp < wlen; temp++)
            {
                if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_ERROR_DET) )
                    return -1; //bus error
                else
                {
                    if(temp == wlen - 1)
                    {
                        I2Cx->WFIFO = (*pwdata) | I2C_TB;   // no need to send STOP for repeated read
                        I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
                        ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
                        if(ret)
                            return ret;
                        break; // write completed
                    }
                    else
                    {
                        I2Cx->WFIFO = (*pwdata++) | I2C_TB;
                        I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
                        ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
                        if(ret)
                            return ret;
                    }
                }
            }
            break;
        }
    }
    /**** master read ****/
    // send slave address first
    I2Cx->WFIFO = (slave_addr << 1) | I2C_READ | I2C_SEND_START | I2C_TB;
    I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_FIFO_EMPTY);
    if(ret)
        return ret;
    while(i < rlen)
    {
        if((rlen - 1) == i)
        {
            I2Cx->WFIFO = I2C_SEND_STOP | I2C_SEND_NACK | I2C_TB;
        }
        else
        {
            I2Cx->WFIFO = I2C_TB;
        }
        timeout = 0;
        I2Cx->SR = I2C_STATUS_RX_FIFO_FULL;
        while(!(I2Cx->RFIFO_STATUS & 0xF0))
        {
            if( asr_i2c_get_flag_status(I2Cx, I2C_STATUS_BUS_ERROR_DET) )
                return -1; //bus error
            timeout++;
            if(timeout > 2000)
                return -2; // timeout
        }
        *(rdata + i) = I2Cx->RFIFO & 0xFF;
        I2Cx->SR = I2C_STATUS_RX_FIFO_HALF_FULL;
        i++;
    }
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TRANS_DONE);
    if(ret)
        return ret;
    return 0;
}


int8_t asr_i2c_write_dbr(I2C_TypeDef * I2Cx, uint8_t slave_addr, uint8_t* pwdata, uint32_t wlen)
{
    uint8_t temp = 0;
    uint8_t ret = 0;
    I2Cx->DBR = slave_addr << 1;
    I2Cx->CR |= I2C_CR_SEND_START;
    I2Cx->CR &= ~I2C_CR_SEND_STOP;
    I2Cx->CR &= ~I2C_INTERRUPT_ARBT_LOSS_DET_EN;
    I2Cx->CR |= I2C_CR_TB;
    I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_BUFFER_EMPTY);
    if(ret)
    {
        return ret;
    }
    while(temp < wlen)
    {
        I2Cx->DBR = *(pwdata++);
        I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
        if (temp == wlen - 1)
        {
            I2Cx->CR &= ~I2C_CR_SEND_START;
            I2Cx->CR |= I2C_CR_SEND_STOP;
            //I2Cx->CR |= I2C_INTERRUPT_ARBT_LOSS_DET_EN;
            I2Cx->CR |= I2C_CR_TB;
        }
        else
        {
            I2Cx->CR &= ~I2C_CR_SEND_START;
            I2Cx->CR &= ~I2C_CR_SEND_STOP;
            I2Cx->CR &= ~I2C_INTERRUPT_ARBT_LOSS_DET_EN;
            I2Cx->CR |= I2C_CR_TB;
        }
        I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
        ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_BUFFER_EMPTY);
        if(ret)
            return ret;
        temp++;
    }

    I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
    I2Cx->CR &= ~I2C_CR_SEND_STOP;
    return 0;
}

int8_t asr_i2c_read_dbr(I2C_TypeDef * I2Cx, uint8_t slave_addr,uint8_t *data, uint32_t size)
{
    uint8_t temp = 0;
    uint8_t ret = 0;
    I2Cx->DBR = slave_addr << 1 | 1;
    I2Cx->CR |= I2C_CR_SEND_START;
    I2Cx->CR &= ~I2C_CR_SEND_STOP;
    I2Cx->CR &= (~I2C_CR_SEND_NACK);
    I2Cx->CR &= ~I2C_INTERRUPT_ARBT_LOSS_DET_EN;
    I2Cx->CR |= I2C_CR_TB;
    I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY | I2C_STATUS_RX_BUFER_FULL;
    ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_TX_BUFFER_EMPTY);
    if(ret)
        return ret;
    I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
    while(temp < size)
    {
        if (temp == size - 1)
        {
            I2Cx->CR &= ~I2C_CR_SEND_START;
            I2Cx->CR |= I2C_CR_SEND_STOP;
            I2Cx->CR |= I2C_INTERRUPT_ARBT_LOSS_DET_EN;
            I2Cx->CR |= I2C_CR_TB;
            I2Cx->CR |= I2C_CR_SEND_NACK;
        }
        else
        {
            I2Cx->CR &= ~I2C_CR_SEND_START;
            I2Cx->CR &= ~I2C_CR_SEND_STOP;
            I2Cx->CR &= ~I2C_INTERRUPT_ARBT_LOSS_DET_EN;
            I2Cx->CR |= I2C_CR_TB;
            I2Cx->CR &= ~I2C_CR_SEND_NACK;
        }

        ret = asr_i2c_check_flag(I2Cx, I2C_STATUS_RX_BUFER_FULL);
        if(ret)
            return ret;
        I2Cx->SR = I2C_STATUS_RX_BUFER_FULL;
        *(data++) = I2Cx->DBR;
        temp++;
    }

    I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
    I2Cx->CR &= ~I2C_CR_SEND_STOP;
    I2Cx->CR &= ~I2C_CR_SEND_NACK;
    return 0;
}

/**
 * Deinitialises an I2C device
 *
 * @param[in]  i2c  the i2c device
 *
 * @return  0 : on success, EIO : if an error occurred during deinitialisation
 */
int32_t asr_i2c_finalize(asr_i2c_dev_t *i2c)
{
    //I2C_TypeDef *I2Cx;

    if(NULL == i2c)
    {
        return -EI2CNUMERR;
    }
    if(I2C0_INDEX == i2c->port) // I2C0_INDEX
    {
        //I2Cx = I2C0;
    }
    else if(I2C1_INDEX == i2c->port) // I2C1_INDEX
    {
        //I2Cx = I2C1;
    }
    else
    {
        return -EI2CNUMERR;
    }

    //disable cm4 interrupt
    port_i2c_disable_irq(i2c->port);

    //spi sclk disable, fpga no effect, soc need
    port_i2c_deinit_clk(i2c->port);

    return 0;
}

#if (I2C_MODE == I2C_MASTER)
void I2C_IRQHandler(I2C_TypeDef * I2Cx)
{
    //if you want to see this interrput bit, you can add it.
    if( (I2Cx->SR) & I2C_STATUS_TX_FIFO_EMPTY )
    {
        printf("I2Cx I2C_STATUS_TX_FIFO_EMPTY\n");
        I2Cx->SR = I2C_STATUS_TX_FIFO_EMPTY;
    }
    if( (I2Cx->SR) & I2C_STATUS_SLAVE_STOP_DET)
    {
        printf("I2Cx I2C_STATUS_SLAVE_STOP_DET\n");
        I2Cx->SR = I2C_STATUS_SLAVE_STOP_DET;
    }
    if( (I2Cx->SR) & I2C_STATUS_TRANS_DONE)
    {
        printf("I2Cx trans done interrupt\n");
        I2Cx->SR = I2C_STATUS_TRANS_DONE;
    }
    if( (I2Cx->SR) & I2C_STATUS_TX_BUFFER_EMPTY)
    {
        printf("I2Cx I2C_STATUS_TX_BUFFER_EMPTY\n");
        I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
    }
    if( (I2Cx->SR) & I2C_STATUS_SLAVE_ADDR_DET)
    {
        printf("I2Cx I2C_STATUS_SLAVE_ADDR_DET\n");
        I2Cx->SR = I2C_STATUS_SLAVE_ADDR_DET;
        I2Cx->CR |= I2C_CR_TB;
    }
    if( (I2Cx->SR) & I2C_STATUS_RX_FIFO_HALF_FULL)
    {
        printf("I2Cx I2C_STATUS_RX_FIFO_HALF_FULL\n");
        I2Cx->CR |= I2C_CR_TB;
        I2Cx->SR = I2C_STATUS_RX_FIFO_HALF_FULL;
    }
    if( (I2Cx->SR) & I2C_STATUS_RX_BUFER_FULL)
    {
        printf("I2Cx I2C_STATUS_RX_BUFER_FULL\n");
        I2Cx->SR = I2C_STATUS_RX_BUFER_FULL;
        I2Cx->CR |= I2C_CR_TB;
    }
}
#else
uint8_t data_rec1[20] = {0};
uint8_t cnt=0,index_rcv=0;
uint8_t tx_data1[20] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0xfe,0xfd,0xfc};
void I2C_IRQHandler(I2C_TypeDef * I2Cx)
{
//    printf("I2C0 status = 0x%x\n", I2C0->SR);
//    if( (I2C0->SR) & I2C_STATUS_TX_FIFO_EMPTY )
//    {
//        I2C0->SR = I2C_STATUS_TX_FIFO_EMPTY;
////            i2c_interrupt_config(I2C0, I2C_INTERRUPT_TX_FIFO_EMPTY_EN, DISABLE);
//    }
    if( (I2Cx->SR) & I2C_STATUS_SLAVE_STOP_DET)
    {
        printf("I2Cx SLAVE_STOP_DET,recv data:");
        for(uint8_t j = 0;j < index_rcv;j++)
        {
            printf(" %x ",data_rec1[j]);
        }
        cnt = 0;
        index_rcv = 0;
        printf("\r\n");
        I2Cx->SR = I2C_STATUS_SLAVE_STOP_DET;
    }
    if( (I2Cx->SR) & I2C_STATUS_TRANS_DONE)
    {
        printf("I2Cx trans done interrupt\n");
        I2Cx->SR = I2C_STATUS_TRANS_DONE;
    }
    if( (I2Cx->SR) & I2C_STATUS_TX_BUFFER_EMPTY)
    {
        printf("I2Cx trans buff empty interrupt\n");
        I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
        if(!((I2Cx->SR) & I2C_STATUS_ACK_NACK))
        {
            I2Cx->DBR = tx_data1[cnt++];
            I2Cx->CR |= I2C_CR_TB;
        }
        else
        {
            cnt = 0;
            I2Cx->DBR = 0;
            printf("recv data:");
            if(index_rcv)
            {
                for(uint8_t j = 0;j < index_rcv;j++)
                {
                    printf(" %x ",data_rec1[j]);
                }
                index_rcv = 0;
                printf("\r\n");
            }
            printf("I2Cx  NO ACK recv\r\n");
        }
//        I2Cx->SR = I2C_STATUS_TX_BUFFER_EMPTY;
    }
    if( (I2Cx->SR) & I2C_STATUS_SLAVE_ADDR_DET)
    {

        printf("I2Cx slave addr det:");
        if(((I2Cx->SR) & I2C_STATUS_RW_MODE))
        {
//            I2Cx->CR |= I2C_CR_TB;
            I2Cx->DBR = tx_data1[cnt++];
            printf("master read\r\n");
        }
        else
        {
//            I2Cx->CR |= I2C_CR_TB;
            printf("master write\r\n");
        }
        I2Cx->SR = I2C_STATUS_SLAVE_ADDR_DET;
        I2Cx->CR |= I2C_CR_TB;
    }
    if( (I2Cx->SR) & I2C_STATUS_RX_FIFO_HALF_FULL)
    {
        printf("I2Cx RX_FIFO_HALF_FULL\n");
        I2Cx->CR |= I2C_CR_TB;
        I2Cx->SR = I2C_STATUS_RX_FIFO_HALF_FULL;
    }
    if( (I2Cx->SR) & I2C_STATUS_RX_BUFER_FULL)
    {
        printf("I2Cx RX_BUFER_FULL\n");
//        I2Cx->CR |= I2C_CR_TB;
        data_rec1[index_rcv] = I2Cx->DBR;
        index_rcv++;
        I2Cx->SR = I2C_STATUS_RX_BUFER_FULL;
        I2Cx->CR |= I2C_CR_TB;
    }
}
#endif
void I2C0_IRQHandler(void)
{
    I2C_IRQHandler(I2C0);
}

void I2C1_IRQHandler(void)
{
    I2C_IRQHandler(I2C1);
}

