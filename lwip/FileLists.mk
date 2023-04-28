LWIP_C_SRCS += lwip_2.1.2/netif/ethernet.c \
        lwip_2.1.2/port/sys_arch.c \
        lwip_app/iperf/lwip_iperf.c \
        lwip_app/ota/http_ota.c \
        lwip_app/lwip_app_ping/lwip_app_ping.c

LWIP_C_SRCDIRS += lwip_2.1.2/api \
        lwip_2.1.2/core \
        lwip_2.1.2/core/ipv4 \
        lwip_2.1.2/core/ipv6 \

LWIP_INCDIRS += lwip_2.1.2/include \
        lwip_2.1.2/port/include \
        lwip_2.1.2/include/lwip \
        lwip_app \
        port/dhcps \
        lwip_app/lwip_app_ping \
        lwip_2.1.2/include/lwip/prot \
        lwip_2.1.2/include/netif \
        lwip_app/iperf \
        lwip_app/ota

ifeq ($(MATTER_SWITCH), 1)
LWIP_INCDIRS += ../middleware/matter/matterapi/config/lwip_if \
        ../middleware/matter/matterapi/config/lwip_if/arch
else
LWIP_INCDIRS += lwip_2.1.2/include/lwip_if \
        lwip_2.1.2/include/lwip_if/arch
endif

MQTT_C_SRCDIRS += lwip_2.1.2/apps/altcp_tls \
        lwip_2.1.2/apps/mqtt

HTTP_C_SRCS += lwip_2.1.2/apps/http/http_client.c