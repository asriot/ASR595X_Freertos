PERIPHERAL_INCDIRS = inc inc/$(SOC)

ifeq ($(DRIVERS),1)
PERIPHERAL_C_SRCDIRS = src/$(SOC)

PERIPHERAL_C_SRCS = asr_dma.c \
                    asr_efuse.c \
                    asr_flash_alg.c \
                    asr_flash.c \
                    asr_gpio.c \
                    asr_i2c.c \
                    asr_i2s.c \
                    asr_pinmux.c \
                    asr_psram.c \
                    asr_rf_spi.c \
                    asr_rtc.c \
                    asr_timer.c \
                    asr_uart.c \
                    asr_pwm.c \
                    asr_wdg.c
endif
#to fix me
EXCLUDE_SRCS += $(ASR_SDK_PERIPHERAL)/src/alto/hal_alto_ram_layout.c
#EXCLUDE_SRCS += $(ASR_SDK_PERIPHERAL)/src/asr_rtc.c
