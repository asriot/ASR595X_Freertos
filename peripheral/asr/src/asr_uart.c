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
#include "asr_uart.h"

#include "asr_port.h"
#include "asr_pinmux.h"
#include "asr_port_peripheral.h"

asr_uart_callback_func_t g_asr_uart_callback_handler[UART_NUM];

UART_TypeDef* uart_get_uartx_via_idx(uint8_t uart_idx)
{
    if(uart_idx>=UART_NUM)
        return NULL;
    switch(uart_idx){
        case 0: return UART0;
        case 1: return UART1;
        case 2: return UART2;
        case 3: return UART3;
        default:return NULL;
    }
}

//    //enable clk
//    tmp_value = REG_RD(PERI_CLK_EN_REG1) & (~(bus_clk_enable_bit | peri_clk_enable_bit));
//    REG_WR(PERI_CLK_EN_REG1, (tmp_value | (bus_clk_enable_bit | peri_clk_enable_bit)));

//    //APB CLK DIV
//    tmp_value = REG_RD(APB_CLK_DIV_REG);
//    tmp_value &= (~(0X03<<2));
//    REG_WR(APB_CLK_DIV_REG, (tmp_value | (APBCLK_DIV_CFG)));
//}
//static void uart_deinit_clk(uint8_t uart_idx)
//{
//    uint8_t bus_clk_enable_bit = 0x01 << (uart_idx+1);
//    uint8_t peri_clk_enable_bit = 0x01 << (uart_idx+14);
//    uint32_t tmp_value=0;

//    tmp_value = REG_RD(PERI_CLK_DIS_REG1) & (~(bus_clk_enable_bit|peri_clk_enable_bit));
//    REG_WR(PERI_CLK_DIS_REG1, (tmp_value | (bus_clk_enable_bit|peri_clk_enable_bit)));

//}

/* get uart flag status */
DEBUG_SECTION uint8_t uart_get_flag_status(UART_TypeDef* UARTx, uint8_t uart_flag)
{
    if(UARTx->FR & uart_flag)
        return 1;
    else
        return 0;
}

DEBUG_SECTION void uartx_put_char(UART_TypeDef* UARTx, uint8_t c)
{
    if(UARTx == NULL)
        return;
    while( uart_get_flag_status(UARTx, UART_FLAG_TX_FIFO_FULL));

    UARTx->DR = c;
}
static void uart_disable(UART_TypeDef* uartx)
{
    uartx->CR &= (~(0x1));
}
static void uart_enable(UART_TypeDef* uartx)
{
     uartx->CR |= 0x1;
}

static uint32_t uart_calc_baud(uint32_t baud)
{
  uint32_t int_div;
  uint32_t fac_div;
  uint32_t remainder;
  uint32_t temp;
  uint32_t uart_clk = UART_CLK;

  temp = 16 * baud;
  if ((0 == baud) || uart_clk < temp){
    return 0;
  }
  int_div = (uint32_t)(uart_clk / temp);
  remainder = uart_clk % temp;
  temp = 8 * remainder / baud;
  fac_div = (temp >> 1) + (temp & 1);

  temp = ((int_div << 16) | (fac_div & 0xFFFF));
  return temp;
}
static void uart_set_baudrate(UART_TypeDef* UARTx, uint32_t baud_rate)
{
    uint32_t br_div = uart_calc_baud(baud_rate);
    UARTx->IBRD = br_div >> 16;  /* baudrate divdier register must be placed before a LCR_H write */
    UARTx->FBRD = br_div & 0x3f;
}
static void uart_set_data_width(UART_TypeDef* UARTx,asr_uart_data_width_t data_width)
{
    UARTx->LCR_H |= (data_width << 5);
}
static void uart_set_stop_bits(UART_TypeDef* UARTx,asr_uart_stop_bits_t stop_bits)
{
    UARTx->LCR_H |= (stop_bits << 3);
}
static void uart_set_parity(UART_TypeDef* UARTx,asr_uart_parity_t parity)
{
    switch(parity)
    {
        case PARITY_ODD:
            UARTx->LCR_H &= ~(3<<1);
            UARTx->LCR_H |= (1<<1);
            break;
        case PARITY_EVEN:
            UARTx->LCR_H |= (3<<1);
            break;
        case PARITY_NO:
            UARTx->LCR_H &= ~(1<<1);
            break;
        default:
            break;
    }
}
static void uart_set_txrx_mode(UART_TypeDef* UARTx,asr_uart_mode_t mode)
{
    uint32_t CR_mode = 0;
    switch(mode)
    {
        case TX_MODE:
            CR_mode = 1;
            break;
        case RX_MODE:
            CR_mode = 2;
            break;
        case TX_RX_MODE:
            CR_mode = 3;
            break;
        default:
            CR_mode = 3;
            break;
    }
    // set CR
    UARTx->CR &= ~(3<<8); // set tx/rx mode to 0
     UARTx->CR |=  (CR_mode << 8);
}
static void uart_set_flow_control(UART_TypeDef* UARTx,asr_uart_flow_control_t flow_ctl)
{
    uint32_t CR_flowCtl = 0;
    switch(flow_ctl)
    {
        case FLOW_CTRL_DISABLED:
            CR_flowCtl = 0;
            break;
        case FLOW_CTRL_CTS:
            CR_flowCtl = 2;
            break;
        case FLOW_CTRL_RTS:
            CR_flowCtl = 1;
            break;
        case FLOW_CTRL_CTS_RTS:
            CR_flowCtl = 3;
            break;
        default:
            CR_flowCtl = 0;
            break;
    }
    UARTx->CR |= (CR_flowCtl << 14);
}
static void uart_flush_fifo(UART_TypeDef* UARTx)
{
    UARTx->LCR_H &= ~(0x1 << 4);
}
static void uart_enable_fifo(UART_TypeDef* UARTx)
{
    UARTx->LCR_H |= (1 << 4);
}
static void uart_set_tx_fifo_threshold(UART_TypeDef* UARTx,asr_uart_fifo_threshold_t fifo_threshold)
{
    UARTx->IFLS &= ~(0x7);
    UARTx->IFLS |= fifo_threshold;  //tx fifo threshold
}
static void uart_set_rx_fifo_threshold(UART_TypeDef* UARTx,asr_uart_fifo_threshold_t fifo_threshold)
{
    UARTx->IFLS &= ~(0x7 << 3);
    UARTx->IFLS |= (FIFO_HALF_FULL << 3); //rx fifo threshold
}
static void uart_set_int_status(UART_TypeDef* UARTx,uint32_t UART_INT_FLAG,uint8_t enable)
{
    if(enable)
        UARTx->IMSC |= UART_INT_FLAG;
    else
        UARTx->IMSC &= (~UART_INT_FLAG);
}
static void uart_enable_int(uint8_t uart_idx,uint32_t UART_INT_FLAG)
{
    UART_TypeDef* UARTx = uart_get_uartx_via_idx(uart_idx);
    uart_set_int_status(UARTx,UART_INT_FLAG,1);

    port_uart_enable_irq(uart_idx);
}
static void uart_disable_int(uint8_t uart_idx,uint32_t UART_INT_FLAG)
{
    UART_TypeDef* UARTx = uart_get_uartx_via_idx(uart_idx);
    uart_set_int_status(UARTx,UART_INT_FLAG,0);

    port_uart_disable_irq(uart_idx);
}
static void uart_disable_all_config(UART_TypeDef* UARTx)
{
    UARTx->CR = 0;
    UARTx->LCR_H = 0;
    UARTx->DMACR &= ~(UART_DMA_TX_EN | UART_DMA_RX_EN);
}

static void uart_set_rx_callback(uint8_t uart_idx,asr_uart_callback_func_t callback)
{
    uart_enable_int(uart_idx,UART_RX_TIMEOUT_INTERRUPT|UART_RX_INTERRUPT);
    g_asr_uart_callback_handler[uart_idx] = callback;
}
int32_t asr_uart_dma_config(asr_uart_dev_t* uart,uint8_t dma_tx_rx_sel,uint8_t new_state)
{
    UART_TypeDef* UARTx = NULL;
    if(uart->port>=UART_NUM)
        return EIO;
    UARTx = uart_get_uartx_via_idx(uart->port);
    if(new_state == ENABLE)
    {
        UARTx->DMACR |= dma_tx_rx_sel;
    }
    else
    {
        UARTx->DMACR &= ~dma_tx_rx_sel;
    }
    return 0;
}

void asr_uart_struct_init(asr_uart_dev_t* UART_InitStruct)
{
    if(!UART_InitStruct)
        return;
    UART_InitStruct->config.baud_rate = UART_BAUDRATE_115200;
    UART_InitStruct->config.data_width = DATA_8BIT;
    UART_InitStruct->config.parity = PARITY_NO;
    UART_InitStruct->config.stop_bits = STOP_1BIT;
    UART_InitStruct->config.flow_control = FLOW_CTRL_DISABLED;
    UART_InitStruct->config.mode = TX_RX_MODE;
    UART_InitStruct->priv = NULL;
}

/**
 * Initialises a UART interface
 *
 *
 * @param[in]  uart  the interface which should be initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_init(asr_uart_dev_t *uart)
{
    UART_TypeDef* UARTx=NULL;
    uint8_t uart_idx = 0;
    if(!uart) return EIO;

    uart_idx = uart->port;

    UARTx = uart_get_uartx_via_idx(uart_idx);
    if(!UARTx) return EIO;

    //enable uart clk
    port_uart_init_clk(uart_idx);

    // wait for the end of current charater
    while(uart_get_flag_status(UARTx, UART_FLAG_BUSY));

    //disable UART
    uart_disable(UARTx);
    //disable all uart config
    uart_disable_int(uart_idx,UART_ALL_IRQ);
    uart_disable_all_config(UARTx);

    //flush fifo
    uart_flush_fifo(UARTx);

    //Reprogram the UARTCR Register
    uart_set_baudrate(UARTx,uart->config.baud_rate);
    uart_set_data_width(UARTx,uart->config.data_width);
    uart_set_stop_bits(UARTx,uart->config.stop_bits);
    uart_set_parity(UARTx,uart->config.parity);
    uart_set_txrx_mode(UARTx,uart->config.mode);
    uart_set_flow_control(UARTx,uart->config.flow_control);

    //set fifo threshold
    uart_enable_fifo(UARTx);
    uart_set_tx_fifo_threshold(UARTx,FIFO_HALF_FULL);
    uart_set_rx_fifo_threshold(UARTx,FIFO_HALF_FULL);

    g_asr_uart_callback_handler[uart_idx] = NULL;
    if(uart->priv)
    {
        uart_set_rx_callback(uart_idx,(asr_uart_callback_func_t)(uart->priv));
    }

    //enable UART
    uart_enable(UARTx);


    return 0;
}

/**
 * Transmit data on a UART interface
 *
 * @param[in]  uart  the UART interface
 * @param[in]  data  pointer to the start of data
 * @param[in]  size  number of bytes to transmit
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_send(asr_uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    UART_TypeDef* UARTx=NULL;
    uint8_t uart_idx = 0;
    int i = 0;

    if((NULL == uart) || (NULL == data))
    {
        return EIO;
    }

    uart_idx = uart->port;

    UARTx = uart_get_uartx_via_idx(uart_idx);
    if(!UARTx) return EIO;

    for(i = 0; i < size; i++)
    {
        uartx_put_char(UARTx,((uint8_t*)data)[i]);
    }

    return 0;
}
#ifdef RTOS_SUPPORT
/**
 * Receive data on a UART interface
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_recv(asr_uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t timeout)
{
    int i = 0;
    int32_t ret = 0;
    uint8_t uart_idx = 0;
    uint8_t *pdata = (uint8_t *)data;

    if((uart == NULL) || (data == NULL))
    {
        return EIO;
    }
#ifdef ALIOS_SUPPORT
    uart_idx = get_uart_phy_port(uart->port);
#else
    uart_idx = uart->port;
#endif
    for (i = 0; i < expect_size; i++)
    {
        ret = lega_rtos_pop_from_queue(&asr_uart_buf_queue[uart_idx], &pdata[i], timeout);
        if(ret)
        {
            return EIO;
        }
    }

    return 0;
}

/**
 * Receive data on a UART interface
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[out]  recv_size    number of bytes received
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_recv_II(asr_uart_dev_t *uart, void *data, uint32_t expect_size,
                      uint32_t *recv_size, uint32_t timeout)
{
    uint8_t *pdata = (uint8_t *)data;
    int i = 0;
    uint32_t rx_count = 0;
    uint8_t uart_idx = 0;
    int32_t ret = 0;

    if((uart == NULL) || (data == NULL))
    {
        return EIO;
    }

#ifdef ALIOS_SUPPORT
    uart_idx = get_uart_phy_port(uart->port);
#else
    uart_idx = uart->port;
#endif

    for (i = 0; i < expect_size; i++)
    {
        ret = lega_rtos_pop_from_queue(&asr_uart_buf_queue[uart_idx], &pdata[i], timeout);

        if(!ret)
        {
            rx_count++;
        }
        else
        {
            if(recv_size)
                {
                    *recv_size = rx_count;
                    return EIO;
                }

        }
    }
    if(recv_size)
    {
        *recv_size = rx_count;
    }
    return 0;
}

#endif

/**
 * Deinitialises a UART interface
 *
 * @param[in]  uart  the interface which should be deinitialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_finalize(asr_uart_dev_t *uart)
{
    uint8_t uart_idx = 0;
    UART_TypeDef* UARTx=NULL;

    if(NULL == uart){
        return EIO;
    }

    uart_idx = uart->port;

    UARTx = uart_get_uartx_via_idx(uart_idx);

    if(uart->priv)
    {
        g_asr_uart_callback_handler[uart_idx] = NULL;
    }
    //disable all uart config
    uart_disable_int(uart_idx,UART_ALL_IRQ);
    uart_disable_all_config(UARTx);
    port_uart_deinit_clk(uart_idx);
    return 0;
}

static uint8_t uart_get_interrupt_status(UART_TypeDef* UARTx,uint32_t UART_INT_FLAG)
{
    if( UARTx->MIS & UART_INT_FLAG )
        return 1;
    else
        return 0;
}

static void uart_clear_interrupt(UART_TypeDef* UARTx, uint32_t UART_INT_FLAG)
{
    UARTx->ICR |= UART_INT_FLAG;
}

static void UARTX_IRQHandler(uint8_t uart_idx)
{
    uint8_t tmp=0;
    UART_TypeDef* UARTx = uart_get_uartx_via_idx(uart_idx);
    if( uart_get_interrupt_status(UARTx, UART_RX_INTERRUPT) ||  uart_get_interrupt_status(UARTx, UART_RX_TIMEOUT_INTERRUPT))
    {
        uart_set_int_status(UARTx,UART_RX_INTERRUPT|UART_RX_TIMEOUT_INTERRUPT,0);

        uart_clear_interrupt(UARTx, UART_RX_INTERRUPT|UART_RX_TIMEOUT_INTERRUPT);

        /* read rx fifo till it's empty */
        while( ! uart_get_flag_status(UARTx, UART_FLAG_RX_FIFO_EMPTY) )
        {
            tmp = (uint8_t)(UARTx->DR);
            if(g_asr_uart_callback_handler[uart_idx] != NULL)
            {
                g_asr_uart_callback_handler[uart_idx](tmp);
            }
        }

        uart_set_int_status(UARTx,UART_RX_INTERRUPT|UART_RX_TIMEOUT_INTERRUPT,1);
    }
}

void UART0_IRQHandler()
{
    asr_intrpt_enter();
    UARTX_IRQHandler(0);
    asr_intrpt_exit();
}

void UART1_IRQHandler()
{
    asr_intrpt_enter();
    UARTX_IRQHandler(1);
    asr_intrpt_exit();
}

void UART2_IRQHandler()
{
    asr_intrpt_enter();
    UARTX_IRQHandler(2);
    asr_intrpt_exit();
}

void asr_uart_set_callback(uint8_t uart_idx,asr_uart_callback_func_t func)
{
    if(g_asr_uart_callback_handler[uart_idx])
    { //enable => enable
        g_asr_uart_callback_handler[uart_idx] = func;
    }
    else
    {//disable => enable
        uart_set_rx_callback(uart_idx,func);
    }
}
