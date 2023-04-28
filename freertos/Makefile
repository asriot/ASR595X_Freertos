ifneq ($(wildcard $(RTOS_PATH)),)
MAKEFILE_PREREQS += $(RTOS_PATH)/Makefile

include $(RTOS_PATH)/FileLists.mk

INCDIRS += $(addprefix $(RTOS_PATH)/,$(RTOS_INCDIRS))

C_SRCS += $(addprefix $(RTOS_PATH)/,$(RTOS_C_SRCS))

C_SRCDIRS += $(addprefix $(RTOS_PATH)/,$(RTOS_C_SRCDIRS))

ASM_SRCDIRS += $(addprefix $(RTOS_PATH)/,$(RTOS_ASM_SRCDIRS))

# Define RTOS_$(RTOS) to show usage of RTOS, such as RTOS_FREERTOS
RTOS_UPPER = $(call uc, $(RTOS))
COMMON_FLAGS += -DRTOS_$(RTOS_UPPER)
endif