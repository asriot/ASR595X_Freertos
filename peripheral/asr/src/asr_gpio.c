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
#include "asr_gpio.h"
#include "asr_pinmux.h"
#include "asr_port_peripheral.h"

asr_gpio_cb_t g_asr_gpio_handler[ASR_GPIO_TOTAL_NUM];
static GPIO_TypeDef * gpio_get_GPIOGROUP(Pad_Num_Type pad_num)
{
    if(GPIO_GROUP0 && (pad_num < ASR_GPIO_NUM_PER_GROUP))
        return GPIO_GROUP0;
    else if(GPIO_GROUP1 && (pad_num >= ASR_GPIO_NUM_PER_GROUP && pad_num < 2*ASR_GPIO_NUM_PER_GROUP))
        return GPIO_GROUP1;
    else
        return NULL;
}
static uint8_t gpio_get_padoffset(Pad_Num_Type pad_num)
{
    return pad_num % ASR_GPIO_NUM_PER_GROUP;
}
static Pad_Num_Type gpio_get_pad_num(asr_gpio_dev_t *gpio)
{
    if(!gpio)
        return PAD_INVALID;
    if(gpio->port >= PAD_INVALID)
        return PAD_INVALID;
    return (Pad_Num_Type)gpio->port;
}
static void gpio_set_input(Pad_Num_Type pad_num)
{
    port_gpio_input_enable(pad_num);
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);

    if(gpio_grp->OUTENSET & (1 << pad_offset))
        gpio_grp->OUTENCLR = (1 << pad_offset);
}
static void gpio_set_output(Pad_Num_Type pad_num)
{
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);

    if(!(gpio_grp->OUTENSET & (1 << pad_offset)))
        gpio_grp->OUTENSET = (1 << pad_offset);
}

static uint8_t gpio_is_output(Pad_Num_Type pad_num)
{
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);

    if(gpio_grp->OUTENSET & (1 << pad_offset))
        return 1;
    else
        return 0;
}
static uint32_t gpio_get_input_value(Pad_Num_Type pad_num)
{
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);

    return (gpio_grp->DATA >> pad_offset) & 1;
}
static uint32_t gpio_get_output_value(Pad_Num_Type pad_num)
{
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);

    return (gpio_grp->DATAOUT >> pad_offset) & 1;
}
static void gpio_set_output_value(Pad_Num_Type pad_num,uint8_t value)
{
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);

    if(value)
    {//high
        gpio_grp->DATAOUT |= (1 << pad_offset);
    }
    else
    {//low
        gpio_grp->DATAOUT &= ~(1 << pad_offset);
    }
}

void GPIO_IRQHandler(void)
{
   // asr_intrpt_enter();
    for(int i = 0; i < ASR_GPIO_TOTAL_NUM; i++)
    {
        GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP((Pad_Num_Type)i);
        uint8_t pad_offset = gpio_get_padoffset((Pad_Num_Type)i);

            //gpio group0 irq
            if(gpio_grp->INTSTATUS & (0x0001 << pad_offset))
            {
                //clear GPIO GROUP0 interrupt
                gpio_grp->INTSTATUS = (0x0001 << pad_offset);
                if(g_asr_gpio_handler[i].cb)
                {
                    g_asr_gpio_handler[i].cb(g_asr_gpio_handler[i].arg);
                }
            }
    }
   // asr_intrpt_exit();
}

void FAST_GPIO_IRQHandler(void)
{
   // asr_sonata_0-15_intrpt_enter();
    for(int i = 0; i < ASR_GPIO_TOTAL_NUM; i++)
    {
        GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP((Pad_Num_Type)i);
        uint8_t pad_offset = gpio_get_padoffset((Pad_Num_Type)i);

            //gpio group0 irq
            if(gpio_grp->INTSTATUS & (0x0001 << pad_offset))
            {
                //clear GPIO GROUP0 interrupt
                gpio_grp->INTSTATUS = (0x0001 << pad_offset);
                if(g_asr_gpio_handler[i].cb)
                {
                    g_asr_gpio_handler[i].cb(g_asr_gpio_handler[i].arg);
                }
            }
    }
    //asr_sonata_0-15_intrpt_exit();
}

void NORMAL_GPIO_IRQHandler(void)
{
    //asr_sonata_16-30_intrpt_enter();
    for(int i = 0; i < ASR_GPIO_TOTAL_NUM; i++)
    {
        GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP((Pad_Num_Type)i);
        uint8_t pad_offset = gpio_get_padoffset((Pad_Num_Type)i);

            //gpio group1 irq
            if(gpio_grp->INTSTATUS & (0x0001 << pad_offset))
            {
                //clear GPIO GROUP1 interrupt
                gpio_grp->INTSTATUS = (0x0001 << pad_offset);
                if(g_asr_gpio_handler[i].cb)
                {
                    g_asr_gpio_handler[i].cb(g_asr_gpio_handler[i].arg);
                }
            }
    }
    //asr_sonata_16-30_intrpt_exit();
}

void GPIO0_IRQHandler(void)
{
   // asr_fugue_0-15_intrpt_enter();
    for(int i = 0; i < ASR_GPIO_TOTAL_NUM; i++)
    {
        GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP((Pad_Num_Type)i);
        uint8_t pad_offset = gpio_get_padoffset((Pad_Num_Type)i);

            //gpio group0 irq
            if(gpio_grp->INTSTATUS & (0x0001 << pad_offset))
            {
                //clear GPIO GROUP0 interrupt
                gpio_grp->INTSTATUS = (0x0001 << pad_offset);
                if(g_asr_gpio_handler[i].cb)
                {
                    g_asr_gpio_handler[i].cb(g_asr_gpio_handler[i].arg);
                }
            }
    }
    //asr_fugue_0-15_intrpt_exit();
}

void GPIO1_IRQHandler(void)
{
   // asr_fugue_16-30_intrpt_enter();
    for(int i = 0; i < ASR_GPIO_TOTAL_NUM; i++)
    {
        GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP((Pad_Num_Type)i);
        uint8_t pad_offset = gpio_get_padoffset((Pad_Num_Type)i);

            //gpio group1 irq
            if(gpio_grp->INTSTATUS & (0x0001 << pad_offset))
            {
                //clear GPIO GROUP1 interrupt
                gpio_grp->INTSTATUS = (0x0001 << pad_offset);
                if(g_asr_gpio_handler[i].cb)
                {
                    g_asr_gpio_handler[i].cb(g_asr_gpio_handler[i].arg);
                }
            }
    }
    //asr_fugue_16-30_intrpt_exit();
}

/**
 * Initialises a GPIO pin
 *
 * @note  Prepares a GPIO pin for use.
 *
 * @param[in]  gpio           the gpio pin which should be initialised
 * @param[in]  configuration  A structure containing the required gpio configuration
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_init(asr_gpio_dev_t *gpio)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    if(pad_num == PAD_INVALID)
    {
        return EIO;
    }
    //pinmux setting
    if(asr_pinmux_config(pad_num,PF_GPIO) == Config_Fail)
    {
        return EIO;
    }
    //enable gpio clk
    port_gpio_init_clk(pad_num);
    //set config
    switch(gpio->config)
    {
        case ASR_ANALOG_MODE:
            break;
        case ASR_INPUT_PULL_UP:
            gpio_set_input(pad_num);
            asr_pad_config(pad_num,PULL_UP);
            break;
        case ASR_IRQ_MODE:
        case ASR_INPUT_PULL_DOWN:
            gpio_set_input(pad_num);
            asr_pad_config(pad_num,PULL_DOWN);
            break;
        case ASR_INPUT_HIGH_IMPEDANCE:
            gpio_set_input(pad_num);
            asr_pad_config(pad_num,PULL_NONE);
            break;
        case ASR_OUTPUT_PUSH_PULL:
        case ASR_OUTPUT_OPEN_DRAIN_NO_PULL:
        case ASR_OUTPUT_OPEN_DRAIN_PULL_UP:
            gpio_set_output(pad_num);
            break;
        default:
            return EIO;
    }
    return 0;
}


/**
 * Sets an output GPIO pin high
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set high
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_output_high(asr_gpio_dev_t *gpio)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    if(pad_num == PAD_INVALID)
        return EIO;

    if(!gpio_is_output(pad_num))
        return EIO;

    gpio_set_output_value(pad_num,1);
    return 0;
}


/**
 * Sets an output GPIO pin low
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set low
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
 int32_t asr_gpio_output_low(asr_gpio_dev_t *gpio)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    if(pad_num == PAD_INVALID)
        return EIO;

    if(!gpio_is_output(pad_num))
        return EIO;

    gpio_set_output_value(pad_num,0);
    return 0;
}


/**
 * Get the state of an output GPIO pin. Using this function on a
 * gpio pin which is set to input mode will return an undefined value.
 *
 * @param[in]  gpio   the gpio pin which should be read
 * @param[in]  value  gpio value
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 *
 * note: defined by asr
 */
int32_t asr_gpio_output_get(asr_gpio_dev_t *gpio, uint32_t *value)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    if(pad_num == PAD_INVALID)
        return EIO;

    if(!gpio_is_output(pad_num))
        return EIO;

    *value = gpio_get_output_value(pad_num);
    return 0;
}


/**
 * Trigger an output GPIO pin's output. Using this function on a
 * gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set low
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_output_toggle(asr_gpio_dev_t *gpio)
{
    uint32_t value;
    if(0 == asr_gpio_output_get(gpio, &value))
    {
        if(value)
        {
            return asr_gpio_output_low(gpio);
        }
        else
        {
            return asr_gpio_output_high(gpio);
        }
    }
    else
    {
        return EIO;
    }
}


/**
 * Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param[in]  gpio   the gpio pin which should be read
 * @param[in]  value  gpio value
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_input_get(asr_gpio_dev_t *gpio, uint32_t *value)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    if(pad_num == PAD_INVALID)
        return EIO;

    if(gpio_is_output(pad_num))
        return EIO;

    *value = gpio_get_input_value(pad_num);
    return 0;
}


/**
 * Enables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which is set to
 * output mode is undefined.
 *
 * @param[in]  gpio     the gpio pin which will provide the interrupt trigger
 * @param[in]  trigger  the type of trigger (rising/falling edge)
 * @param[in]  handler  a function pointer to the interrupt handler
 * @param[in]  arg      an argument that will be passed to the interrupt handler
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_enable_irq(asr_gpio_dev_t *gpio, asr_gpio_irq_trigger_t trigger,
                            asr_gpio_irq_handler_t handler, void *arg)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);
    if(pad_num == PAD_INVALID)
        return EIO;

    if(gpio_is_output(pad_num))
        return EIO;

    g_asr_gpio_handler[pad_num].cb = handler;
    g_asr_gpio_handler[pad_num].arg = arg;
    switch(trigger)
    {
        case ASR_IRQ_TRIGGER_RISING_EDGE:
            gpio_grp->INTTYPESET = (1 << pad_offset); //edge or level trig, 1:For falling edge or rising edge; 0:For LOW or HIGH level
            gpio_grp->INTPOLSET = (1 << pad_offset); //trig polarity,       1:For HIGH level or rising edge; 0:For LOW level or falling edge.
            break;
        case ASR_IRQ_TRIGGER_FALLING_EDGE:
            gpio_grp->INTTYPESET = (1 << pad_offset);
            gpio_grp->INTPOLCLR = (1 << pad_offset);
            break;
        case ASR_IRQ_TRIGGER_HIGH_LEVEL:
            gpio_grp->INTTYPECLR = (1 << pad_offset);
            gpio_grp->INTPOLSET = (1 << pad_offset);
            break;
        case ASR_IRQ_TRIGGER_LOW_LEVEL:
            gpio_grp->INTTYPECLR = (1 << pad_offset);
            gpio_grp->INTPOLCLR = (1 << pad_offset);
            break;
        case ASR_IRQ_TRIGGER_BOTH_EDGES:
            gpio_grp->INTBOTHEDGESET = (1 << pad_offset); //double edge set
            gpio_grp->INTTYPESET = (1 << pad_offset); //edge or level trig
            break;
        default:
            return EIO;
    }
    gpio_grp->INTENSET = (1 << pad_offset); //int enable
    port_gpio_enable_irq(pad_num);
    return 0;
}


/**
 * Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref asr_gpio_input_irq_enable is undefined.
 *
 * @param[in]  gpio  the gpio pin which provided the interrupt trigger
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_disable_irq(asr_gpio_dev_t *gpio)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);
    if(pad_num == PAD_INVALID)
        return EIO;

    g_asr_gpio_handler[pad_num].cb = NULL;
    g_asr_gpio_handler[pad_num].arg = NULL;
    gpio_grp->INTENCLR = (1 << pad_offset);
    gpio_grp->INTTYPECLR = (1 << pad_offset);
    gpio_grp->INTPOLCLR = (1 << pad_offset);
    gpio_grp->INTBOTHEDGECLR = (1 << pad_offset);
    return 0;
}


/**
 * Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref asr_gpio_input_irq_enable is undefined.
 *
 * @param[in]  gpio  the gpio pin which provided the interrupt trigger
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_clear_irq(asr_gpio_dev_t *gpio)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);
    if(pad_num == PAD_INVALID)
        return EIO;

    if(gpio_grp->INTSTATUS & (1 << pad_offset))
    {
        //clear GPIO interrupt status
        gpio_grp->INTSTATUS = (1 << pad_offset);
    }
    return 0;
}


/**
 * Set a GPIO pin in default state.
 *
 * @param[in]  gpio  the gpio pin which should be deinitialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_finalize(asr_gpio_dev_t *gpio)
{
    Pad_Num_Type pad_num = gpio_get_pad_num(gpio);
    GPIO_TypeDef * gpio_grp = gpio_get_GPIOGROUP(pad_num);
    uint8_t pad_offset = gpio_get_padoffset(pad_num);
    if(pad_num == PAD_INVALID)
        return EIO;

    if(gpio_grp->OUTENSET & (1 << pad_offset))
    {
        //clear GPIO output enable bit
        gpio_grp->OUTENCLR = (1 << pad_offset);
    }
    asr_gpio_disable_irq(gpio);
    asr_gpio_clear_irq(gpio);
    return 0;
}
