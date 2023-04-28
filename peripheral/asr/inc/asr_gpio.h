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
#ifndef _ASR_GPIO_H_
#define _ASR_GPIO_H_
#include "asr_port.h"
#include "asr_common.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASR_GPIO_NUM_PER_GROUP 16
#define ASR_GPIO_PAD_NUM  PAD_INVALID
/*
    CANON definitons
*/
typedef struct __GPIO
{
    __IO uint32_t DATA;
    __IO uint32_t DATAOUT;
    __IO uint32_t INTBOTHEDGESET;
    __IO uint32_t INTBOTHEDGECLR;
    __IO uint32_t OUTENSET;
    __IO uint32_t OUTENCLR;
    __I  uint32_t DUMMY1[2];
    __IO uint32_t INTENSET;
    __IO uint32_t INTENCLR;
    __IO uint32_t INTTYPESET;
    __IO uint32_t INTTYPECLR;
    __IO uint32_t INTPOLSET;
    __IO uint32_t INTPOLCLR;
    __IO uint32_t INTSTATUS;
} GPIO_TypeDef;
#define GPIO_GROUP0 ((GPIO_TypeDef *)(GPIO_GROUP0_REG_BASE))
#define GPIO_GROUP1 ((GPIO_TypeDef *)(GPIO_GROUP1_REG_BASE))

/*
 * Pin configuration
 */
typedef enum {
    ASR_ANALOG_MODE,               /* Used as a function pin, input and output analog */
    ASR_IRQ_MODE,                  /* Used to trigger interrupt */
    ASR_INPUT_PULL_UP,             /* Input with an internal pull-up resistor - use with devices
                                  that actively drive the signal low - e.g. button connected to ground */
    ASR_INPUT_PULL_DOWN,           /* Input with an internal pull-down resistor - use with devices
                                  that actively drive the signal high - e.g. button connected to a power rail */
    ASR_INPUT_HIGH_IMPEDANCE,      /* Input - must always be driven, either actively or by an external pullup resistor */
    ASR_OUTPUT_PUSH_PULL,          /* Output actively driven high and actively driven low -
                                  must not be connected to other active outputs - e.g. LED output */
    ASR_OUTPUT_OPEN_DRAIN_NO_PULL, /* Output actively driven low but is high-impedance when set high -
                                  can be connected to other open-drain/open-collector outputs.
                                  Needs an external pull-up resistor */
    ASR_OUTPUT_OPEN_DRAIN_PULL_UP, /* Output actively driven low and is pulled high
                                  with an internal resistor when set high -
                                  can be connected to other open-drain/open-collector outputs. */
} asr_gpio_config_t;

/*
 * GPIO dev struct
 */
typedef struct {
    uint8_t       port;    /* gpio port */
    asr_gpio_config_t config;  /* gpio config */
    void         *priv;    /* priv data */
} asr_gpio_dev_t;

/*
 * GPIO interrupt trigger
 */
typedef enum {
    ASR_IRQ_TRIGGER_RISING_EDGE  = 0x1, /* Interrupt triggered at input signal's rising edge  */
    ASR_IRQ_TRIGGER_FALLING_EDGE = 0x2, /* Interrupt triggered at input signal's falling edge */
    ASR_IRQ_TRIGGER_HIGH_LEVEL   = 0x3, /* Interrupt triggered at input signal's high level   */
    ASR_IRQ_TRIGGER_LOW_LEVEL    = 0x4, /* Interrupt triggered at input signal's low level    */
    ASR_IRQ_TRIGGER_BOTH_EDGES   = 0x5, /* not support */
} asr_gpio_irq_trigger_t;

/*
 * GPIO interrupt callback handler
 */
typedef void (*asr_gpio_irq_handler_t)(void *arg);

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
int32_t asr_gpio_init(asr_gpio_dev_t *gpio);

/**
 * Sets an output GPIO pin high
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set high
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_output_high(asr_gpio_dev_t *gpio);

/**
 * Sets an output GPIO pin low
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set low
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_output_low(asr_gpio_dev_t *gpio);

/**
 * Trigger an output GPIO pin's output. Using this function on a
 * gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set low
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_output_toggle(asr_gpio_dev_t *gpio);

/**
 * Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param[in]  gpio   the gpio pin which should be read
 * @param[in]  value  gpio value
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_input_get(asr_gpio_dev_t *gpio, uint32_t *value);

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
                            asr_gpio_irq_handler_t handler, void *arg);

/**
 * Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref asr_gpio_input_irq_enable is undefined.
 *
 * @param[in]  gpio  the gpio pin which provided the interrupt trigger
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_disable_irq(asr_gpio_dev_t *gpio);

/**
 * Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref asr_gpio_input_irq_enable is undefined.
 *
 * @param[in]  gpio  the gpio pin which provided the interrupt trigger
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_clear_irq(asr_gpio_dev_t *gpio);

/**
 * Set a GPIO pin in default state.
 *
 * @param[in]  gpio  the gpio pin which should be deinitialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_gpio_finalize(asr_gpio_dev_t *gpio);

/* Add other asr api here*/

typedef struct {
    asr_gpio_irq_handler_t cb;
    void *arg;
} asr_gpio_cb_t;

#ifdef __cplusplus
}
#endif

#endif