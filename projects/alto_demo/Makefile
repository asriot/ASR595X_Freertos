TARGET = alto_demo
ASR_SDK_ROOT = ../..

SOC = alto

SRCDIRS = .
INCDIRS = .

RTOS = freertos

LEGA_RTOS = 1
LWIP = 1
AT_CMD = 1
DRIVERS = 1
KV = 1
SYSTEM = 1
WPA3 = 1
SECURITY = 1
BACKTRACE = 1
MULTI_HEAP_REGIONS = 1

# MBEDTLS = 1
# CLOUD_SWITCH=1
# ELOG_SWITCH=1

#ble start
BLE_COEX := 1
BLE_TARGET  := ble_sonata_sdk_demo

ifeq ($(BLE_COEX), 1)
CFLAGS += -DALTO_BLE_NO_BLOCK

ifeq ($(BLE_TARGET),ble_ll_rel)
INCDIRS += $(ASR_SDK_ROOT)/demo/duet_demo/app/ble_demo/hl_peripheral
CFLAGS += -D__VTOR_PRESENT=1 -DCFG_DUET_BLE -DCFG_REL -DSONATA_RTOS_SUPPORT -DCODE_USE_ALTO
ASMFLAGS += -DSTACK_INIT_PATTERN=0xF3F3F3F3
LDFLAGS += -Wl,-R $(ASR_SDK_ROOT)/lib/ble/ble_rom_v10_0_8_asr_symbol.txt -Wl,-z,max-page-size=4096
endif

ifeq ($(BLE_TARGET),ble_sonata_sdk_demo)

C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_sonata_sdk_demo/app.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_sonata_sdk_demo/data_transport_svc.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_sonata_sdk_demo/user_platform.c

INCDIRS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_sonata_sdk_demo

CFLAGS += -D__VTOR_PRESENT=1 -DCFG_DUET_BLE -DCFG_REL -DSONATA_RTOS_SUPPORT -DCODE_USE_ALTO
ASMFLAGS += -DSTACK_INIT_PATTERN=0xF3F3F3F3
LDFLAGS += -Wl,-R $(ASR_SDK_ROOT)/lib/ble/ble_rom_v10_0_8_asr_symbol.txt -Wl,-z,max-page-size=4096
endif

ifeq ($(BLE_TARGET),ble_perf_test_peripheral_demo)

C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_peripheral_demo/app.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_peripheral_demo/data_transport_svc.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_peripheral_demo/user_platform.c

INCDIRS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_peripheral_demo

CFLAGS += -D__VTOR_PRESENT=1 -DCFG_DUET_BLE -DCFG_REL -DSONATA_RTOS_SUPPORT -DCODE_USE_ALTO
ASMFLAGS += -DSTACK_INIT_PATTERN=0xF3F3F3F3
LDFLAGS += -Wl,-R $(ASR_SDK_ROOT)/lib/ble/ble_rom_v10_0_8_asr_symbol.txt -Wl,-z,max-page-size=4096
endif

ifeq ($(BLE_TARGET),ble_perf_test_central_demo)

C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_central_demo/app.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_central_demo/app_uart.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_central_demo/user_platform.c

INCDIRS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_perf_test_central_demo

CFLAGS += -D__VTOR_PRESENT=1 -DCFG_DUET_BLE -DCFG_REL -DSONATA_RTOS_SUPPORT -DCODE_USE_ALTO
ASMFLAGS += -DSTACK_INIT_PATTERN=0xF3F3F3F3
LDFLAGS += -Wl,-R $(ASR_SDK_ROOT)/lib/ble/ble_rom_v10_0_8_asr_symbol.txt -Wl,-z,max-page-size=4096
endif

ifeq ($(BLE_TARGET),ble_mesh_generic_onoff_server_demo)

C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_mesh_generic_onoff_server_demo/app.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_mesh_generic_onoff_server_demo/user_platform.c

INCDIRS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_mesh_generic_onoff_server_demo

CFLAGS += -D__VTOR_PRESENT=1 -DCFG_DUET_BLE -DCFG_REL -DSONATA_RTOS_SUPPORT -DCODE_USE_ALTO
ASMFLAGS += -DSTACK_INIT_PATTERN=0xF3F3F3F3
LDFLAGS += -Wl,-R $(ASR_SDK_ROOT)/lib/ble/ble_rom_v10_0_8_asr_symbol.txt -Wl,-z,max-page-size=4096

LDLIBS += $(ASR_SDK_ROOT)/lib/ble/lib_ble_mesh.a
endif

ifeq ($(BLE_TARGET),ble_mesh_generic_onoff_client_demo)

C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_mesh_generic_onoff_client_demo/app.c
C_SRCS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_mesh_generic_onoff_client_demo/user_platform.c

INCDIRS += $(ASR_SDK_ROOT)/projects/alto_demo/ble_demo/ble_mesh_generic_onoff_client_demo

CFLAGS += -D__VTOR_PRESENT=1 -DCFG_DUET_BLE -DCFG_REL -DSONATA_RTOS_SUPPORT -DCODE_USE_ALTO
ASMFLAGS += -DSTACK_INIT_PATTERN=0xF3F3F3F3
LDFLAGS += -Wl,-R $(ASR_SDK_ROOT)/lib/ble/ble_rom_v10_0_8_asr_symbol.txt -Wl,-z,max-page-size=4096

LDLIBS += $(ASR_SDK_ROOT)/lib/ble/lib_ble_mesh.a
endif
endif
#ble end

COMMON_FLAGS := -g -Os -DPRINTF2_SUPPORT -DCFG_PLF_RV32 -DLEGA_A0V2
CFLAGS += -D_SPI_FLASH_240MHz_ -DDCDC_PFMMODE_CLOSE -DLEGA_A0V2 -D_SPI_FLASH_ENABLE_ -DSYSTEM_SUPPORT_OS -DWIFI_DEVICE -DCFG_BATX=1 -DCFG_BARX=2 -DCFG_REORD_BUF=4 -DCFG_SPC=4 -DCFG_TXDESC0=4 -DCFG_TXDESC1=4 -DCFG_TXDESC2=4 -DCFG_TXDESC3=4 -DCFG_TXDESC4=4 -DCFG_CMON -DCFG_MDM_VER_V30 -DCFG_HE -DCFG_SOFTAP_SUPPORT -DCFG_SNIFFER_SUPPORT -DCFG_CUS_FRAME -DCFG_WF_DBG=1 -D__FPU_PRESENT=1 -DDX_CC_TEE -DHASH_SHA_512_SUPPORTED -DCC_HW_VERSION=0xF0 -DDLLI_MAX_BUFF_SIZE=0x10000 -DSSI_CONFIG_TRNG_MODE=0 -DLWIP_APP_IPERF -DSYSTEM_RECOVERY -DSYSTEM_COREDUMP -DCFG_REC -DCFG_SNIFFER_UNICAST_SUPPORT -DPS_CLOSE_APLL -DCFG_STA_AP_COEX
CFLAGS += -DCFG_SEG -DPRINTF2_SUPPORT -DAT_USER_DEBUG -DCFG_PLF_RV32 -DSECURITY_ENGINE_INIT -DDUET_RF_SLEEP -DLOW_LEVEL_FLASH_RW_SUPPORT -DCFG_RTC_INDEPENDENT -DATCMD_WIFI_SUPPORT -DDHCPS_POOL_SET_SUPPORT
CFLAGS += -DCFG_OPTIMIZE_CHANNEL_SENSITIVITY
CFLAGS += -Wno-address-of-packed-member

ifeq ($(CLOUD_SWITCH), 1)
CFLAGS += -DCLOUD_SUPPORT -DCLOUD_TEST -DCLOUD_CONNECT_TEST -DCLOUD_PRIVATE_SUPPORT -DCLOUD_MQTT_BROKER_SUPPORT -DCLOUD_BAIDU_SUPPORT
CFLAGS += -DCLOUD_AWS_SUPPORT -DCLOUD_HUAWEI_SUPPORT -DCLOUD_JD_SUPPORT -DCLOUD_TENCENT_SUPPORT
MBEDTLS = 1
endif

ifeq ($(ip),v6)
CFLAGS += -DLWIP_DUALSTACK
LDLIBS += $(ASR_SDK_ROOT)/lib/libasr_wifi6_v6.a
else
LDLIBS += $(ASR_SDK_ROOT)/lib/libasr_wifi6.a
endif


ifeq ($(WPA3), 1)
CFLAGS += -DCFG_SAE -DCFG_SAE_AUTH_OPT -DUSE_SMALL_STACK -DCFG_MFP -DCONFIG_IEEE80211W -DWFA_CERTIFICATE_N
endif

# HTTP_DEMO = 1
# HTTPS_DEMO = 1

ifeq ($(HTTP_DEMO), 1)
CFLAGS += -DHTTP_CLIENT_DEMO_SUPPORT
C_SRCS += $(ASR_SDK_ROOT)/lwip/lwip_app/http_demo/http_client_test.c
INCDIRS += $(ASR_SDK_ROOT)/lwip/lwip_app/http_demo
endif

ifeq ($(HTTPS_DEMO), 1)
CFLAGS += -DHTTPS_TLS_DEMO_SUPPORT
C_SRCS += $(ASR_SDK_ROOT)/lwip/lwip_app/https_demo/src/tls_client_test.c\
        $(ASR_SDK_ROOT)/lwip/lwip_app/https_demo/src/mbedtls_net_sock.c
INCDIRS += $(ASR_SDK_ROOT)/lwip/lwip_app/https_demo/include
MBEDTLS = 1
endif

#FLASH_UUID_DEMO = 1
#PSRAM_DEMO = 1

ifeq ($(FLASH_UUID_DEMO), 1)
CFLAGS += -DAT_GET_FLASH_UUID
endif

ifeq ($(PSRAM_DEMO), 1)
CFLAGS += -DAT_PSRAM_TEST
endif

ifeq ($(MULTI_HEAP_REGIONS), 1)
CFLAGS += -DMULTI_HEAP_REGIONS
endif

include $(ASR_SDK_ROOT)/build/Makefile.base
