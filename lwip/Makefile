ifneq ($(wildcard $(LWIP_PATH)),)
include $(LWIP_PATH)/FileLists.mk

INCDIRS += $(addprefix $(LWIP_PATH)/,$(LWIP_INCDIRS))

C_SRCS += $(addprefix $(LWIP_PATH)/,$(LWIP_C_SRCS))

C_SRCDIRS += $(addprefix $(LWIP_PATH)/,$(LWIP_C_SRCDIRS))

COMMON_FLAGS += -DLWIP_APP_IPERF

ifeq ($(CLOUD_SWITCH), 1)

C_SRCS += $(addprefix $(LWIP_PATH)/,$(HTTP_C_SRCS))

C_SRCDIRS += $(addprefix $(LWIP_PATH)/,$(MQTT_C_SRCDIRS))

COMMON_FLAGS += -DCLOUD_SUPPORT

endif

endif