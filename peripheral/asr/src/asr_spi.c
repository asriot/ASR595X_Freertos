#include "asr_spi.h"

#include "asr_port.h"
#include "asr_pinmux.h"
#include "asr_port_peripheral.h"
asr_spi_callback_func g_asr_spi_callback_handler[SPI_NUM] = {0};

SPI_TypeDef* getSpixViaIdx(uint8_t spi_idx)
{
    switch(spi_idx){
        case SPI0_INDEX:
            return SPI0;
        case SPI1_INDEX:
            return SPI1;
        case SPI2_INDEX:
            return SPI2;
        default:
            return NULL;
    }
}

void asr_spi_struct_init(asr_spi_dev_t * init_struct)
{
    init_struct->config.freq = 1000000; // 1M
    init_struct->config.mode = SPI_ROLE_MASTER;
    init_struct->priv = NULL;
}

void asr_spi_interrupt_config(SPI_TypeDef * SPIx, uint8_t spi_interrupt, uint8_t new_state)
{
    if(new_state == ENABLE)
        SPIx->IMSC |= spi_interrupt;
    else
        SPIx->IMSC &= ~spi_interrupt;
}

int32_t asr_spi_dma_config(asr_spi_dev_t * spi,uint8_t dma_tx_rx_sel,uint8_t new_state)
{
    SPI_TypeDef* SPIx = NULL;
    if(spi->port>=SPI_NUM)
        return EIO;
    SPIx = getSpixViaIdx(spi->port);

    if(new_state == ENABLE)
    {
        SPIx->DMA_CR |= dma_tx_rx_sel;
    }
    else
    {
        SPIx->DMA_CR &= ~dma_tx_rx_sel;
    }
    return 0;
}

void asr_spi_cmd(SPI_TypeDef * SPIx, uint8_t new_state)
{
    if(new_state == ENABLE)
        SPIx->CR1 |= (0x1<<1);
    else
        SPIx->CR1 &= ~(0x1<<1);
}

int32_t asr_spi_cpol_cpha_config(asr_spi_dev_t * spi,uint8_t mode)
{
    uint8_t cpol, cpha;
    SPI_TypeDef* SPIx = NULL;
    if(spi->port>=SPI_NUM)
        return EIO;
    SPIx = getSpixViaIdx(spi->port);
    SPIx->CR1 &= ~(0x1<<1);
    switch (mode)
    {
        case 0:
            cpol = 0;
            cpha = 0;
            break;
        case 1:
            cpol = 0;
            cpha = 1;
            break;
        case 2:
            cpol = 1;
            cpha = 0;
            break;
        case 3:
            cpol = 1;
            cpha = 1;
            break;
        default:
            cpol = 0;
            cpha = 0;
            break;
    }
    /* set sclk polarity & phase */
    SPIx->CR0 &= ~(0x3 << 6);  // reset SPI clk phase/polarity setting to mode 0
    SPIx->CR0 |= (cpol << SPI_CLK_POLARITY_POS) | (cpha << SPI_CLK_PHASE_POS);
    SPIx->CR1 |= (0x1<<1);
    return 0;
}

int32_t asr_spi_init(asr_spi_dev_t * spi)
{
    uint32_t spi_clk = SPI_CLK;
    uint8_t cpol, cpha;
    SPI_TypeDef* SPIx = NULL;

    if(!spi) return EIO;

    if(spi->port>=SPI_NUM)
        return EIO;
    SPIx = getSpixViaIdx(spi->port);
    //enable spi clk
    port_spi_init_clk(spi->port);

    //fpga no effect, soc need
    asr_spi_cmd(SPIx, DISABLE);
    asr_spi_interrupt_config(SPIx, SPI_INTERRUPT_ALL, DISABLE);
    asr_spi_interrupt_clear(SPIx, SPI_INTERRUPT_ALL);

    /* set frame format */
    SPIx->CR0 &= ~(0x3 << 4);  // reset FRF to 0
    SPIx->CR0 |= SPI_FRAME_FORMAT_SPI;
    /* set sclk divider */
    SPIx->CPSR &= ~0xff; // reset CPSR to 0
    SPIx->CPSR |= 0x2;   // set CPSR to 2, shoule be even number between 2-254
    SPIx->CR0 &= (0x00ff); // reset SCR to 0
    SPIx->CR0 |= (spi_clk/2/spi->config.freq - 1) << 8;    // set SCR to 0x7, serial clk = 16M/2/(1+7) = 1M
    switch (SPI_CPOL_CPHA_MODE)
    {
        case 0:
            cpol = 0;
            cpha = 0;
            break;
        case 1:
            cpol = 0;
            cpha = 1;
            break;
        case 2:
            cpol = 1;
            cpha = 0;
            break;
        case 3:
            cpol = 1;
            cpha = 1;
            break;
        default:
            cpol = 0;
            cpha = 0;
            break;
    }
    /* set sclk polarity & phase */
    SPIx->CR0 &= ~(0x3 << 6);  // reset SPI clk phase/polarity setting to mode 0
    SPIx->CR0 |= (cpol << SPI_CLK_POLARITY_POS) | (cpha << SPI_CLK_PHASE_POS);

    /* set data size */
    SPIx->CR0 &= ~(0xf);   // reset data size to 0
    SPIx->CR0 |= SPI_DATA_SIZE_8BIT;

    if(spi->config.mode == SPI_ROLE_MASTER)
    {
        SPIx->CR1 &= ~(0x1 << 2);  // reset master/slave select to 0, which is master mode
    }
    else
    {
        SPIx->CR1 &= ~(0x1 << 2);  // reset master/slave select to 0, which is master mode
        SPIx->CR1 |= SPI_ROLE_SLAVE; // set to slave role
    }

    if(spi->priv)
    {
        //enable rx interrupt
        SPIx->IMSC |= (SPI_INTERRUPT_RX_TIMEOUT|SPI_INTERRUPT_RX_FIFO_TRIGGER);
        //enable cm4 interrupt
        port_spi_enable_irq(spi->port);
        g_asr_spi_callback_handler[spi->port] = (asr_spi_callback_func)(spi->priv);
    }
    SPIx->CR1 |= (0x1<<1);
    return 0;
}

/**
 * De-initialises a SPI interface
 *
 *
 * @param[in]  spi the SPI device to be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred
 */
int32_t asr_spi_finalize(asr_spi_dev_t *spi)
{
    SPI_TypeDef* SPIx;


    if(NULL == spi)
    {
        return EIO;
    }

    if(spi->port>=SPI_NUM)
        return EIO;
    SPIx = getSpixViaIdx(spi->port);

    //disable all spi interrupt
    SPIx->IMSC  = SPI_DISABLE_INTERRUPT_ALL;
    //disable all spi config
    SPIx->CR0 = 0;
    SPIx->CR1 = 0;

    //disable cm4 interrupt
    port_spi_disable_irq(spi->port);

    //spi sclk disable, fpga no effect, soc need
    port_spi_deinit_clk(spi->port);

    g_asr_spi_callback_handler[spi->port] = NULL;
    return 0;
}

int32_t asr_spi_send(asr_spi_dev_t *spi, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    SPI_TypeDef* SPIx = NULL;
    if(spi->port>=SPI_NUM)
        return EIO;
    SPIx = getSpixViaIdx(spi->port);
    while(size--)
    {
        while( !(asr_spi_get_flag_status(SPIx, SPI_FLAG_TX_FIFO_NOT_FULL)) ); //wait till tx fifo is not full
        SPIx->DR = *data++;
    }
    return 0;
}

//void asr_spi_receive(SPI_TypeDef * SPIx, void * rx_data, uint16_t len)
//{
//    while(len--)
//    {
//        while( !(spi_get_flag_status(SPIx, SPI_FLAG_RX_FIFO_NOT_EMPTY)) ); //wait till rx fifo is not empty, change to timeout mechanism???

//        *rx_data++ = SPIx->DR ;
//    }
//}


/* spi interrupt handler */
void SPIX_IRQHandler(uint8_t spi_idx)
{
    uint16_t tmp;
    SPI_TypeDef* SPIx = getSpixViaIdx(spi_idx);
    if( asr_spi_get_interrupt_status(SPIx,SPI_INTERRUPT_TX_FIFO_TRIGGER))
    {
        asr_spi_interrupt_config(SPIx,SPI_INTERRUPT_TX_FIFO_TRIGGER,DISABLE); // disable
        asr_spi_interrupt_clear(SPIx,SPI_INTERRUPT_TX_FIFO_TRIGGER); // clear
        asr_spi_interrupt_config(SPIx,SPI_INTERRUPT_TX_FIFO_TRIGGER,ENABLE); // enable
    }
    if( asr_spi_get_interrupt_status(SPIx,SPI_INTERRUPT_RX_FIFO_TRIGGER) || asr_spi_get_interrupt_status(SPIx,SPI_INTERRUPT_RX_TIMEOUT) )
    {
        asr_spi_interrupt_config(SPIx,SPI_INTERRUPT_RX_FIFO_TRIGGER|SPI_INTERRUPT_RX_TIMEOUT,DISABLE); // disable
        asr_spi_interrupt_clear(SPIx,SPI_INTERRUPT_RX_FIFO_TRIGGER|SPI_INTERRUPT_RX_TIMEOUT); // clear
        while( SPIx->SR & SPI_FLAG_RX_FIFO_NOT_EMPTY)
        {
            tmp = (uint16_t)(SPIx->DR);
            if(g_asr_spi_callback_handler[spi_idx] != NULL)
            {
                g_asr_spi_callback_handler[spi_idx](tmp);
            }
        }
        asr_spi_interrupt_config(SPIx,SPI_INTERRUPT_RX_FIFO_TRIGGER|SPI_INTERRUPT_RX_TIMEOUT,ENABLE); // enable
    }
}

void SPI0_IRQHandler()
{
    asr_intrpt_enter();
    SPIX_IRQHandler(0);
    asr_intrpt_exit();
}

void SPI1_IRQHandler()
{
    asr_intrpt_enter();
    SPIX_IRQHandler(1);
    asr_intrpt_exit();
}

void SPI2_IRQHandler()
{
    asr_intrpt_enter();
    SPIX_IRQHandler(2);
    asr_intrpt_exit();
}

void asr_spi_set_callback(uint8_t spi_idx,asr_spi_callback_func func)
{
    if(spi_idx >= SPI_NUM)
        return;
    g_asr_spi_callback_handler[spi_idx] = func;

}