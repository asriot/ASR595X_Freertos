include $(ASR_SDK_PERIPHERAL)/FileLists.mk
INCDIRS += $(addprefix $(ASR_SDK_PERIPHERAL)/,$(PERIPHERAL_INCDIRS))
INCDIRS += $(addprefix $(ASR_SDK_PERIPHERAL)/,$(PERIPHERAL_INCDIRS))/$(SOC)
ifeq ($(DRIVERS),1)
C_SRCDIRS += $(addprefix $(ASR_SDK_PERIPHERAL)/,$(PERIPHERAL_C_SRCDIRS))
C_SRCS += $(addprefix $(ASR_SDK_PERIPHERAL)/src/,$(PERIPHERAL_C_SRCS))
else
############### add the depended soc files ###############
PERIPHERAL_C_SRCS += $(SOC)/asr_$(SOC)_port.c
PERIPHERAL_C_SRCS += asr_flash_alg.c

ifneq ($(filter asr_pinmux.c, $(PERIPHERAL_C_SRCS)),)
PERIPHERAL_C_SRCS += $(SOC)/asr_$(SOC)_pinmux.c
endif

ifneq ($(filter asr_flash.c, $(PERIPHERAL_C_SRCS)),)
PERIPHERAL_C_SRCS += asr_flash_alg.c
PERIPHERAL_C_SRCS += $(SOC)/asr_$(SOC)_board.c
endif

ifneq ($(filter asr_i2s.c, $(PERIPHERAL_C_SRCS)),)
PERIPHERAL_C_SRCS += asr_rf_spi.c
endif
############### end the depended soc files ###############

C_SRCS += $(addprefix $(ASR_SDK_PERIPHERAL)/src/,$(PERIPHERAL_C_SRCS))
endif