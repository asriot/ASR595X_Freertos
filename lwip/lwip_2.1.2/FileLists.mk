export LWIP_2.1.2_FILES_LIST
LWIP_2.1.2 := ../lwip/lwip_2.1.2

EXCEPT_1 := $(LWIP_2.1.2)/apps
EXCEPT_2 := $(LWIP_2.1.2)/netif

EXCEPT_FILES := $(LWIP_2.1.2)/port/perf.c \

LWIP_2.1.2_DIR := $(shell find $(LWIP_2.1.2) \( -path $(EXCEPT_1) -o -path $(EXCEPT_2) \) -prune -o -type d -print)

LWIP_2.1.2_FILES_TMP := $(foreach dir, $(LWIP_2.1.2_DIR), $(wildcard $(dir)/*.c))
LWIP_2.1.2_FILES_TMP += $(EXCEPT_2)/ethernet.c $(wildcard $(EXCEPT_1)/coap/*.c)
LWIP_2.1.2_FILES_TMP += $(EXCEPT_1)/mqtt/mqtt.c $(EXCEPT_1)/http/http_client.c
LWIP_2.1.2_FILES_LIST := $(filter-out $(EXCEPT_FILES),$(LWIP_2.1.2_FILES_TMP))
