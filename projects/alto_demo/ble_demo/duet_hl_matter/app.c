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
/**
 ****************************************************************************************
 *
 * @file app.c
 *
 * @brief Application entry point
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arch.h" // Application Definition
#include "sonata_ble_api.h"
#include <stdio.h>
#if !defined ALIOS_SUPPORT && !defined HARMONYOS_SUPPORT
#include "user_platform.h"
#endif
#include "sonata_ble_hook.h"
#include "sonata_gap_api.h"
#include "sonata_gatt_api.h"
#include "sonata_log.h"
#include "sonata_utils_api.h"
#if !defined ALIOS_SUPPORT && !defined HARMONYOS_SUPPORT
//#include "at_api.h"
#endif

#include "app.h"
#if BLE_BATT_SERVER
#include "sonata_prf_bass_api.h"
#endif // BLE_BATT_SERVER
#ifdef SONATA_RTOS_SUPPORT
#include "lega_rtos_api.h"
#endif
#include "sonata_stack_user_config.h"
#if CONFIG_ENABLE_ASR_APP_MESH
#include "sonata_mesh_api.h"

#define MESH_OPCODE_VENDOR_OO_SET_UNACK (0x000917d1)
#define MESH_OPCODE_VENDOR_SET (0x000917d2)
#define MESH_OPCODE_VENDOR_STATUS (0x000917d3)
#define REPLAY_NUMBER_ELEMENT 90;

#define APP_RESET_TIMER_INTERVAL (6000) // unit:ms
#define APP_TIMER_INTERVAL (1000)       // unit:ms
#define APP_ATTATION_TIMER (3000)       // unit:ms
#define APP_RESET_SWITCH_COUNT (6)
#define APP_RESET_SAVE_TAG (APP_DATA_SAVE_TAG_FIRST + 0)
#define APP_RESET_SAVE_LEN (1)
#define APP_UUID_MAC_OFFSET 7

#define APP_MSG_RESET_TIMER 20
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef struct
{
    uint8_t cmd;
    uint8_t msg[4];
} vendor_onoff_rx_no_ack_t;

extern void sonata_mesh_api_init(void);
static void app_clean_data_cb(void);
static mesh_prov_param_t init_prov_param = {
     .uuid  =  {
     //   0xA8, 0x01, //CID AliBaBa
     //   0xD1, //PID
     //   0x8A, 0x25, 0x00, 0x00, //ProductID: 9610
     //   0x57, 0x5D, 0x69, 0x63, 0xA7, 0xF8, //MAC:f8a763695d57
     //   0x00, //FeatureFlag
     //   0x00, 0x00//RFU
        0x17,0x09,0x1A,0x00,0x02,0x02,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x00,0x00,0x00 },
     .p_uri = NULL,
     .static_oob = 0,
    .static_oob_len = 0,
    .oob_info = 0,
    .output_size = 0,
    .output_actions = 0,
    .input_size = 0,
    .input_actions = 0,
    .cid = 0x0917,
    .pid = 0x001A,
    .vid = 0x0001,
    .loc=  0x0100,
};

static uint8_t static_auth_val[MESH_PROVISION_AUTH_MAX_NUM] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static uint32_t mesh_feature                                = MESH_FEAT_RELAY_NODE_SUP | MESH_FEAT_PROXY_NODE_SUP;
static uint8_t irq                                          = 0;

static uint16_t onoff_msg_count                     = 0;
static uint16_t vendor_msg_count                    = 0;
static mesh_provisioner_netkey_t provisioner_netkey = {
    .addOrDel = true,
    //.netkey = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x00 },
    .netkey = { 0xf3, 0xcd, 0x55, 0xb4, 0xdf, 0x23, 0x1d, 0xa3, 0xdd, 0x4e, 0x32, 0xa8, 0x08, 0x26, 0x0b, 0xe4 },

    .key_index = 0,
};

static mesh_provisioner_appkey_t provisioner_appkey = {
    .addOrDel = true,
    //.appkey = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x00 },
    .appkey = { 0x7a, 0x3c, 0xa7, 0x1a, 0x98, 0xa4, 0x70, 0x1d, 0x94, 0x17, 0xc2, 0x1b, 0xec, 0x2e, 0xc7, 0x0c },

    .key_index    = 0,
    .netkey_index = 0,
};
typedef enum
{
    APP_MESH_SELF_PROV_DEFAULT,
    APP_MESH_SELF_PROV_ONOFF_BIND,
    APP_MESH_SELF_PROV_SET_VENDOR_SUB_A,
    APP_MESH_SELF_PROV_SET_VENDOR_SUB_B,
    APP_MESH_SELF_PROV_SET_VENDOR_SUB_C,
    APP_MESH_SELF_PROV_SET_ONOFF_SUB_A,
    APP_MESH_SELF_PROV_SET_ONOFF_SUB_B,
    APP_MESH_SELF_PROV_SET_ONOFF_SUB_C,
} app_self_prov_state_t;

static uint8_t next_state = APP_MESH_SELF_PROV_DEFAULT;

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
static bool first_provsioned = false;
uint16_t self_addr           = 0;
void app_set_set_prov_data(uint16_t addr)
{
    if (addr == 0)
    {
        printf("addr unassigned\r\n");
        return;
    }
    first_provsioned = true;
    self_addr        = addr;
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    mesh_core_evt_cb_params_t set_param;
    set_param.local_prov_data_set.iv           = 0;
    set_param.local_prov_data_set.key_index    = 0;
    set_param.local_prov_data_set.flags        = 0;
    set_param.local_prov_data_set.unicast_addr = addr;
    memmove(set_param.local_prov_data_set.netkey, provisioner_netkey.netkey, MESH_KEYS_LEN);
    mesh_core_evt_cb(MESH_SET_LOCAL_PROV_DATA, &set_param);
}

void app_set_appkey_param(void)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    mesh_core_evt_cb_params_t set_param;
    set_param.local_appkey_set.addOrDel     = 1;
    set_param.local_appkey_set.key_index    = provisioner_appkey.key_index;
    set_param.local_appkey_set.netkey_index = provisioner_appkey.netkey_index;
    memmove(set_param.local_appkey_set.appkey, provisioner_appkey.appkey, MESH_KEYS_LEN);
    mesh_core_evt_cb(MESH_SET_LOCAL_APPKEY, &set_param);
}

void app_set_model_app_bind_vendor(void)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    next_state = APP_MESH_SELF_PROV_ONOFF_BIND;
    mesh_core_evt_cb_params_t set_param;
    set_param.local_model_app_bind.element_addr = sonata_mesh_get_prim_addr();
    set_param.local_model_app_bind.appkey_index = provisioner_appkey.key_index;
    set_param.local_model_app_bind.model_id     = MESH_MODELID_VENS_ASR;
    mesh_core_evt_cb(MESH_SET_LOCAL_MODEL_APP_BIND, &set_param);
}

void app_set_model_app_bind_onoff(void)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    next_state = APP_MESH_SELF_PROV_DEFAULT;
    mesh_core_evt_cb_params_t set_param;
    set_param.local_model_app_bind.element_addr = sonata_mesh_get_prim_addr();
    set_param.local_model_app_bind.appkey_index = provisioner_appkey.key_index;
    set_param.local_model_app_bind.model_id     = MESH_MODELID_GENC_OO;
    mesh_core_evt_cb(MESH_SET_LOCAL_MODEL_APP_BIND, &set_param);
}

void app_add_subs_param(uint32_t model_id, uint16_t sub_addr)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    if (next_state == APP_MESH_SELF_PROV_SET_ONOFF_SUB_C)
    {
        next_state = APP_MESH_SELF_PROV_DEFAULT;
    }
    else
    {
        next_state = next_state + 1;
    }

    mesh_core_evt_cb_params_t set_param;
    set_param.local_sub_set.element_addr = sonata_mesh_get_prim_addr();
    set_param.local_sub_set.sub_addr     = sub_addr;
    set_param.local_sub_set.model_id     = model_id;
    set_param.local_sub_set.addOrDel     = 1;
    mesh_core_evt_cb(MESH_SET_LOCAL_SUBS_PARAM, &set_param);
}

void app_remote_add_subs_param(uint32_t model_id, uint16_t sub_addr)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    mesh_core_evt_cb_params_t set_param;
    set_param.local_sub_set.element_addr = sonata_mesh_get_prim_addr();
    set_param.local_sub_set.sub_addr     = sub_addr;
    set_param.local_sub_set.model_id     = model_id;
    set_param.local_sub_set.addOrDel     = 1;
    mesh_core_evt_cb(MESH_SET_LOCAL_SUBS_PARAM, &set_param);
}
void app_remote_del_subs_param(uint32_t model_id, uint16_t sub_addr)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    mesh_core_evt_cb_params_t set_param;
    set_param.local_sub_set.element_addr = sonata_mesh_get_prim_addr();
    set_param.local_sub_set.sub_addr     = sub_addr;
    set_param.local_sub_set.model_id     = model_id;
    set_param.local_sub_set.addOrDel     = 0;
    mesh_core_evt_cb(MESH_SET_LOCAL_SUBS_PARAM, &set_param);
}
void app_set_pub_param(uint16_t dst, uint8_t ttl, uint8_t count, uint8_t interval)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    mesh_core_evt_cb_params_t set_param;
    set_param.local_pub_set.element_addr            = sonata_mesh_get_prim_addr();
    set_param.local_pub_set.pub_addr                = dst;
    set_param.local_pub_set.appkey_index            = provisioner_appkey.key_index;
    set_param.local_pub_set.credentialFlag          = 0;
    set_param.local_pub_set.ttl                     = ttl;
    set_param.local_pub_set.period                  = 0;
    set_param.local_pub_set.retransmitCount         = count;
    set_param.local_pub_set.retransmitIntervalSteps = interval;
    set_param.local_pub_set.model_id                = MESH_MODELID_GENC_OO;
    mesh_core_evt_cb(MESH_SET_LOCAL_PUBLISH_PARAM, &set_param);
}

void app_mesh_self_config(void)
{
    if (!sonata_mesh_is_provisioned())
    {
        uint16_t addr = (uint16_t) ((*(sonata_get_bt_address() + 4)) << 8) + *(sonata_get_bt_address() + 5);
        app_set_set_prov_data(addr);
    }
}
uint16_t app_get_self_sub_addr()
{
    return 0xC000 + sonata_mesh_get_prim_addr();
}
uint16_t app_get_group_sub_addr(uint8_t div)
{
    return 0xC100 + (sonata_mesh_get_prim_addr() - 1) / div;
}

STATUS mesh_core_evt_ind(mesh_core_evt_ind_t evt, mesh_core_evt_ind_params_t * p_param)
{

    switch (evt)
    {
    case (MESH_PUBLIC_KEY_REQUEST): {
    }
    break;
    case MESH_AUTH_OOB_IND: {
        sonata_mesh_send_prov_auth_data_rsp(true, MESH_PROVISION_AUTH_MAX_NUM, static_auth_val);
    }
    break;
    case MESH_ATTATION_IND: {
    }
    break;
    case MESH_APPKEY_UPDATE: {
        APP_TRC("MESH_APPKEY_UPDATE\r\n");
        APP_TRC("appkey:0x");
        for (int i = 0; i < 16; i++)
        {
            APP_TRC("%02x", p_param->appkey_update_ind.key[i]);
        }
        APP_TRC("\r\n");
    }
    break;
    case MESH_NETKEY_UPDATE: {
        APP_TRC("MESH_NETKEY_UPDATE\r\n");
        APP_TRC("netkey:0x");
        for (int i = 0; i < 16; i++)
        {
            APP_TRC("%02x", p_param->netkey_updata_ind.key[i]);
        }
        APP_TRC("\r\n");
    }
    break;
    case MESH_SELF_ADDR_IND: {
        APP_TRC("MESH_SELF_ADDR_IND %d\r\n", p_param->self_addr_ind.addr);
    }
    break;
    case MESH_SELF_DEV_KEY_IND: {
        APP_TRC("MESH_SELF_DEV_KEY_IND\r\n");
        APP_TRC("devkey:0x");
        for (int i = 0; i < 16; i++)
        {
            APP_TRC("%02x", p_param->dev_key.key[i]);
        }
        APP_TRC("\r\n");
    }
    break;
    case MESH_SELF_IV_SEQ_IND: {
        APP_TRC("MESH_SELF_IV_SEQ_IND\r\n");
        APP_TRC("IV %ld,SEQ %ld\r\n", p_param->iv_seq.iv, p_param->iv_seq.seq);
    }
    break;
    case MESH_SUBS_UPDATE: {
        APP_TRC("MESH_SUBS_UPDATE\r\n");
        APP_TRC("element_addr%u, modelid 0x%x, sub_addr 0x%x\r\n  ", p_param->subs_ind.element_addr,
                (unsigned int) p_param->subs_ind.model_id, p_param->subs_ind.addr);
    }
    break;
    case MESH_PUB_UPDATE: {
        APP_TRC("MESH_PUB_UPDATE\r\n");
        APP_TRC("element_addr%u, modelid 0x%x, pub_addr 0x%x\r\n  ", p_param->pub_ind.element_addr,
                (unsigned int) p_param->pub_ind.model_id, p_param->pub_ind.addr);
    }
    break;

    case MESH_APPKEY_BINDED: {
    }
    break;
    case MESH_GRPADDR_SUBSED: {
    }
    break;
    case MESH_PROVISION_STATE: {
        APP_TRC("MESH_PROVISION_STATE %d\r\n", p_param->prov_state_ind.state);
        if (p_param->prov_state_ind.state) // true means be provisioned success
        {
            sonata_mesh_save_data();
            if (first_provsioned)
            {
                sonata_mesh_proxy_service_broadcast(MESH_PROXY_ADV_STOP);
            }
        }
    }
    break;
    case MESH_APPKEY_BINDING: {
        APP_TRC("MESH_APPKEY_BINDING add %u :element %u, modelid 0x%x, index %u \r\n  ", p_param->appkey_bind.bind,
                p_param->appkey_bind.element, (unsigned int) p_param->appkey_bind.modelid,
                p_param->appkey_bind.appkey_global_index);
        sonata_mesh_save_data();
    }
    break;
    case MESH_PROV_DATA_SET_CMP: {
        APP_TRC("APP: %s ,evt=MESH_PROV_DATA_SET_CMP\r\n", __FUNCTION__);
        app_set_appkey_param();
    }
    break;
    case MESH_APPKEY_SET_CMP: {
        APP_TRC("APP: %s ,evt=MESH_APPKEY_SET_CMP\r\n", __FUNCTION__);
        app_set_model_app_bind_vendor();
    }
    break;
    case MESH_MODEL_APP_BIND_CMP: {
        APP_TRC("APP: %s ,evt=MESH_MODEL_APP_BIND_CMP,next_state=%d\r\n", __FUNCTION__, next_state);
        if (next_state == APP_MESH_SELF_PROV_ONOFF_BIND)
        {
            app_set_model_app_bind_onoff();
        }
    }
    break;

    default:
        break;
    }
    return MESH_ERROR_NO_ERROR;
}

void gens_onoff_cb(mesh_state_ind_t * p_state)
{
    if (p_state->type != MESH_STATE_GEN_ONOFF)
    {
        return;
    }
    onoff_msg_count++;

    if (p_state->state)
    {
        printf("sever on\n");
        // turn_on_led();
    }
    else
    {
        printf("sever off\n");
        // turn_off_led();
    }
}
void app_mesh_set_network_param(uint8_t count, uint8_t interval)
{
    APP_TRC("APP: %s count:%d interval:%d \r\n", __FUNCTION__, count, interval);

    mesh_set_params_t set_param;
    set_param.network_transmit.count    = count;
    set_param.network_transmit.interval = interval;
    sonata_mesh_param_set(MESH_NETWORK_TRANSMIT, &set_param);
}

void app_mesh_set_relay_param(uint8_t state, uint8_t count, uint8_t interval)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    mesh_set_params_t set_param;
    set_param.relay_state.state    = state;
    set_param.relay_state.count    = count;
    set_param.relay_state.interval = interval;
    sonata_mesh_param_set(MESH_RELAY_STATE, &set_param);
}

void vens_onoff_cb(mesh_api_model_msg_rx_t * msg_rx)
{
    if ((msg_rx->opcode != MESH_OPCODE_VENDOR_OO_SET_UNACK) && (msg_rx->opcode != MESH_OPCODE_VENDOR_SET) &&
        (msg_rx->opcode != MESH_OPCODE_VENDOR_STATUS))
    {
        return;
    }
}
void get_onoff_rx_number(void)
{
    printf("sever onoff_msg_rx_count:%d\n", onoff_msg_count);
}
void get_vendor_rx_number(void)
{
    printf("sever_vendor msg_rx_count:%d\n", vendor_msg_count);
}

STATUS publish_msg_sent(mesh_model_publish_param_t * p_param, STATUS status)
{
    irq = 0;
    sonata_api_free(p_param);
    return API_SUCCESS;
}
static uint8_t tid_onoff = 0;

CRITICAL_FUNC_SEG void app_mesh_pub_onoff_msg(uint8_t onoff)
{
    if (irq == 1)
    {
        return;
    }
    irq = 1;
    mesh_model_publish_param_t * msg_param =
        sonata_api_malloc(sizeof(mesh_model_publish_param_t) + MESH_MODELID_GEN_OO_SET_MIN_LEN);
    msg_param->element      = 0;
    msg_param->modelid      = MESH_MODELID_GENC_OO;
    msg_param->opcode       = 0x0382;
    msg_param->len          = MESH_MODELID_GEN_OO_SET_MIN_LEN;
    msg_param->trans_mic_64 = 0;
    util_write16p(msg_param->msg + MESH_MODEL_GEN_OO_SET_OO_POS, onoff);
    *(msg_param->msg + MESH_MODEL_GEN_OO_SET_TID_POS) = tid_onoff++;
    mesh_msg_publish(msg_param, publish_msg_sent);
}

void app_model_msg_sent_cb(mesh_model_msg_param_t * p_param, STATUS status)
{
    sonata_api_free(p_param);
}

void app_model_msg_onoff_sent_cb(mesh_model_msg_param_t * p_param, STATUS status)
{
    sonata_api_free(p_param);
}

void app_mesh_send_onoff_packet(uint16_t dst, uint8_t ttl, uint8_t * data)
{
    uint8_t onoff                      = 0;
    onoff                              = data[0];
    mesh_model_msg_param_t * msg_param = sonata_api_malloc(sizeof(mesh_model_msg_param_t) + MESH_MODELID_GEN_OO_SET_MIN_LEN);
    msg_param->opcode                  = 0x0382;
    msg_param->appkey_global_index     = 0;
    msg_param->dst                     = dst;
    msg_param->element                 = 0;
    msg_param->modelid                 = MESH_MODELID_GENC_OO;
    msg_param->mic_64                  = 0;
    msg_param->ttl                     = ttl;
    util_write16p(msg_param->msg + MESH_MODEL_GEN_OO_SET_OO_POS, onoff);
    *(msg_param->msg + MESH_MODEL_GEN_OO_SET_TID_POS) = tid_onoff++;
    msg_param->msg_len                                = MESH_MODELID_GEN_OO_SET_MIN_LEN;
    sonata_mesh_msg_send(msg_param, app_model_msg_onoff_sent_cb);
}

void app_mesh_send_packet(uint16_t dst, uint8_t ttl, uint8_t * data)
{
    mesh_model_msg_param_t * msg_param = sonata_api_malloc(sizeof(mesh_model_msg_param_t) + 5);
    msg_param->opcode                  = MESH_OPCODE_VENDOR_OO_SET_UNACK;
    msg_param->appkey_global_index     = 0;
    msg_param->dst                     = dst;
    msg_param->element                 = 0;
    msg_param->modelid                 = MESH_MODELID_VENS_ASR;
    msg_param->mic_64                  = 0;
    msg_param->ttl                     = ttl;
    memmove(msg_param->msg, data, 5);
    msg_param->msg_len = 5;
    sonata_mesh_msg_send(msg_param, app_model_msg_sent_cb);
}

void app_mesh_control_fan(uint8_t on_off)
{
    app_set_pub_param(0xc003, 5, 1, 1);
    app_mesh_pub_onoff_msg(on_off);
}
void app_mesh_control_light(uint8_t on_off)
{
    app_set_pub_param(0xc002, 5, 1, 1);
    app_mesh_pub_onoff_msg(on_off);
}

void started_cb(void)
{
    // user_load_data(); // user

    if (!sonata_mesh_is_provisioned())
    {
        app_set_set_prov_data(1);
        APP_TRC("auto mesh_provision_service_broadcast\r\n");
    }
    else
    {

        APP_TRC("sonata_mesh_proxy_service_broadcast\r\n");
        // sonata_mesh_proxy_service_broadcast(MESH_PROXY_ADV_START_NET);
    }
}

static void app_ble_on_callback(uint16_t status)
{
    APP_TRC("bluetooth on: 0x%X\r\n", status);
    mesh_set_params_t set_param;
    sonata_mesh_register_core_evt_ind(mesh_core_evt_ind);
    set_param.mesh_role.role = MESH_PROV_ROLE_DEVICE;
    sonata_mesh_param_set(MESH_ROLE, &set_param);

    set_param.addr_type.type = MESH_PRIVATE_STATIC_ADDR;
    sonata_mesh_param_set(MESH_ADDR_TYPE, &set_param);
    set_param.feature_param.feature = mesh_feature;
    sonata_mesh_param_set(MESH_FEATURE_SUPPORT, &set_param);

    // for mesh multi-node test
    mesh_get_params_t get_param;
    sonata_mesh_param_get(MESH_SYSTEM_ADDR, &get_param);
    memmove(init_prov_param.uuid + APP_UUID_MAC_OFFSET, get_param.addr.bd_addr, MESH_ADDR_LEN);
    set_param.prov_param = init_prov_param;
    sonata_mesh_param_set(MESH_PROV_PARAM, &set_param);

    set_param.replay_number = REPLAY_NUMBER_ELEMENT;
    sonata_mesh_param_set(MESH_NUMBER_REPLAY, &set_param);

    set_param.nb_addr_msg_replay = 50;
    sonata_mesh_param_set(MESH_MSG_NUMBER_REPLAY, &set_param);

    sonata_mesh_model_register(MESH_MODELID_GENC_OO, 0, 1, gens_onoff_cb);
    mesh_vendor_model_register(MESH_MODELID_VENS_ASR, 0, vens_onoff_cb);
    sonata_mesh_start(started_cb);
}
void app_clean_prov_data(void)
{
    sonata_mesh_clean_data(app_clean_data_cb);
}

static void app_clean_data_cb(void)
{
    platform_reset(0);
}
#endif
/*
 * DEFINES
 ****************************************************************************************
 */

#define APP_BLE_ADV_ON (1)
#define APP_BLE_ADV_OFF (0)

#define APP_BLE_SCAN_ON (1)
#define APP_BLE_SCAN_OFF (0)
#define APP_BLE_DEVICE_NAME_LEN (255)
#define APP_BLE_CONNECT_MAX (10)
#define APP_BLE_INVALID_CONIDX (0xFF)
#define APP_BLE_ADV_MAX (0xB)
#define APP_BLE_INVALID_ADVIDX (0xFF)
#define APP_MAX_SERVICE_NUM (10)
#define APP_BLE_INIT_INVALID_IDX (0xFF)
#define APP_BLE_MAX_MTU_SIZE (512)
#define APP_BLE_FUN_ERROR (0xFE)

////#if (defined(CFG_PTA_TEST) || defined(CFG_BLE_WVT_ON))
//#define APP_WVT_ENABLE    1
////#else
////#define APP_WVT_ENABLE    0
////#endif

#define BLE_MCRC_MIN_INTERVAL 192
#define MS_BLE_CHANNEL_NUM 7

typedef struct ble_gatt_att_reg_list
{
    uint16_t start_hdl;
    uint8_t nb_att;
    uint8_t state;
    uint8_t perm;
    uint8_t uuid[SONATA_ATT_UUID_128_LEN];
    ble_gatt_att_opr_t * att_opr_list;
    sonata_gatt_att_desc_t * att_desc;
    uint8_t vendor;
} ble_gatt_att_reg_list_t;

typedef struct ble_gatt_att_manager
{
    uint8_t reg_nb;
    uint8_t add_nb;
    ble_gatt_att_reg_list_t * reg_list[APP_MAX_SERVICE_NUM];
} ble_gatt_att_manager_t;

typedef struct app_env_t
{

    uint8_t gAppStatus;
    uint16_t attrHandle;
    uint16_t targetWriteHandle;
    uint16_t targetReadHandle;
    uint16_t targetNtfHandle;
    app_uuids appUuids; // save user's input uuid
} app_env;
static app_env gAppEnv;

/*!
 * @brief save local handle start index
 */
// Mark for profile dis
static uint8_t app_max_adv_evt                = 0;
static uint8_t current_adv_idx                = 0;
uint16_t adv_length                           = 5;
uint8_t ble_adv_set_state[APP_MAX_ADV_IDX]    = { APP_BLE_ADV_OFF, APP_BLE_ADV_OFF, APP_BLE_ADV_OFF };
uint8_t ble_adv_idx_table[APP_MAX_ADV_IDX]    = { 0xff, 0xff, 0xff };
uint8_t ble_adv_change_state[APP_MAX_ADV_IDX] = { 0, 0, 0 };

uint8_t app_connected_state[7] = { APP_STATE_DISCONNECTED, APP_STATE_DISCONNECTED, APP_STATE_DISCONNECTED, APP_STATE_DISCONNECTED,
                                   APP_STATE_DISCONNECTED, APP_STATE_DISCONNECTED, APP_STATE_DISCONNECTED };
// uint8_t adv_value[MAX_ADV_DATA_LEN] = {
//         0x02, 0x01, 0x06, 0x06, 0x09, 0x6d, 0x69, 0x64, 0x64, 0x64, 0x12, 0xFF, 0xA8, 0x06, 0x01,
//         0x31, 0x38, 0x32, 0x37, 0x33, 0x36, 0x34, 0x35, 0x46, 0x43, 0x30, 0x30, 0x30, 0x34
// };

uint8_t adv_value[MAX_ADV_DATA_LEN]               = { 0x04, 0x09, 'a', 'b', 'c', 0 };
enum BLE_ADV_STATE ble_adv_state[APP_MAX_ADV_IDX] = { BLE_ADV_STATE_IDLE, BLE_ADV_STATE_IDLE, BLE_ADV_STATE_IDLE };
ble_adv_param_t ble_adv_param                     = { BLE_MCRC_MIN_INTERVAL, BLE_MCRC_MIN_INTERVAL };
ble_adv_data_t ble_adv_data[APP_MAX_ADV_IDX]      = { 0 };
ble_scan_data_t ble_scan_data[APP_MAX_ADV_IDX]    = { 0 };

// Connection
uint8_t gTargetAddr[APP_BD_ADDR_LEN]                            = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t ble_connect_id                                          = 0xFF;
uint8_t write_uuid[16]                                          = { 0xF0, 0x80, 0x0 };
uint8_t read_uuid[16]                                           = { 0xF0, 0x80, 0x0 };
uint8_t current_adv_id                                          = 0;
uint16_t read_handle                                            = 0xFF;
uint16_t write_handle                                           = 0xFF;
static uint8_t app_connection_state                             = 0;
static uint16_t ble_con_mtu[APP_BLE_CONNECT_MAX]                = { 0 };
static uint8_t ble_init_idx                                     = APP_BLE_INIT_INVALID_IDX;
static connect_req_info_t connect_req_list[APP_BLE_CONNECT_MAX] = { 0 };
static uint8_t g_curr_conn_index                                = SONATA_ADDR_NONE;
static sonata_gap_connection_req_ind_t g_connection_req;
// Bond
static bool app_bond                        = false;
static uint8_t app_loc_irk[APP_GAP_KEY_LEN] = { 0 };
static uint8_t app_rand_cnt                 = 0;

uint8_t app_csrk[APP_GAP_KEY_LEN]           = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0 };
bonded_dev_info_list_t bonded_dev_info      = { 0 };
uint8_t peer_dbaddr[APP_BD_ADDR_LEN]        = { 0 };
uint8_t app_loc_addr[APP_BD_ADDR_LEN]       = { 0x0C, 0x20, 0x18, 0xA7, 0x9F, 0xDD };
static enum sonata_gap_io_cap app_iocap     = SONATA_GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
static enum sonata_gap_auth app_auth        = SONATA_GAP_AUTH_REQ_MITM_BOND;
static uint8_t app_req_auth                 = SONATA_GAP_AUTH_REQ_NO_MITM_NO_BOND;

// App
uint8_t local_dev_name[APP_BLE_DEVICE_NAME_LEN + 1] = "asr_ble_demo";
static uint16_t local_dev_appearance                = 0;
static ble_gatt_att_manager_t service_reg_env       = { 0 };
static uint16_t app_duration                        = 0;
static app_core_evt_ind_cb app_evt_cb               = NULL;
static app_sec_req_cb sec_req_call                  = NULL;

static uint8_t need_connect_confirm               = 0;
static uint8_t service_state[APP_MAX_SERVICE_NUM] = { 0 };
cb_fun ble_cb_fun                                 = NULL;
app_ble_scan_callback_t p_scan_cb                 = NULL;

// Scan
static enum BLE_SCAN_STATE scan_state = BLE_SCAN_STATE_IDLE;
static uint8_t scan_set_state         = APP_BLE_SCAN_OFF;
app_ble_scan_param_t app_scan_param   = { 0 };

// matter
static matter_event_callback_t matter_event_cb = NULL;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
static uint16_t app_ble_set_adv_data(uint8_t adv_id);

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/*
void print_serv_env(void)
{

    printf("service_reg_env.reg_nb %d %d\r\n",service_reg_env.reg_nb,service_reg_env.add_nb);
    for(int i = 0; i < service_reg_env.reg_nb; i++)
    {
        printf("list :0x%lx %d\r\n",service_reg_env.reg_list[i],service_reg_env.reg_list[i]->nb_att);
    }
}*/
/*!
 * @brief
 */
typedef enum
{
    /// Peripheral Bonded
    SONATA_TAG_PERIPH_BONDED = (APP_DATA_SAVE_TAG_FIRST + 0),
    /// Mouse NTF Cfg
    SONATA_TAG_MOUSE_NTF_CFG,
    /// Mouse Timeout value
    SONATA_TAG_MOUSE_TIMEOUT,
    /// Peer Device BD Address
    SONATA_TAG_PEER_BD_ADDRESS,
    /// EDIV (2bytes), RAND NB (8bytes),  LTK (16 bytes), Key Size (1 byte)
    SONATA_TAG_LTK,
    /// app_ltk_key_in
    SONATA_TAG_LTK_IN,
    /// app irk addr
    SONATA_TAG_IRK_ADDR,
    /// app irk
    SONATA_TAG_IRK,
    /// device address
    SONATA_TAG_BD_ADDRESS,
    /// bonded dev info
    SONATA_TAG_BONDED_DEV_INFO,
    /// start pair on boot
    SONATA_TAG_PAIR_ON_BOOT,
} sonata_app_nvds_tag;

/*!
 * @brief
 */
typedef enum
{
    /// Peripheral Bonded len
    SONATA_LEN_PERIPH_BONDED = 1,
    /// Mouse NTF Cfg len
    SONATA_LEN_MOUSE_NTF_CFG = 2,
    /// Mouse Timeout value len
    SONATA_LEN_MOUSE_TIMEOUT = 2,
    /// Peer Device BD Address len
    SONATA_LEN_PEER_BD_ADDRESS = 6,
    /// EDIV (2bytes), RAND NB (8bytes),  LTK (16 bytes), Key Size (1 byte)
    SONATA_LEN_LTK = 27,
    /// app_ltk_key_in len
    SONATA_LEN_LTK_IN = 16,
    /// app irk addr len
    SONATA_LEN_IRK_ADDR = 6,
    /// app irk len
    SONATA_LEN_IRK = 16,
    /// device address
    SONATA_LEN_BD_ADDRESS = 6,
    /// bonded dev info len
    SONATA_LEN_BONDED_DEV_INFO = 218, // 218: 3, 290:4,
                                      /// start pair on boot
    SONATA_LEN_PAIR_ON_BOOT = 1,
} sonata_app_nvds_len;

/*!
 * @brief
 */
typedef enum
{
    SONATA_SERVICE_INIT = 0,
    SONATA_SERVICE_ENABLE,
    SONATA_SERVICE_DISABLE
} sonata_serivice_state;

/*
 * EXTERNAL FUNCTION DECLARATION
 ****************************************************************************************
 */
extern int __wrap_printf(const char * format, ...);
static uint8_t app_timer_handler(uint16_t id);

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static sonata_app_timer_callback_t app_timer_callbacks = {
    .timeout = app_timer_handler,
};

void ble_set_callback(cb_fun cb)
{
    ble_cb_fun = cb;
}

void ble_matter_event_callback_reg(matter_event_callback_t cb)
{
    matter_event_cb = cb;
    APP_TRC("APP: %s  cb=%p \r\n", __FUNCTION__, cb);
}

static void app_print_hex(uint8_t * hex, uint8_t len)
{
    for (int i = 0; i < len; i++)
    {
        APP_TRC("%x%x", hex[i] >> 4, hex[i] & 0xf);
    }
    APP_TRC("\r\n");
}

void app_connect_req_list_add(uint8_t * addr, uint8_t conidx)
{
    for (int idx = 0; idx < APP_BLE_CONNECT_MAX; idx++)
    {
        if (connect_req_list[idx].conidx == APP_BLE_INVALID_CONIDX)
        {
            connect_req_list[idx].conidx = conidx;
            memmove(connect_req_list[idx].bd_addr, addr, APP_BD_ADDR_LEN);
            return;
        }
    }
    APP_TRC("APP: %s  no space\r\n", __FUNCTION__);
}

static void app_connect_req_list_del(uint8_t conidx)
{
    for (int idx = 0; idx < APP_BLE_CONNECT_MAX; idx++)
    {
        if (connect_req_list[idx].conidx == conidx)
        {
            connect_req_list[idx].conidx = APP_BLE_INVALID_CONIDX;
            return;
        }
    }
    APP_TRC("APP: %s  not found\r\n", __FUNCTION__);
}

static uint8_t app_get_conidx_by_addr(uint8_t * addr)
{
    if (addr == NULL)
    {
        return APP_BLE_FUN_ERROR;
    }
    for (int idx = 0; idx < APP_BLE_CONNECT_MAX; idx++)
    {
        if (!memcmp(connect_req_list[idx].bd_addr, addr, APP_BD_ADDR_LEN))
        {
            return connect_req_list[idx].conidx;
        }
    }
    APP_TRC("APP: %s  not found\r\n", __FUNCTION__);
    return APP_BLE_FUN_ERROR;
}

static uint8_t * app_get_addr_by_conidx(uint8_t conidx)
{
    for (int idx = 0; idx < APP_BLE_CONNECT_MAX; idx++)
    {
        if (connect_req_list[idx].conidx == conidx)
        {
            return connect_req_list[idx].bd_addr;
        }
    }
    APP_TRC("APP: %s  not found\r\n", __FUNCTION__);
    return NULL;
}

void app_ble_set_device_name(uint8_t * name, uint32_t len)
{
    APP_TRC("APP: %s  %d\r\n", __FUNCTION__, len);
    if (APP_BLE_DEVICE_NAME_LEN < len)
    {
        return;
    }
    memmove(local_dev_name, name, len);
    local_dev_name[len] = '\0';
}

static bool app_get_is_setting_state(enum BLE_ADV_STATE curr_adv_state)
{
    switch (curr_adv_state)
    {
    case BLE_ADV_STATE_CREATING:
    case BLE_ADV_STATE_SETTING_ADV_DATA:
    case BLE_ADV_STATE_SETTING_SCAN_RSP_DATA:
    case BLE_ADV_STATE_STARTING:
    case BLE_ADV_STATE_STOPPING:
        return true;
    default:
        return false;
    }
}

static bool app_get_is_need_deal(uint8_t adv_idx)
{
    if (ble_adv_change_state[adv_idx])
    {
        return true;
    }
    return false;
}

static uint8_t app_other_adv_need_deal(int index)
{
    for (uint8_t i = APP_CONN_ADV_IDX; i < APP_MAX_ADV_IDX; i++)
    {
        if (index != i && app_get_is_need_deal(i))
        {
            APP_TRC("APP: %s  ,index=%d\r\n", __FUNCTION__, i);
            return i;
        }
    }
    return APP_MAX_ADV_IDX;
}

static void app_exe_other_adv(uint8_t adv_idx)
{
    uint8_t other_adv = app_other_adv_need_deal(adv_idx);
    if (other_adv == APP_MAX_ADV_IDX)
    {
        return;
    }
    current_adv_idx               = other_adv;
    ble_adv_change_state[adv_idx] = 0;
    if (ble_adv_set_state[other_adv] == APP_BLE_ADV_OFF)
    {
        app_ble_stop_adv(other_adv);
    }
    else if (ble_adv_state[other_adv] == BLE_ADV_STATE_CREATED)
    {
        ble_adv_state[other_adv] = BLE_ADV_STATE_SETTING_ADV_DATA;
        app_ble_set_adv_data(other_adv);
    }
    else if (ble_adv_state[other_adv] == BLE_ADV_STATE_STARTED)
    {
        app_ble_set_adv_data(other_adv);
    }
    else
    {
        app_ble_config_legacy_advertising(other_adv);
    }
}

bool app_other_adv_is_setting_state(int index)
{
    for (int i = APP_CONN_ADV_IDX; i < APP_MAX_ADV_IDX; i++)
    {
        if (index != i && app_get_is_setting_state(ble_adv_state[i]))
        {
            APP_TRC("APP: %s  ,index=%d\r\n", __FUNCTION__, i);
            return true;
        }
    }
    return false;
}

static void app_data_init(void)
{
    memset(&app_scan_param, 0, sizeof(app_ble_scan_param_t));
    memset(connect_req_list, 0xff, sizeof(connect_req_list));
    app_scan_param.addr_count              = 0;
    app_scan_param.own_addr_type           = SONATA_GAP_STATIC_ADDR;
    app_scan_param.scan_param.type         = SONATA_GAP_SCAN_TYPE_OBSERVER;
    app_scan_param.scan_param.prop         = SONATA_GAP_SCAN_PROP_ACTIVE_1M_BIT | SONATA_GAP_SCAN_PROP_PHY_1M_BIT; // 0x05
    app_scan_param.scan_param.dup_filt_pol = SONATA_GAP_DUP_FILT_DIS;
    app_scan_param.scan_param.scan_param_1m.scan_intv    = 0x0140;
    app_scan_param.scan_param.scan_param_1m.scan_wd      = 0x00A0;
    app_scan_param.scan_param.scan_param_coded.scan_intv = 24;
    app_scan_param.scan_param.scan_param_coded.scan_wd   = 12;
    app_scan_param.scan_param.duration                   = 0;
    app_scan_param.scan_param.period                     = 0;
    for (int i = 0; i < APP_BLE_CONNECT_MAX; i++)
    {
        ble_con_mtu[i] = APP_DEFAULT_MTU_SIZE;
    }
}

/*!
 * @brief config legacy advertising
 */
void app_ble_config_legacy_advertising(uint8_t adv_idx)
{
    APP_TRC("APP: %s  ,adv_idx=%d\r\n", __FUNCTION__, adv_idx);

    sonata_gap_directed_adv_create_param_t param = { 0 };
    uint8_t own_addr_type                        = SONATA_GAP_STATIC_ADDR;

    if (adv_idx == APP_CONN_ADV_IDX)
    {
        param.disc_mode    = SONATA_GAP_ADV_MODE_GEN_DISC;
        param.prop         = SONATA_GAP_ADV_PROP_UNDIR_CONN_MASK;
        param.filter_pol   = SONATA_ADV_ALLOW_SCAN_ANY_CON_ANY;
        param.addr_type    = SONATA_GAP_STATIC_ADDR;
        param.adv_intv_min = BLE_MCRC_MIN_INTERVAL;
        param.adv_intv_max = BLE_MCRC_MIN_INTERVAL;
        param.chnl_map     = MS_BLE_CHANNEL_NUM;
        param.phy          = SONATA_GAP_PHY_LE_1MBPS;
    }
    else if (adv_idx == APP_NON_CONN_ADV_IDX)
    {

        param.disc_mode    = SONATA_GAP_ADV_MODE_BEACON;
        param.prop         = SONATA_GAP_ADV_PROP_NON_CONN_NON_SCAN_MASK;
        param.filter_pol   = SONATA_ADV_ALLOW_SCAN_ANY_CON_ANY;
        param.addr_type    = SONATA_GAP_STATIC_ADDR;
        param.adv_intv_min = ble_adv_param.advertising_interval_min;
        param.adv_intv_max = ble_adv_param.advertising_interval_max;
        param.chnl_map     = MS_BLE_CHANNEL_NUM;
        param.phy          = SONATA_GAP_PHY_LE_1MBPS;
    }
    else if (adv_idx == APP_MATTER_ADV_IDX)
    {
        param.disc_mode    = SONATA_GAP_ADV_MODE_GEN_DISC;
        param.prop         = SONATA_GAP_ADV_PROP_UNDIR_CONN_MASK;
        param.max_tx_pwr   = 0xE2;
        param.filter_pol   = SONATA_ADV_ALLOW_SCAN_ANY_CON_ANY;
        param.addr_type    = SONATA_GAP_STATIC_ADDR;
        param.adv_intv_min = BLE_MCRC_MIN_INTERVAL;
        param.adv_intv_max = BLE_MCRC_MIN_INTERVAL;
        param.chnl_map     = MS_BLE_CHANNEL_NUM;
        param.phy          = SONATA_GAP_PHY_LE_1MBPS;
#ifdef PUB_RAND_ADDR_COEXIST
        own_addr_type = SONATA_GAP_SELF_SET_RAND_ADDR;
#endif
    }
    ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATING;
    current_adv_idx        = adv_idx;
    uint16_t ret = sonata_ble_config_legacy_advertising(own_addr_type, &param); // Next event:SONATA_GAP_CMP_ADVERTISING_CONFIG
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
        ble_adv_state[adv_idx] = BLE_ADV_STATE_IDLE;
    }
}

/*!
 * @brief set advertising data
 */
static uint16_t app_ble_set_adv_data(uint8_t adv_idx)
{
    APP_TRC("APP: %s, adv_idx=%d, len=%d\r\n", __FUNCTION__, adv_idx, ble_adv_data[adv_idx].ble_advdataLen);
    APP_TRC("APP: %s\r\n", ble_adv_data[adv_idx].ble_advdata);
    // Call API
    current_adv_idx = adv_idx;
    uint16_t ret    = sonata_ble_set_advertising_data_byid(ble_adv_idx_table[adv_idx], ble_adv_data[adv_idx].ble_advdataLen,
                                                           &ble_adv_data[adv_idx].ble_advdata[0]);
    // Next event:SONATA_GAP_CMP_SET_ADV_DATA
    if (ret != API_SUCCESS)
    {

        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
    return ret;
}

static uint16_t app_ble_set_scansponse_data(uint8_t adv_idx)
{
    APP_TRC("APP: %s,adv_idx=%d\r\n", __FUNCTION__, adv_idx);
    if (adv_idx >= APP_MAX_ADV_IDX)
    {
        APP_TRC_ERR("%s,Error, adv_idx=%d\r\n", __FUNCTION__, adv_idx);
        return API_FAILURE;
    }
    current_adv_idx = adv_idx;
    // Call API
    uint16_t ret = sonata_ble_set_scan_response_data_byid(ble_adv_idx_table[adv_idx], ble_scan_data[adv_idx].ble_respdataLen,
                                                          ble_scan_data[adv_idx].ble_respdata);
    // Next event:SONATA_GAP_CMP_SET_ADV_DATA
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
        ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
    }
    return ret;
}
uint8_t app_get_adv_status(uint8_t adv_idx)
{
    if (ble_adv_state[adv_idx] == BLE_ADV_STATE_IDLE || ble_adv_state[adv_idx] == BLE_ADV_STATE_STOPPING ||
        ble_adv_state[adv_idx] == BLE_ADV_STATE_CREATED)
    {
        return 0;
    }
    return 1;
}

uint16_t app_ble_start_advertising(uint8_t adv_idx)
{
    APP_TRC("APP: %s,adv_idx=%d\r\n", __FUNCTION__, adv_idx);
    if (adv_idx >= APP_MAX_ADV_IDX)
    {
        APP_TRC_ERR("%s,Error, adv_idx=%d\r\n", __FUNCTION__, adv_idx);
        return API_FAILURE;
    }
    current_adv_idx = adv_idx;
    // Call api
    uint16_t ret = sonata_ble_start_advertising_byid(ble_adv_idx_table[adv_idx], app_duration, app_max_adv_evt);
    // Next event:SONATA_GAP_CMP_ADVERTISING_START
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
    return ret;
}

uint16_t app_ble_advertising_stop(uint8_t adv_idx)
{
    APP_TRC("APP: %s ,adv_idx=%d, state=%d, \r\n", __FUNCTION__, adv_idx, ble_adv_state[adv_idx]);
    if (adv_idx >= APP_MAX_ADV_IDX)
    {
        APP_TRC_ERR("%s,Error, adv_idx=%d\r\n", __FUNCTION__, adv_idx);
        return API_FAILURE;
    }
    ble_adv_set_state[adv_idx] = APP_BLE_ADV_OFF;

    if (ble_adv_state[adv_idx] != BLE_ADV_STATE_STARTED)
    {
        APP_TRC("APP: %s  LINE:%d\r\n", __FUNCTION__, __LINE__);
        return API_FAILURE;
    }
    // Call api
    if (app_other_adv_is_setting_state(adv_idx))
    {
        APP_TRC("APP: %s  LINE:%d\r\n", __FUNCTION__, __LINE__);
        ble_adv_change_state[adv_idx] = 1;
        return 0;
    }
    current_adv_idx = adv_idx;

    ble_adv_state[adv_idx] = BLE_ADV_STATE_STOPPING;
    uint16_t ret           = sonata_ble_stop_advertising_byid(ble_adv_idx_table[adv_idx]);
    // Next event:SONATA_GAP_CMP_ADVERTISING_START
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
    return ret;
}

uint16_t app_ble_stop_adv(uint8_t adv_idx)
{
    APP_TRC("APP: %s ,adv_idx=%d, state=%d, \r\n", __FUNCTION__, adv_idx, ble_adv_state[adv_idx]);
    if (adv_idx >= APP_MAX_ADV_IDX)
    {
        APP_TRC_ERR("%s,Error, adv_idx=%d\r\n", __FUNCTION__, adv_idx);
        return API_FAILURE;
    }
    current_adv_idx = adv_idx;

    if (ble_adv_state[adv_idx] != BLE_ADV_STATE_STARTED)
    {
        return API_FAILURE;
    }
    // Call api

    uint16_t ret = sonata_ble_stop_advertising_byid(ble_adv_idx_table[adv_idx]);
    // Next event:SONATA_GAP_CMP_ADVERTISING_START
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
    return ret;
}

uint16_t app_ble_stop_adv_without_id(void)
{
    return app_ble_stop_adv(APP_CONN_ADV_IDX);
}

int app_ble_advertising_start(uint8_t adv_idx, ble_adv_data_t * data, ble_scan_data_t * scan_data)
{
    APP_TRC("************************%s**********************************  adv_idx=%d\r\n", __FUNCTION__, adv_idx);
    if (data->ble_advdataLen > MAX_ADV_DATA_LEN)
    {
        APP_TRC_ERR("Error: adv data length=%d\r\n", data->ble_advdataLen);
        return -1;
    }
    if (NULL != scan_data && scan_data->ble_respdataLen > MAX_ADV_DATA_LEN)
    {
        APP_TRC_ERR("Error: scan response data length=%d\r\n", scan_data->ble_respdataLen);
        return -1;
    }
    if (adv_idx >= APP_MAX_ADV_IDX)
    {
        APP_TRC_ERR("Error: adv_idx=%d\r\n", adv_idx);
        return -1;
    }
    ble_adv_data[adv_idx].ble_advdataLen = data->ble_advdataLen;
    memcpy((void *) &ble_adv_data[adv_idx], (void *) data->ble_advdata, data->ble_advdataLen);
    if (NULL != scan_data && scan_data->ble_respdataLen != 0)
    {
        memcpy((void *) &ble_scan_data[adv_idx], (void *) scan_data->ble_respdata, scan_data->ble_respdataLen);
        ble_scan_data[adv_idx].ble_respdataLen = scan_data->ble_respdataLen;
    }
    else
    {
        ble_scan_data[adv_idx].ble_respdataLen = 0;
    }
    ble_adv_set_state[adv_idx]    = APP_BLE_ADV_ON;
    ble_adv_change_state[adv_idx] = 1;

    if ((ble_adv_state[adv_idx] != BLE_ADV_STATE_IDLE) && (ble_adv_state[adv_idx] != BLE_ADV_STATE_CREATED) &&
        ble_adv_state[adv_idx] != BLE_ADV_STATE_STARTED)
    {
        APP_TRC("APP: %s ,myself state=%d, \r\n", __FUNCTION__, ble_adv_state[adv_idx]);
        return -1;
    }
    if (app_other_adv_is_setting_state(adv_idx))
    {
        APP_TRC("APP: %s ,other state=%d, \r\n", __FUNCTION__, ble_adv_state[adv_idx]);
        return -1;
    }
    current_adv_idx               = adv_idx;
    ble_adv_change_state[adv_idx] = 0;
    if (ble_adv_state[adv_idx] == BLE_ADV_STATE_CREATED)
    {
        ble_adv_state[adv_idx] = BLE_ADV_STATE_SETTING_ADV_DATA;
        app_ble_set_adv_data(adv_idx);
    }
    else if (ble_adv_state[adv_idx] == BLE_ADV_STATE_STARTED)
    {
        app_ble_set_adv_data(adv_idx);
    }
    else
    {
        app_ble_config_legacy_advertising(adv_idx);
    }
    return 0;
}

void app_ble_config_legacy_advertising_with_param(uint8_t own_addr_type, sonata_gap_directed_adv_create_param_t * param)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    ble_adv_state[APP_CONN_ADV_IDX] = BLE_ADV_STATE_CREATING;
    current_adv_idx                 = APP_CONN_ADV_IDX;
    uint16_t ret = sonata_ble_config_legacy_advertising(own_addr_type, param); // Next event:SONATA_GAP_CMP_ADVERTISING_CONFIG
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
        ble_adv_state[APP_CONN_ADV_IDX] = BLE_ADV_STATE_IDLE;
    }
}

void app_ble_start_advertising_with_param(sonata_gap_directed_adv_create_param_t * param, ble_adv_data_t * data,
                                          ble_scan_data_t * scan_data, uint8_t own_addr_type, uint16_t duration,
                                          uint8_t max_adv_evt)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    if (data->ble_advdataLen > MAX_ADV_DATA_LEN)
    {
        APP_TRC("APP: %s ,Error: adv data length=%d, \r\n", __FUNCTION__, data->ble_advdataLen);
        return;
    }
    if (NULL != scan_data && scan_data->ble_respdataLen > MAX_ADV_DATA_LEN)
    {
        APP_TRC("APP: %s ,Error: scan response data length=%d, \r\n", __FUNCTION__, scan_data->ble_respdataLen);
        return;
    }
    ble_adv_data[APP_CONN_ADV_IDX].ble_advdataLen = data->ble_advdataLen;
    memcpy((void *) &ble_adv_data[APP_CONN_ADV_IDX], (void *) data->ble_advdata, data->ble_advdataLen);
    if (NULL != scan_data && scan_data->ble_respdataLen != 0)
    {
        memcpy((void *) &ble_scan_data[APP_CONN_ADV_IDX], (void *) scan_data->ble_respdata, scan_data->ble_respdataLen);
        ble_scan_data[APP_CONN_ADV_IDX].ble_respdataLen = scan_data->ble_respdataLen;
    }
    else
    {
        ble_scan_data[APP_CONN_ADV_IDX].ble_respdataLen = 0;
    }
    app_duration                           = duration;
    app_max_adv_evt                        = max_adv_evt;
    ble_adv_set_state[APP_CONN_ADV_IDX]    = APP_BLE_ADV_ON;
    ble_adv_change_state[APP_CONN_ADV_IDX] = 1;
    if ((ble_adv_state[APP_CONN_ADV_IDX] != BLE_ADV_STATE_IDLE) && (ble_adv_state[APP_CONN_ADV_IDX] != BLE_ADV_STATE_CREATED) &&
        ble_adv_state[APP_CONN_ADV_IDX] != BLE_ADV_STATE_STARTED)
    {
        return;
    }
    current_adv_idx                        = APP_CONN_ADV_IDX;
    ble_adv_change_state[APP_CONN_ADV_IDX] = 0;
    if (ble_adv_state[APP_CONN_ADV_IDX] == BLE_ADV_STATE_CREATED)
    {
        ble_adv_state[APP_CONN_ADV_IDX] = BLE_ADV_STATE_SETTING_ADV_DATA;
        app_ble_set_adv_data(current_adv_id);
    }
    else if (ble_adv_state[APP_CONN_ADV_IDX] == BLE_ADV_STATE_STARTED)
    {
        app_ble_set_adv_data(current_adv_id);
    }
    else
    {
        app_ble_config_legacy_advertising_with_param(own_addr_type, param);
    }
}

static bool app_get_is_scan_setting_state(enum BLE_SCAN_STATE curr_scan_state)
{
    switch (curr_scan_state)
    {
    case BLE_SCAN_STATE_CREATING:
    case BLE_SCAN_STATE_WHILTELIST_SETTING:
    case BLE_SCAN_STATE_STARTING:
    case BLE_SCAN_STATE_STOPPING:
    case BLE_SCAN_STATE_WHILTELIST_AGAIN:
        return true;
    default:
        return false;
    }
}

uint8_t app_ble_get_scan_status(void)
{
    switch (scan_state)
    {
    case BLE_SCAN_STATE_IDLE:
    case BLE_SCAN_STATE_WHILTELIST_SETTING:
    case BLE_SCAN_STATE_STOPPING:
    case BLE_SCAN_STATE_WHILTELIST_AGAIN:
    case BLE_SCAN_STATE_CREATING:
    case BLE_SCAN_STATE_CREATED:
        return 0;
    default:
        return 1;
    }
}
/*!
 * @brief Config scanning
 */
void app_ble_config_scanning()
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    uint16_t ret = sonata_ble_config_scanning(app_scan_param.own_addr_type);
    // Next event:SONATA_GAP_CMP_SCANNING_CONFIG
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
}

/*!
 * @brief Start scanning
 */
void app_ble_start_scanning(void)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    uint16_t ret = sonata_ble_start_scanning(&app_scan_param.scan_param);
    // Scan result will show in app_gap_scan_result_callback()
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
}

void app_ble_start_scan_with_param(app_ble_scan_param_t * param)
{
    APP_TRC("APP: %s \r\n", __FUNCTION__);
    memmove(&app_scan_param, param, sizeof(app_ble_scan_param_t));
    scan_set_state = APP_BLE_SCAN_ON;
    if (app_get_is_scan_setting_state(scan_state))
    {
        APP_TRC("scan is running: \r\n");
        return;
    }
    if (app_scan_param.addr_count == 0)
    {
        if (scan_state == BLE_SCAN_STATE_IDLE)
        {
            scan_state = BLE_SCAN_STATE_CREATING;
            app_ble_config_scanning();
        }
        else if (scan_state == BLE_SCAN_STATE_CREATED)
        {
            scan_state = BLE_SCAN_STATE_STARTING;
            app_ble_start_scanning();
        }
        else if (scan_state == BLE_SCAN_STATE_STARTED)
        {
            scan_state = BLE_SCAN_STATE_STOPPING;
            sonata_ble_stop_scanning();
        }
    }
    else
    {
        if (scan_state == BLE_SCAN_STATE_STARTED)
        {
            scan_state = BLE_SCAN_STATE_STOPPING;
            sonata_ble_stop_scanning();
        }
        else if (scan_state == BLE_SCAN_STATE_CREATED)
        {
            scan_state = BLE_SCAN_STATE_WHILTELIST_AGAIN;
            sonata_ble_gap_set_white_list(app_scan_param.addr_count, app_scan_param.trans_addr_list);
        }
        else if (scan_state == BLE_SCAN_STATE_IDLE)
        {
            scan_state = BLE_SCAN_STATE_WHILTELIST_SETTING;
            sonata_ble_gap_set_white_list(app_scan_param.addr_count, app_scan_param.trans_addr_list);
        }
    }
}

void app_ble_stop_scanning(void)
{
    APP_TRC("APP: %s \r\n", __FUNCTION__);
    scan_set_state = APP_BLE_SCAN_OFF;
    if (app_get_is_scan_setting_state(scan_state))
    {
        APP_TRC("[stop]scan is running: \r\n");
        return;
    }
    if (scan_state != BLE_SCAN_STATE_STARTED)
    {
        APP_TRC("[stop]not started: \r\n");
        return;
    }
    scan_state = BLE_SCAN_STATE_STOPPING;
    sonata_ble_stop_scanning();
}

static void app_ble_scan_fsm(enum BLE_SCAN_STATE curr_scan_state, enum BLE_SCAN_EVENT event)
{
    APP_TRC("APP: %s,curr_scan_state=%d, event=%d\r\n", __FUNCTION__, curr_scan_state, event);
    app_ble_stack_msg_t msg = { 0 };
    switch (event)
    {
    case BLE_SCAN_EVENT_WT_CMP: {
        if (scan_set_state == APP_BLE_SCAN_ON)
        {
            if (curr_scan_state == BLE_SCAN_STATE_WHILTELIST_AGAIN)
            {
                scan_state = BLE_SCAN_STATE_STARTING;
                app_ble_start_scanning();
            }
            else if (curr_scan_state == BLE_SCAN_STATE_WHILTELIST_AGAIN)
            {
                scan_state = BLE_SCAN_STATE_CREATING;
                app_ble_config_scanning();
            }
            else
            {
                APP_TRC("SCAN ERROR\r\n");
            }
        }
        else
        {
            if (curr_scan_state == BLE_SCAN_STATE_WHILTELIST_AGAIN)
            {
                scan_state = BLE_SCAN_STATE_CREATED;
            }
            else if (curr_scan_state == BLE_SCAN_STATE_WHILTELIST_AGAIN)
            {
                scan_state = BLE_SCAN_STATE_IDLE;
            }
            if (ble_cb_fun != NULL)
            {
                msg.event_type = APP_BLE_STACK_EVENT_SCAN_OFF;
                ble_cb_fun(msg);
            }
        }
    }
    break;
    case BLE_SCAN_EVENT_CONFIG_CMP: {
        if (scan_set_state == APP_BLE_SCAN_ON)
        {
            if (curr_scan_state == BLE_SCAN_STATE_CREATING)
            {
                scan_state = BLE_SCAN_STATE_STARTING;
                app_ble_start_scanning();
            }
            else
            {
                APP_TRC("APP: SCAN ERROR, \r\n");
            }
        }
        else
        {
            scan_state = BLE_SCAN_STATE_CREATED;
            if (ble_cb_fun != NULL)
            {
                msg.event_type = APP_BLE_STACK_EVENT_SCAN_OFF;
                ble_cb_fun(msg);
            }
        }
    }
    break;
    case BLE_SCAN_EVENT_START_CMP: {
        if (scan_set_state == APP_BLE_SCAN_ON)
        {
            scan_state = BLE_SCAN_STATE_STARTED;
            if (ble_cb_fun != NULL)
            {
                msg.event_type = APP_BLE_STACK_EVENT_SCAN_ON;
                ble_cb_fun(msg);
            }
        }
        else
        {
            scan_state = BLE_SCAN_STATE_STOPPING;
            sonata_ble_stop_scanning();
        }
    }
    break;
    case BLE_SCAN_EVENT_STOP_CMP: {
        if (scan_set_state == APP_BLE_SCAN_ON)
        {
            scan_state = BLE_SCAN_STATE_STARTING;
            app_ble_start_scanning();
        }
        else
        {
            scan_state = BLE_SCAN_STATE_CREATED;
            if (ble_cb_fun != NULL)
            {
                msg.event_type = APP_BLE_STACK_EVENT_SCAN_OFF;
                ble_cb_fun(msg);
            }
        }
    }
    break;
    case BLE_SCAN_EVENT_DELETE_CMP: {
        if (scan_set_state == BLE_SCAN_STATE_CREATED)
        {
            scan_state = BLE_SCAN_STATE_IDLE;
        }
        else
        {
            APP_TRC("APP: SCAN ERROR, \r\n");
        }
    }
    break;
    default:
        break;
    }
}

/*!
 * @brief config initiating
 */
static void app_ble_config_initiating()
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    // Call api to config init
    uint16_t ret = sonata_ble_config_initiating(SONATA_GAP_STATIC_ADDR);
    // Next event:SONATA_GAP_CMP_INITIATING_CONFIG
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
}

/*!
 * @brief Check MAC address
 * @param address
 * @return
 */
static bool app_ble_check_address(uint8_t * address)
{
    uint8_t error_address[APP_BD_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    if (address == NULL)
    {
        return false;
    }
    if (memcmp(address, error_address, APP_BD_ADDR_LEN) == 0)
    {
        return false;
    }
    return true;
}

/*!
 * @brief Start initiating
 */
void app_ble_start_initiating(uint8_t * target)
{
    APP_TRC("APP: %s  target:%02X,%02X,%02X,%02X,%02X,%02X\r\n", __FUNCTION__, target[0], target[1], target[2], target[3],
            target[4], target[5]);
    if (app_ble_check_address(target) == false)
    {
        APP_TRC("APP: %s, Target address is not right. Stop\r\n", __FUNCTION__);
        return;
    }
    sonata_gap_init_param_t param = { 0 };
    param.type                    = SONATA_GAP_INIT_TYPE_DIRECT_CONN_EST;
    param.prop                    = SONATA_GAP_INIT_PROP_1M_BIT | SONATA_GAP_INIT_PROP_2M_BIT | SONATA_GAP_INIT_PROP_CODED_BIT;
    param.conn_to                 = 0;
    param.peer_addr.addr_type     = SONATA_GAP_STATIC_ADDR; //  Addr
    memcpy(param.peer_addr.addr.addr, target, APP_BD_ADDR_LEN);

    if (param.prop & SONATA_GAP_INIT_PROP_1M_BIT)
    {
        APP_TRC("APP: %s  (%02X)set SONATA_GAP_INIT_PROP_1M_BIT \r\n", __FUNCTION__, param.prop);
        param.scan_param_1m.scan_intv      = 0x0200;
        param.scan_param_1m.scan_wd        = 0x0100;
        param.conn_param_1m.conn_intv_min  = 0x0028;
        param.conn_param_1m.conn_intv_max  = 0x0028;
        param.conn_param_1m.conn_latency   = 0;
        param.conn_param_1m.supervision_to = 0x01F4;
        param.conn_param_1m.ce_len_min     = 0x0008;
        param.conn_param_1m.ce_len_max     = 0x0008;
    }
    if (param.prop & SONATA_GAP_INIT_PROP_2M_BIT)
    {
        APP_TRC("APP: %s  (%02X)set SONATA_GAP_INIT_PROP_2M_BIT \r\n", __FUNCTION__, param.prop);

        param.conn_param_2m.conn_intv_min  = 0x0028;
        param.conn_param_2m.conn_intv_max  = 0x0028;
        param.conn_param_2m.conn_latency   = 0;
        param.conn_param_2m.supervision_to = 0x01F4;
        param.conn_param_2m.ce_len_min     = 0x0008;
        param.conn_param_2m.ce_len_max     = 0x0008;
    }
    if (param.prop & SONATA_GAP_INIT_PROP_CODED_BIT)
    {
        APP_TRC("APP: %s  (%02X)set SONATA_GAP_INIT_PROP_CODED_BIT \r\n", __FUNCTION__, param.prop);
        param.scan_param_coded.scan_intv      = 0x0200;
        param.scan_param_coded.scan_wd        = 0x0100;
        param.conn_param_coded.conn_intv_min  = 0x0028;
        param.conn_param_coded.conn_intv_max  = 0x0028;
        param.conn_param_coded.conn_latency   = 0;
        param.conn_param_coded.supervision_to = 0x01F4;
        param.conn_param_coded.ce_len_min     = 0x0008;
        param.conn_param_coded.ce_len_max     = 0x0008;
    }

    uint16_t ret = sonata_ble_start_initiating(&param);
    // Next event:If connected, SONATA_GAP_CMP_INITIATING_DELETE event will be received
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
}

/*!
 * @brief
 * @return
 */
uint16_t app_ble_stop_initiating(void)
{
    APP_TRC("APP: %s ,, \r\n", __FUNCTION__);
    sonata_ble_stop_initiating();
    return 0;
}

void app_ble_start_connect(uint8_t own_addr_type)
{
    APP_TRC("APP: %s ,, \r\n", __FUNCTION__);
    if (ble_init_idx == APP_BLE_INIT_INVALID_IDX)
    {
        sonata_ble_config_initiating(own_addr_type);
    }
    else
    {
        app_ble_start_initiating(gTargetAddr);
    }
}

void app_ble_disconnect(void)
{
    APP_TRC("APP: %s ,ble_connect_id=%02X(X), \r\n", __FUNCTION__, ble_connect_id);
    app_connect_req_list_del(ble_connect_id);
    if (ble_connect_id != 0xFF)
    {
        sonata_ble_gap_disconnect(ble_connect_id, 0);
    }
    ble_connect_id = 0xFF;
}

uint8_t app_get_connect_status(uint16_t conn_hdl)
{
    if (app_connected_state[conn_hdl] == APP_STATE_CONNECTED)
    {
        return 1;
    }
    return 0;
}

void app_print_adv_status(void)
{
    printf("adv_ble_state %d %d set state %d %d \r\n", ble_adv_state[APP_CONN_ADV_IDX], ble_adv_state[APP_NON_CONN_ADV_IDX],
           ble_adv_set_state[APP_CONN_ADV_IDX], ble_adv_set_state[APP_NON_CONN_ADV_IDX]);
}

/*!
 * @brief
 */
static void app_ble_on()
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    sonata_gap_set_dev_config_cmd cmd = { 0 };
    cmd.role                          = SONATA_GAP_ROLE_ALL;
    cmd.gap_start_hdl                 = 0;
    cmd.gatt_start_hdl                = 0;
    cmd.renew_dur                     = 0x0096;
    cmd.privacy_cfg                   = 0;
    cmd.pairing_mode                  = SONATA_GAP_PAIRING_SEC_CON | SONATA_GAP_PAIRING_LEGACY;
    cmd.att_cfg                       = 0x0080;
    cmd.max_mtu                       = APP_BLE_MAX_MTU_SIZE;
    cmd.max_mps                       = 0x02A0;
    cmd.max_nb_lecb                   = 0x0A;
    cmd.hl_trans_dbg                  = false;
#if APP_WVT_ENABLE
    cmd.hl_trans_dbg = true;
    printf("%s ,WVT Enable, \r\n", __FUNCTION__);
#endif
    // Get bond status from FS
    //    uint8_t length = SONATA_LEN_PERIPH_BONDED;
    //    if (sonata_fs_read(SONATA_TAG_PERIPH_BONDED, &length, (uint8_t *)&app_bond) != SONATA_FS_OK)
    //    {
    // If read value is invalid, set status to not bonded
    //        app_bond = 0;
    //    }
    //    if (app_bond == 1)
    //    {
    //        memcpy(cmd.irk.key, app_loc_irk, 16);
    //    }
    //    length = SONATA_LEN_BONDED_DEV_INFO;
    //    if (sonata_fs_read(SONATA_TAG_BONDED_DEV_INFO, &length, (uint8_t *)&bonded_dev_info) != SONATA_FS_OK)
    //    {
    //        APP_TRC("APP: %s  read bonded device info fail \r\n", __FUNCTION__);
    //    }

    uint16_t ret = sonata_ble_on(&cmd); // Next event:SONATA_GAP_CMP_BLE_ON
    if (ret != API_SUCCESS)
    {
        APP_TRC("APP: %s  ERROR:%02X\r\n", __FUNCTION__, ret);
    }
}

extern void ble_reset_cmp(void);
/*
 * LOCAL FUNCTION DEFINITIONS    Callback functions
 ****************************************************************************************
 */
static void test_adv_h(void)
{
    sonata_gap_directed_adv_create_param_t param = { 0 };
    param.disc_mode                              = SONATA_GAP_ADV_MODE_GEN_DISC;
    param.prop                                   = SONATA_GAP_ADV_PROP_UNDIR_CONN_MASK;
    // param.max_tx_pwr = 0xE2;
    param.filter_pol = SONATA_ADV_ALLOW_SCAN_ANY_CON_ANY;
    //    msg->adv_param.adv_param.peer_addr.addr.addr:00
    param.addr_type    = SONATA_GAP_STATIC_ADDR;
    param.adv_intv_min = BLE_MCRC_MIN_INTERVAL;
    param.adv_intv_max = BLE_MCRC_MIN_INTERVAL;
    param.chnl_map     = MS_BLE_CHANNEL_NUM;
    param.phy          = SONATA_GAP_PHY_LE_1MBPS;
    ble_adv_data_t data_Set;
    memmove(data_Set.ble_advdata, adv_value, adv_length);
    data_Set.ble_advdataLen = adv_length;

    app_ble_start_advertising_with_param(&param, &data_Set, NULL, SONATA_GAP_STATIC_ADDR, 0, 0);
}
#ifdef MEDIA_MATTER_TEST
void adv_media_connect_start()
{
    ble_adv_data_t data_Set;
    memmove(data_Set.ble_advdata, adv_value, adv_length);
    data_Set.ble_advdataLen = adv_length;
    app_ble_advertising_start(APP_CONN_ADV_IDX, &data_Set, NULL);
}

void adv_media_beacon_start()
{
    uint8_t value[MAX_ADV_DATA_LEN] = { 0x04, 0x09, 'd', 'e', 'f', 0 };
    ble_adv_data_t data_Set;
    memmove(data_Set.ble_advdata, value, 5);
    data_Set.ble_advdataLen = 5;
    app_ble_advertising_start(APP_NON_CONN_ADV_IDX, &data_Set, NULL);
}
#endif

static void app_ble_adv_event_handler(uint8_t adv_idx, bool on)
{
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg = { 0 };
        if (adv_idx == APP_CONN_ADV_IDX)
        {
            if (on)
            {
                msg.event_type = APP_BLE_STACK_EVENT_ADV_ON;
            }
            else
            {
                msg.event_type = APP_BLE_STACK_EVENT_ADV_OFF;
            }
            ble_cb_fun(msg);
        }
        else if (adv_idx == APP_NON_CONN_ADV_IDX)
        {
            if (on)
            {
                msg.event_type = APP_BLE_STACK_EVENT_NONCONN_ADV_ON;
            }
            else
            {
                msg.event_type = APP_BLE_STACK_EVENT_NONCONN_ADV_OFF;
            }
            ble_cb_fun(msg);
        }
    }
}
bool is_matter_activity(uint16_t param)
{
    return ble_adv_idx_table[APP_MATTER_ADV_IDX] == param;
}
void set_static_ramdom_address()
{
    printf("set_static_ramdom_address enter");
#ifdef PUB_RAND_ADDR_COEXIST
    uint8_t addr[6] = { 0xFF, 0x11, 0x22, 0x33, 0x44, 0xFF };
    sonata_ble_self_set_random_address(&(addr[0]), 6);
#endif
}
/*!
 * @brief BLE complete event handler
 * @param opt_id @see sonata_ble_complete_type
 * @param status complete status
 * @param param param and deparam will be difference according to difference complete event.
 * @param dwparam param and deparam will be difference according to difference complete event.
 * @return
 */
static uint16_t app_ble_complete_event_handler(sonata_ble_complete_type opt_id, uint8_t status, uint16_t param, uint32_t dwparam)
{
    APP_TRC("CMP: %s  opt_id=%04X,status=%02X,param=%04X,dwparam=%lu\r\n", __FUNCTION__, opt_id, status, param, dwparam);
    uint16_t cb_result      = CB_DONE;
    uint8_t adv_idx         = current_adv_idx;
    app_ble_stack_msg_t msg = { 0 };
    if (status != 0)
    {
        APP_TRC("ERR: %s  **********Operation[%04X] Fail. Reason:%02X(X)\r\n", __FUNCTION__, opt_id, status);
        return CB_DONE;
    }

    if (matter_event_cb && (matter_event_cb(opt_id, status, param, dwparam) == MATTER_EVENT_FINISHED))
    {
        return cb_result;
    }

    switch (opt_id)
    {
    case SONATA_GAP_CMP_BLE_ON: // 0x0F01
                                // ble_adv_set_state[adv_idx] = APP_BLE_ADV_ON;
                                // test_adv_h();
#if CONFIG_ENABLE_ASR_APP_MESH
        app_ble_on_callback(status);
#endif
        if (ble_cb_fun != NULL)
        {
            msg.event_type = APP_BLE_STACK_EVENT_STACK_READY;
            ble_cb_fun(msg);
        }
        set_static_ramdom_address();
        printf("AAA: %s ,SONATA_GAP_CMP_BLE_ON, \r\n", __FUNCTION__);
        break;
    case SONATA_GAP_CMP_ADVERTISING_CONFIG: // 0x0F02
        printf("ble_adv_idx_table[%d]=%d\r\n", adv_idx, param);
        ble_adv_idx_table[adv_idx] = param;
        if (ble_adv_set_state[adv_idx] == APP_BLE_ADV_ON)
        {
            ble_adv_state[adv_idx] = BLE_ADV_STATE_SETTING_ADV_DATA;
            if (API_SUCCESS != app_ble_set_adv_data(adv_idx))
            {
                ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
            }
            else
            {
                break;
            }
        }
        else
        {
            ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
            app_ble_adv_event_handler(adv_idx, false);
        }
        app_exe_other_adv(adv_idx);
        break;
    case SONATA_GAP_CMP_SET_ADV_DATA: // 0x01A9
        if (ble_adv_set_state[adv_idx] == APP_BLE_ADV_ON)
        {
            if (ble_scan_data[adv_idx].ble_respdataLen == 0)
            {
                if (ble_adv_state[adv_idx] == BLE_ADV_STATE_SETTING_ADV_DATA)
                {
                    if (API_SUCCESS == app_ble_start_advertising(adv_idx))
                    {
                        ble_adv_state[adv_idx] = BLE_ADV_STATE_STARTING;
                    }
                    else
                    {
                        ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
                        app_exe_other_adv(adv_idx);
                    }
                }
                else
                {
                    app_exe_other_adv(adv_idx);
                }
            }
            else if (API_SUCCESS == app_ble_set_scansponse_data(adv_idx))
            {
                if (ble_adv_state[adv_idx] == BLE_ADV_STATE_SETTING_ADV_DATA)
                {
                    ble_adv_state[adv_idx] = BLE_ADV_STATE_SETTING_SCAN_RSP_DATA;
                }
            }
            else
            {}
        }
        else
        {
            if (ble_adv_state[adv_idx] == BLE_ADV_STATE_STARTED)
            {
                if (API_SUCCESS == app_ble_stop_adv(adv_idx))
                {
                    ble_adv_state[adv_idx] = BLE_ADV_STATE_STOPPING;
                }
            }
            else
            {
                ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
                app_ble_adv_event_handler(adv_idx, false);
                app_exe_other_adv(adv_idx);
            }
        }
        break;
    case SONATA_GAP_CMP_SET_SCAN_RSP_DATA:
        if (ble_adv_set_state[adv_idx] == APP_BLE_ADV_ON)
        {
            if (ble_adv_state[adv_idx] == BLE_ADV_STATE_SETTING_SCAN_RSP_DATA)
            {
                if (API_SUCCESS == app_ble_start_advertising(adv_idx))
                {
                    ble_adv_state[adv_idx] = BLE_ADV_STATE_STARTING;
                }
                else
                {
                    ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
                    app_exe_other_adv(adv_idx);
                }
            }
            else
            {
                // adv is on ,only update data
                app_exe_other_adv(adv_idx);
            }
        }
        else
        {

            if (ble_adv_state[adv_idx] == BLE_ADV_STATE_STARTED)
            {
                if (API_SUCCESS == app_ble_stop_adv(adv_idx))
                {
                    ble_adv_state[adv_idx] = BLE_ADV_STATE_STOPPING;
                }
            }
            else
            {
                ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
                app_ble_adv_event_handler(adv_idx, false);
                app_exe_other_adv(adv_idx);
            }
        }
        break;
    case SONATA_GAP_CMP_ADVERTISING_START: // 0x0F06

        ble_adv_state[adv_idx] = BLE_ADV_STATE_STARTED;
        if (ble_adv_set_state[adv_idx] == APP_BLE_ADV_ON)
        {
            app_ble_adv_event_handler(adv_idx, true);
            app_exe_other_adv(adv_idx);
            if (app_evt_cb != NULL)
            {
                app_adv_status_ind_t status_ind;
                status_ind.advId  = param;
                status_ind.status = status;
                app_evt_cb(BLE_ADV_START, (void *) &status_ind);
            }
        }
        else
        {
            if (API_SUCCESS == app_ble_stop_adv(adv_idx))
            {
                ble_adv_state[adv_idx] = BLE_ADV_STATE_STOPPING;
            }
            else
            {
                app_exe_other_adv(adv_idx);
            }
        }
        break;
    case SONATA_GAP_CMP_ADVERTISING_STOP:
        ble_adv_state[adv_idx] = BLE_ADV_STATE_CREATED;
        if (ble_adv_set_state[adv_idx] == APP_BLE_ADV_OFF)
        {
            app_ble_adv_event_handler(adv_idx, false);
            app_exe_other_adv(adv_idx);
        }
        else
        {
            if (API_SUCCESS == app_ble_start_advertising(adv_idx))
            {
                ble_adv_state[adv_idx] = BLE_ADV_STATE_STARTING;
            }
            else
            {
                app_exe_other_adv(adv_idx);
            }
        }
        break;
    case SONATA_GAP_CMP_ADVERTISING_DELETE:
        ble_adv_state[adv_idx] = BLE_ADV_STATE_IDLE;
        break;
    case SONATA_GAP_CMP_PROFILE_TASK_ADD: // 0x011B
        break;
    case SONATA_GAP_CMP_SCANNING_START: // 0x0F07
        app_ble_scan_fsm(scan_state, BLE_SCAN_EVENT_START_CMP);
        break;
    case SONATA_GAP_CMP_SCANNING_STOP: // 0x0F08
        app_ble_scan_fsm(scan_state, BLE_SCAN_EVENT_STOP_CMP);
        cb_result = CB_DONE; // delete scan instance
        break;
    case SONATA_GAP_CMP_SCANNING_DELETE: // 0x0F0F
        app_ble_scan_fsm(scan_state, BLE_SCAN_EVENT_DELETE_CMP);
        break;
    case SONATA_GAP_CMP_INITIATING_CONFIG: // 0x0F04
        ble_init_idx = param;
        app_ble_start_initiating(gTargetAddr);
        break;
    case SONATA_GAP_CMP_INITIATING_STOP:
        cb_result = CB_REJECT; // delete scan instance
        break;
    case SONATA_GAP_CMP_INITIATING_DELETE: // 0x0F10
        // if (app_ble_check_target_addr())
        // {
        //     sonata_ble_gatt_disc_all_characteristic(ble_connect_id, 1, 0XFFFF);
        // }
        break;
    case SONATA_GATT_CMP_NOTIFY:
        APP_TRC("CMP: SONATA_GATT_CMP_NOTIFY, seq:%d \r\n", (uint16_t) dwparam);
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_NOTIFY;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GATT_CMP_INDICATE:
        APP_TRC("CMP: SONATA_GATT_CMP_INDICATE, seq:%d \r\n", (uint16_t) dwparam);
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_INDICATE;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GATT_CMP_DISC_ALL_SVC: // 0x0402
        APP_TRC("CMP: SONATA_GATT_CMP_DISC_ALL_SVC, seq:%d \r\n", (uint16_t) dwparam);
        // sonata_ble_gatt_read_by_handle(param, demo_handle_id);
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_SVC_DISC;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GATT_CMP_DISC_ALL_CHAR:
        APP_TRC("CMP: SONATA_GATT_CMP_DISC_ALL_CHAR, seq:%d \r\n", (uint16_t) dwparam);
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_CHAR_DISC;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GATT_CMP_DISC_BY_UUID_SVC:
        APP_TRC("CMP: SONATA_GATT_CMP_DISC_BY_UUID_SVC, seq:%d \r\n", (uint16_t) dwparam);
        // sonata_ble_gatt_read_by_handle(param, demo_handle_id);
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_SVC_DISC;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GATT_CMP_DISC_DESC_CHAR:
        APP_TRC("CMP: SONATA_GATT_CMP_DISC_DESC_CHAR, seq:%d \r\n", (uint16_t) dwparam);
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_DISC_DESC_CHAR;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GATT_CMP_READ: // 0x0408
        break;
    case SONATA_GATT_CMP_WRITE:
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_WRITE_REQ;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GATT_CMP_WRITE_NO_RESPONSE:
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_WRITE_CMD;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GAP_CMP_SET_WL:
        app_ble_scan_fsm(scan_state, BLE_SCAN_EVENT_WT_CMP);
        break;
    case SONATA_GAP_CMP_SCANNING_CONFIG:
        app_ble_scan_fsm(scan_state, BLE_SCAN_EVENT_CONFIG_CMP);
        break;
    case SONATA_GATT_CMP_MTU_EXCH:
        APP_TRC("CMP: SONATA_GATT_CMP_MTU_EXCH, seq:%d \r\n", (uint16_t) dwparam);
        if (ble_cb_fun != NULL)
        {
            msg.event_type     = APP_BLE_STACK_EVENT_CMP_MTU;
            msg.param.conn_hdl = param;
            ble_cb_fun(msg);
        }
        break;
    case SONATA_GAP_CMP_RESET:
        ble_reset_cmp();
        break;
    default:
        APP_TRC("CMP: %s, App get complete event,but not handle. opt_id=%04X\r\n", __FUNCTION__, opt_id);
        break;
    }

    APP_TRC("CMP: adv_idx=%d, adv state=%d, set state=%d, \r\n", adv_idx, ble_adv_state[adv_idx], ble_adv_set_state[adv_idx]);
    return cb_result;
}

static void app_ble_gatt_add_srv_rsp_hand(uint16_t handle)
{
    APP_TRC("APP: %s ,handle=%d, \r\n", __FUNCTION__, handle);
    uint8_t reg_nb                                              = service_reg_env.reg_nb;
    service_reg_env.reg_list[service_reg_env.reg_nb]->state     = SONATA_SERVICE_ENABLE;
    service_reg_env.reg_list[service_reg_env.reg_nb]->start_hdl = handle;
    service_state[service_reg_env.reg_nb]                       = SONATA_SERVICE_ENABLE;
    service_reg_env.reg_nb++;
    // print_serv_env();
    if (service_reg_env.add_nb != service_reg_env.reg_nb)
    {
        APP_TRC("add new service\r\n");
        uint8_t perm                  = service_reg_env.reg_list[service_reg_env.reg_nb]->perm;
        uint8_t * uuid                = service_reg_env.reg_list[service_reg_env.reg_nb]->uuid;
        uint8_t nb_att                = service_reg_env.reg_list[service_reg_env.reg_nb]->nb_att;
        sonata_gatt_att_desc_t * atts = service_reg_env.reg_list[service_reg_env.reg_nb]->att_desc;
        sonata_ble_gatt_add_service_request(service_reg_env.reg_list[service_reg_env.reg_nb]->start_hdl, perm, uuid, nb_att,
                                            &atts[1]);
    }

    if (app_evt_cb != NULL)
    {
        app_reg_service_cmp_t status_ind;
        status_ind.len = SONATA_PERM_GET(service_reg_env.reg_list[reg_nb]->perm, SVC_UUID_LEN);
        memmove(status_ind.uuid, service_reg_env.reg_list[reg_nb]->uuid, SONATA_ATT_UUID_128_LEN);
        status_ind.status  = 0;
        status_ind.handler = service_reg_env.reg_nb << 16;
        app_evt_cb(BLE_SERVICE_ADD_CMP, (void *) &status_ind);
    }
}

static void app_ble_set_svc_visibility(uint16_t handle)
{
    for (int i = 0; i < service_reg_env.reg_nb; i++)
    {
        if (service_reg_env.reg_list[i]->start_hdl == handle)
        {
            service_reg_env.reg_list[i]->state = service_state[i];
        }
    }
}

static uint16_t app_ble_rsp_event_handler(uint16_t opt_id, uint8_t status, uint16_t handle, uint16_t perm, uint16_t ext_perm,
                                          uint16_t length, void * param)
{
    APP_TRC("APP: %s  opt_id=%04X, %d %d %d \r\n", __FUNCTION__, opt_id, perm, ext_perm, length);
    if (status != 0)
    {
        APP_TRC("APP: %s  ERROR=%04X,%x\r\n", __FUNCTION__, status, param);
    }
    switch (opt_id)
    {
    case SONATA_GATT_ADD_SVC_RSP: {

        APP_TRC("APP: %s  handle=%04X,\r\n", __FUNCTION__, handle);
        // Should save the start handle id for future use
        app_ble_gatt_add_srv_rsp_hand(handle);
#ifdef MEDIA_MATTER_TEST
        adv_media_connect_start();
#endif
        break;
    }
    case SONATA_GATT_SVC_VISIBILITY_SET_RSP: {
        app_ble_set_svc_visibility(handle);
    }
    break;
    default:
        break;
    }

    return CB_DONE;
}

/*!
 * @brief get devcie informations
 * @param info_type @see sonata_gap_local_dev_info
 * @param info
 */
static uint16_t app_get_dev_info_callback(sonata_gap_local_dev_info info_type, void * info)
{
    switch (info_type)
    {
    case SONATA_GET_DEV_VERSION: {
#if APP_DBG
        sonata_gap_dev_version_ind_t * dev_info = (sonata_gap_dev_version_ind_t *) info;
        APP_TRC("APP: %s, hci_ver =0x%02X\r\n", __FUNCTION__, dev_info->hci_ver);
        APP_TRC("APP: %s, lmp_ver =0x%02X\r\n", __FUNCTION__, dev_info->lmp_ver);
        APP_TRC("APP: %s, host_ver =0x%02X\r\n", __FUNCTION__, dev_info->host_ver);
        APP_TRC("APP: %s, hci_subver =0x%04X\r\n", __FUNCTION__, dev_info->hci_subver);
        APP_TRC("APP: %s, lmp_subver =0x%04X\r\n", __FUNCTION__, dev_info->lmp_subver);
        APP_TRC("APP: %s, host_subver =0x%04X\r\n", __FUNCTION__, dev_info->host_subver);
        APP_TRC("APP: %s, manuf_name =0x%04X\r\n", __FUNCTION__, dev_info->manuf_name);
#endif // SONATA_API_TASK_DBG
    }
    break;
    case SONATA_GET_DEV_BDADDR: {
#if APP_DBG
        sonata_gap_dev_bdaddr_ind_t * param = (sonata_gap_dev_bdaddr_ind_t *) info;
        APP_TRC("APP: %s, SONATA_GET_DEV_BDADDR:", __FUNCTION__);
        for (int i = 0; i < APP_BD_ADDR_LEN; ++i)
        {
            APP_TRC("%02X ", param->addr.addr.addr[i]);
        }
        APP_TRC("\r\n");
#endif // SONATA_API_TASK_DBG
    }
    break;

    case SONATA_GET_DEV_ADV_TX_POWER: {
#if APP_DBG
        sonata_gap_dev_adv_tx_power_ind_t * param = (sonata_gap_dev_adv_tx_power_ind_t *) info;
        APP_TRC("APP: %s, SONATA_GET_DEV_ADV_TX_POWER power_lvl =0x%02X\r\n", __FUNCTION__, param->power_lvl);
#endif // SONATA_API_TASK_DBG
    }
    break;
    case SONATA_GET_WLIST_SIZE: {
#if APP_DBG
        sonata_gap_list_size_ind_t * param = (sonata_gap_list_size_ind_t *) info;
        APP_TRC("APP: %s, SONATA_GET_WLIST_SIZE size =0x%02X\r\n", __FUNCTION__, param->size);
#endif // SONATA_API_TASK_DBG

        break;
    }
    case SONATA_GET_ANTENNA_INFO: {
#if APP_DBG
        sonata_gap_antenna_inf_ind_t * param = (sonata_gap_antenna_inf_ind_t *) info;
        APP_TRC(">>> SONATA_GET_ANTENNA_INFO supp_switching_sampl_rates =0x%02X, antennae_num =0x%02X, max_switching_pattern_len "
                "=0x%02X, max_cte_len =0x%02X\r\n",
                param->supp_switching_sampl_rates, param->antennae_num, param->max_switching_pattern_len, param->max_cte_len);
#endif // SONATA_API_TASK_DBG
    }
    break;

    case SONATA_GET_SUGGESTED_DFLT_LE_DATA_LEN: {
#if APP_DBG
        sonata_gap_sugg_dflt_data_len_ind_t * param = (sonata_gap_sugg_dflt_data_len_ind_t *) info;
        APP_TRC(">>> SONATA_GET_SUGGESTED_DFLT_LE_DATA_LEN suggted_max_tx_octets =0x%02X, suggted_max_tx_time =0x%02X\r\n",
                param->suggted_max_tx_octets, param->suggted_max_tx_time);
#endif // SONATA_API_TASK_DBG
        break;
    }
    case SONATA_GET_MAX_LE_DATA_LEN: {
#if APP_DBG
        sonata_gap_max_data_len_ind_t * param = (sonata_gap_max_data_len_ind_t *) info;
        APP_TRC(">>> SONATA_GET_MAX_LE_DATA_LEN suppted_max_tx_octets =0x%04X, suppted_max_tx_time =0x%04X, suppted_max_rx_octets "
                "=0x%04X, suppted_max_rx_time =0x%04X\r\n",
                param->suppted_max_tx_octets, param->suppted_max_tx_time, param->suppted_max_rx_octets, param->suppted_max_rx_time);
#endif // SONATA_API_TASK_DBG
        break;
    }
    case SONATA_GET_PAL_SIZE: {
#if APP_DBG
        sonata_gap_list_size_ind_t * param = (sonata_gap_list_size_ind_t *) info;
        APP_TRC("APP: %s, SONATA_GET_PAL_SIZE size =0x%02X\r\n", __FUNCTION__, param->size);
#endif // SONATA_API_TASK_DBG
        break;
    }
    case SONATA_GET_RAL_SIZE: {
#if APP_DBG
        sonata_gap_list_size_ind_t * param = (sonata_gap_list_size_ind_t *) info;
        APP_TRC("APP: %s, SONATA_GET_RAL_SIZE size =0x%02X\r\n", __FUNCTION__, param->size);
#endif // SONATA_API_TASK_DBG
        break;
    }
    case SONATA_GET_NB_ADV_SETS: {
#if APP_DBG
        sonata_gap_nb_adv_sets_ind_t * param = (sonata_gap_nb_adv_sets_ind_t *) info;
        APP_TRC("APP: %s, SONATA_GET_NB_ADV_SETS nb_adv_sets =0x%02X\r\n", __FUNCTION__, param->nb_adv_sets);
#endif // SONATA_API_TASK_DBG

        break;
    }
    case SONATA_GET_MAX_LE_ADV_DATA_LEN: {
#if APP_DBG
        sonata_gap_max_adv_data_len_ind_t * param = (sonata_gap_max_adv_data_len_ind_t *) info;
        APP_TRC(">>> SONATA_GET_MAX_LE_ADV_DATA_LEN param->length=0x%02X\r\n", param->length);
#endif // SONATA_API_TASK_DBG
        break;
    }
    case SONATA_GET_DEV_TX_PWR: {
#if APP_DBG
        sonata_gap_dev_tx_pwr_ind_t * param = (sonata_gap_dev_tx_pwr_ind_t *) info;
        APP_TRC(">>> SONATA_GET_DEV_TX_PWR min_tx_pwr =0x%04X, max_tx_pwr =0x%04X\r\n", param->min_tx_pwr, param->max_tx_pwr);
#endif // SONATA_API_TASK_DBG
        break;
    }
    case SONATA_GET_DEV_RF_PATH_COMP: {
#if APP_DBG
        sonata_gap_dev_rf_path_comp_ind_t * param = (sonata_gap_dev_rf_path_comp_ind_t *) info;
        APP_TRC(">>> SONATA_GET_DEV_RF_PATH_COMP tx_path_comp =0x%04X, rx_path_comp =0x%04X\r\n", param->tx_path_comp,
                param->rx_path_comp);
#endif // SONATA_API_TASK_DBG
        break;
    }
    default:
        APP_TRC("APP: %s  No progress for info_type=%02X\r\n", __FUNCTION__, info_type);
        break;
    }
    return CB_DONE;
}

void app_ble_set_device_appearance(uint16_t value)
{
    APP_TRC("APP: %s  value=%d\r\n", __FUNCTION__, value);
    local_dev_appearance = value;
}

/*!
 * @brief Deal with peer device get local information request.
 * @param opt @see asr_gap_dev_info
 */
static uint16_t app_gap_peer_get_local_info_callback(uint8_t conidx, sonata_gap_dev_info opt)
{
    APP_TRC("APP: %s ,conidx=%d, opt=%d, \r\n", __FUNCTION__, conidx, opt);
    switch (opt)
    {
    case SONATA_GAP_DEV_NAME:
        sonata_ble_gap_send_get_dev_info_cfm_for_dev_name(conidx, strlen((char *) local_dev_name), local_dev_name);
        break;

    case SONATA_GAP_DEV_APPEARANCE:
        sonata_ble_gap_send_get_dev_info_cfm_for_dev_appearance(conidx, local_dev_appearance);

        break;

    case SONATA_GAP_DEV_SLV_PREF_PARAMS:
        sonata_ble_gap_send_get_dev_info_cfm_for_slv_pref_params(conidx, 8, 10, 0, 200);

        break;

    default:
        break;
    }

    return CB_DONE;
}

void app_gap_set_scan_cb(app_ble_scan_callback_t cb)
{
    p_scan_cb = cb;
}

typedef struct app_adv_report_ind
{
    uint8_t actv_idx;
    uint8_t info;
    struct sonata_gap_bdaddr trans_addr;
    struct sonata_gap_bdaddr target_addr;
    int8_t tx_pwr;
    int8_t rssi;
    uint8_t phy_prim;
    uint8_t phy_second;
    uint8_t adv_sid;
    uint16_t period_adv_intv;
    uint16_t length;
    uint8_t data[128];
} app_adv_report_ind_t;

static app_adv_report_ind_t report_ind_cache;

void app_gap_backup_scan_result(sonata_gap_ext_adv_report_ind_t * result)
{
    memset(&report_ind_cache, 0, sizeof(app_adv_report_ind_t));
    report_ind_cache.actv_idx        = result->actv_idx;
    report_ind_cache.info            = result->info;
    report_ind_cache.tx_pwr          = result->tx_pwr;
    report_ind_cache.rssi            = result->rssi;
    report_ind_cache.phy_prim        = result->phy_prim;
    report_ind_cache.phy_second      = result->phy_second;
    report_ind_cache.adv_sid         = result->adv_sid;
    report_ind_cache.period_adv_intv = result->period_adv_intv;
    report_ind_cache.length          = result->length;
    memcpy(&report_ind_cache.trans_addr, &result->trans_addr, sizeof(struct sonata_gap_bdaddr));
    memcpy(&report_ind_cache.target_addr, &result->target_addr, sizeof(struct sonata_gap_bdaddr));
    memcpy(report_ind_cache.data, result->data, (result->length > 128 ? 128 : result->length));
}

/*!
 * @brief GAP scan result callback
 * @param result
 */
static uint16_t app_gap_scan_result_callback(sonata_gap_ext_adv_report_ind_t * result)
{
    static uint8_t adv_ind_flag = 0;

    uint8_t type          = result->info & SONATA_GAP_REPORT_INFO_REPORT_TYPE_MASK;
    uint8_t complete_flag = result->info & SONATA_GAP_REPORT_INFO_COMPLETE_BIT;
    if (0 == complete_flag)
    {
        APP_TRC("  not complete, drop it\r\n");
    }
    switch (type)
    {
    case SONATA_GAP_REPORT_TYPE_ADV_EXT:
        APP_TRC("SCAN: type=ADV_EXT      ");
        return CB_DONE;
        break;
    case SONATA_GAP_REPORT_TYPE_ADV_LEG: {
        uint8_t scan_flag = result->info & SONATA_GAP_REPORT_INFO_SCAN_ADV_BIT;
        if (scan_flag != 0 &&
            ((app_scan_param.scan_param.prop & SONATA_GAP_SCAN_PROP_ACTIVE_1M_BIT) ||
             (app_scan_param.scan_param.prop & SONATA_GAP_SCAN_PROP_ACTIVE_CODED_BIT)))
        {

            // APP_TRC("ADV_LEG wait rsp\r\n");
            adv_ind_flag = 1;
            app_gap_backup_scan_result(result);
            return CB_DONE;
        }
        else
        {
            // APP_TRC("ADV_LEG report %d\r\n", result->rssi);
            adv_ind_flag = 0;
            if (p_scan_cb)
            {
                p_scan_cb(result, NULL);
                return CB_DONE;
            }
        }
    }
    break;
    case SONATA_GAP_REPORT_TYPE_SCAN_RSP_EXT:
        APP_TRC("SCAN: type=SCAN_RSP_EXT ");
        return CB_DONE;
        break;
    case SONATA_GAP_REPORT_TYPE_SCAN_RSP_LEG: {
        if (adv_ind_flag == 0)
        {
            APP_TRC("SCAN_RSP_LEG not found adv ind \r\n ");
            return CB_DONE;
        }
        else
        {
            adv_ind_flag = 0;
            if (0 == memcmp(report_ind_cache.trans_addr.addr.addr, result->trans_addr.addr.addr, 6))
            {

                // APP_TRC("ADV rsp report %d %d\r\n", report_ind_cache.rssi, result->rssi);
                if (p_scan_cb)
                {
                    p_scan_cb(&report_ind_cache, result);
                    return CB_DONE;
                }
                APP_TRC("user handle adv + scan \r\n ");
            }
            else
            {
                APP_TRC("  trans_addr:");
                for (int i = 0; i < APP_BD_ADDR_LEN; ++i)
                {
                    APP_TRC("%02X ", result->trans_addr.addr.addr[i]);
                }
                for (int i = 0; i < APP_BD_ADDR_LEN; ++i)
                {
                    APP_TRC("%02X ", report_ind_cache.trans_addr.addr.addr[i]);
                }
                APP_TRC("SCAN_RSP_LEG addr not match \r\n ");
            }
        }
    }
    break;
    case SONATA_GAP_REPORT_TYPE_PER_ADV:
        return CB_DONE;
    }
    return CB_DONE;
}

/*!
 * @brief
 * @param conidx
 * @param conhdl
 * @param reason
 * @return
 */
static uint16_t app_gap_disconnect_ind_callback(uint8_t conidx, uint16_t conhdl, uint8_t reason)
{
    APP_TRC("APP: %s  conidx=%d,conhdl=%d,reason=%02x\r\n", __FUNCTION__, conidx, conhdl, reason);
    app_connected_state[conidx] = APP_STATE_DISCONNECTED;
    ble_con_mtu[conidx]         = APP_DEFAULT_MTU_SIZE;

    app_connect_req_list_del(conidx);
#ifdef MEDIA_MATTER_TEST
    app_ble_advertising_stop(APP_NON_CONN_ADV_IDX);
    adv_media_connect_start();
#endif
    if (matter_event_cb && (matter_event_cb(APP_BLE_STACK_EVENT_DISCONNECTED, reason, conidx, conhdl) == MATTER_EVENT_FINISHED))
    {
        return CB_DONE;
    }
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg           = { 0 };
        msg.event_type                    = APP_BLE_STACK_EVENT_DISCONNECTED;
        msg.param.disconnect_msg.conn_hdl = conidx;
        msg.param.disconnect_msg.reason   = reason;
        ble_cb_fun(msg);
    }
    if (app_evt_cb != NULL)
    {
        app_connect_status_ind_t status_ind;
        status_ind.connId = conidx;
        uint8_t * bd_addr = app_get_addr_by_conidx(conidx);
        if (NULL != bd_addr)
        {
            memmove(status_ind.addr, bd_addr, APP_BD_ADDR_LEN);
        }
        app_evt_cb(BLE_DEV_DISCONNECTED, (void *) &status_ind);
    }
    return CB_DONE;
}

void app_ble_gatt_add_srv_rsp(uint16_t handle)
{
    APP_TRC("APP: %s ,handle=%d,nb_att=%d \r\n", __FUNCTION__, handle, service_reg_env.reg_list[service_reg_env.reg_nb]->nb_att);

    service_reg_env.reg_list[service_reg_env.reg_nb]->state     = SONATA_SERVICE_ENABLE;
    service_reg_env.reg_list[service_reg_env.reg_nb]->start_hdl = handle;
    if (NULL != service_reg_env.reg_list[service_reg_env.reg_nb]->att_desc)
    {
        sonata_api_free(service_reg_env.reg_list[service_reg_env.reg_nb]->att_desc);
    }
    service_reg_env.reg_nb++;
    if (service_reg_env.add_nb != service_reg_env.reg_nb)
    {
        APP_TRC("add new service\r\n");
        uint8_t perm                  = service_reg_env.reg_list[service_reg_env.reg_nb]->perm;
        uint8_t * uuid                = service_reg_env.reg_list[service_reg_env.reg_nb]->uuid;
        uint8_t nb_att                = service_reg_env.reg_list[service_reg_env.reg_nb]->nb_att;
        sonata_gatt_att_desc_t * atts = service_reg_env.reg_list[service_reg_env.reg_nb]->att_desc;
        sonata_ble_gatt_add_service_request(service_reg_env.reg_list[service_reg_env.reg_nb]->start_hdl, perm, uuid, nb_att,
                                            &atts[1]);
    }
}

ble_gatt_att_reg_list_t * app_ble_get_reg_list_by_handle(uint16_t handle)
{
    // print_serv_env();
    for (int i = 0; i < service_reg_env.reg_nb; i++)
    {
        ble_gatt_att_reg_list_t * p_list = service_reg_env.reg_list[i];
        //printf("nb_att:%d\r\n", p_list->nb_att);
        if (p_list->start_hdl <= handle && (p_list->start_hdl + p_list->nb_att) >= handle)
        {
            return p_list;
        }
    }
    // printf("[API]get null handle\r\n");
    return NULL;
}

void app_ble_gatt_read_request_handler(uint16_t handle, uint16_t * p_length, uint8_t * p_value)
{
    APP_TRC("APP: %s ,handle=%d, \r\n", __FUNCTION__, handle);
    ble_gatt_att_reg_list_t * p_list = app_ble_get_reg_list_by_handle(handle);
    if (NULL == p_list)
    {
        return;
    }
    uint16_t localhandle = handle - p_list->start_hdl - 1;
    if (p_list->att_opr_list[localhandle].read_request != NULL)
    {
        if (p_list->vendor == 1)
        {
            p_list->att_opr_list[localhandle].read_request(p_value, p_length);
        }
        else
        {
            uint16_t temp_length = 0;
            uint8_t * temp_value = NULL;
            p_list->att_opr_list[localhandle].read_request((uint8_t *)&temp_value, &temp_length);
            *p_length = temp_length;
            if (temp_length > 0 && temp_length < 250)
            {
                memmove(p_value, temp_value, temp_length);
            }
        }
    }
}

void app_user_config_write_cb(uint16_t handle, uint8_t * data, uint16_t size)
{
    static uint16_t s_indicateEnable = 0;
    // printf("user_config_write_cb\r\n");
    if (size != sizeof(s_indicateEnable))
    { // Size check
        return;
    }
    s_indicateEnable = *(uint16_t *) data;
    if (s_indicateEnable == 0x0002)
    {
        int32_t send_status = sonata_ble_gatt_send_indicate_event(0, handle, size, data); // 0x0200
        printf("send status %ld\r\n", send_status);
    }
}

void app_ble_gatt_write_request_handler(uint16_t handle, uint16_t length, uint8_t * p_value)
{
    // printf("[API]ble_gatt_write_request_handler %d\r\n",handle);
    APP_TRC("APP: %s ,handle=%d,length=%d, \r\n", __FUNCTION__, handle, length);
    ble_gatt_att_reg_list_t * p_list = app_ble_get_reg_list_by_handle(handle);
    if (NULL == p_list)
    {
        return;
    }
    uint16_t localhandle = handle - p_list->start_hdl - 1;
    if (p_list->att_opr_list[localhandle].write_request != NULL)
    {
        p_list->att_opr_list[localhandle].write_request(p_value, length);
    }
    else
    {
        app_user_config_write_cb(handle, p_value, length); // TODO !!!!!!!!!!20200923
        // printf("[API] write NULL Pointer\r\n");
    }
}

void app_ble_gatt_get_att_info(uint16_t handle, uint16_t * p_length, uint8_t * p_status)
{
    APP_TRC("APP: %s ,handle=%d, \r\n", __FUNCTION__, handle);
    ble_gatt_att_reg_list_t * p_list = app_ble_get_reg_list_by_handle(handle);
    if (NULL == p_list)
    {
        *p_length = 0;
        *p_status = BLE_STATUS_FAILED;
        return;
    }
    *p_length = 200; // TODO, maybe need to record
    *p_status = BLE_STATUS_SUCCESS;
}

int32_t app_ble_gatt_data_send(uint8_t conidx, uint16_t local_handle, uint16_t idx, uint16_t length, uint8_t * p_value)
{
    int32_t send_status;
    uint16_t localhandle = 0;
    ble_gatt_att_reg_list_t * p_list = NULL;

    if (local_handle > service_reg_env.add_nb)
    {
        return -1;
    }
    p_list = service_reg_env.reg_list[local_handle];
    if (NULL == p_list)
    {
        return -1;
    }
    //idx = 5;
    localhandle = p_list->start_hdl + idx;
    send_status = sonata_ble_gatt_send_indicate_event(conidx, localhandle, length, p_value);
    APP_TRC("APP: %s ,conidx=%d,localhandle=%d,length=%d,send_status=%X, \r\n", __FUNCTION__, conidx, localhandle, length,send_status);
    return send_status;
}

int32_t app_ble_gatt_data_send_notify(uint16_t local_handle, uint16_t idx, uint16_t length, uint8_t * p_value)
{
    int32_t send_status;
    uint16_t localhandle = 0;
    ble_gatt_att_reg_list_t * p_list = NULL;

    if (local_handle > service_reg_env.add_nb)
    {
        return  -1;
    }
    p_list = service_reg_env.reg_list[local_handle];
    if (NULL == p_list)
    {
        return -1;
    }
    // idx = 5;
    localhandle = p_list->start_hdl + idx;
    send_status = sonata_ble_gatt_send_notify_event(0, localhandle, length, p_value);
    APP_TRC("APP: %s ,conidx=%d,localhandle=%d,length=%d,send_status=%X, \r\n", __FUNCTION__, 0, localhandle, length, send_status);
    return send_status;
}

static void app_ble_enable_service(uint16_t start_hdl, uint8_t perm)
{
    APP_TRC("APP: %s ,handle =%02X(X), perm=%X, \r\n", __FUNCTION__, start_hdl, perm);
    sonata_ble_gatt_set_service_visibility(start_hdl, true);
}

static void app_ble_disable_service(uint16_t start_hdl, uint8_t perm)
{
    APP_TRC("APP: %s ,handle =%02X(X), perm=%X, \r\n", __FUNCTION__, start_hdl, perm);
    sonata_ble_gatt_set_service_visibility(start_hdl, false);
}

int app_ble_disable_service_by_handler(uint16_t start_hdl)
{
    if (start_hdl > service_reg_env.add_nb)
    {
        return -1;
    }
    ble_gatt_att_reg_list_t * p_list = service_reg_env.reg_list[start_hdl];
    if (NULL == p_list)
    {
        return -1;
    }

    service_state[start_hdl] = SONATA_SERVICE_DISABLE;
    app_ble_disable_service(p_list->start_hdl, p_list->perm);
    return 0;
}

static uint8_t app_ble_search_svc(uint8_t * service_uuid)
{
    for (int idx = 0; idx < service_reg_env.add_nb; idx++)
    {
        if (!memcmp(service_uuid, service_reg_env.reg_list[idx]->uuid, SONATA_ATT_UUID_128_LEN))
        {
            return idx;
        }
    }
    return APP_MAX_SERVICE_NUM;
}

int app_ble_gatt_add_svc_helper(uint16_t * start_hdl, uint8_t nb_att, uint8_t vendor, ble_gatt_att_reg_t * atts)
{
    // printf("ble_gatt_add_svc_helper\r\n");
    APP_TRC("APP: %s ,nb_att=%d, \r\n", __FUNCTION__, nb_att);
    uint8_t perm = atts[0].att_desc.perm;
    // PERM_SET(perm, SVC_UUID_LEN,2);
    uint8_t uuid[SONATA_ATT_UUID_128_LEN];
    uint16_t svc_hdl = *start_hdl;
    uint8_t local_idx;
    memmove(uuid, atts[0].att_desc.uuid, SONATA_ATT_UUID_128_LEN); // the first should  servicce attr!!!
    local_idx = app_ble_search_svc(uuid);
    if (APP_MAX_SERVICE_NUM != local_idx)
    {
        APP_TRC("found  service %d\r\n", local_idx);
        if (service_reg_env.reg_list[local_idx]->state == SONATA_SERVICE_DISABLE)
        {

            service_state[local_idx] = SONATA_SERVICE_ENABLE;
            *start_hdl               = local_idx;
            app_ble_enable_service(service_reg_env.reg_list[local_idx]->start_hdl, service_reg_env.reg_list[local_idx]->perm);
            return BLE_STATUS_SUCCESS;
        }

        else if (service_reg_env.reg_list[local_idx]->state == SONATA_SERVICE_ENABLE)
        {
            APP_TRC("duplicate add service %d\r\n", local_idx);
            *start_hdl = local_idx;
            return BLE_STATUS_FAILED;
        }
    }
    APP_TRC("struct %d %d\r\n", sizeof(ble_gatt_att_reg_list_t), sizeof(ble_gatt_att_manager_t));
    ble_gatt_att_reg_list_t * p_list =
        (ble_gatt_att_reg_list_t *) sonata_api_malloc(sizeof(ble_gatt_att_opr_t) * (nb_att - 1) + sizeof(ble_gatt_att_reg_list_t));
    memset(p_list, 0, sizeof(ble_gatt_att_opr_t) * (nb_att - 1) + sizeof(ble_gatt_att_reg_list_t));
    p_list->att_opr_list = (ble_gatt_att_opr_t *) ((uint8_t *) p_list + sizeof(ble_gatt_att_reg_list_t));
    p_list->nb_att       = nb_att - 1;
    p_list->vendor       = vendor;
    APP_TRC("nb_att %d\r\n", p_list->nb_att);

    for (int i = 1; i < nb_att; i++)
    {
        memmove(p_list->att_opr_list + i - 1, &atts[i].att_opr, sizeof(ble_gatt_att_opr_t));
    };
    memmove(p_list->uuid, atts[0].att_desc.uuid, SONATA_ATT_UUID_128_LEN);
    p_list->perm                      = perm;
    sonata_gatt_att_desc_t * att_desc = sonata_api_malloc(nb_att * sizeof(sonata_gatt_att_desc_t));
    for (int i = 0; i < nb_att; ++i)
    {

        att_desc[i].perm     = atts[i].att_desc.perm;
        att_desc[i].max_len  = atts[i].att_desc.max_len;
        att_desc[i].ext_perm = atts[i].att_desc.ext_perm;
        // PERM_SET(msg->svc_desc.atts[i].ext_perm, UUID_LEN,2);
        memcpy(att_desc[i].uuid, atts[i].att_desc.uuid, SONATA_ATT_UUID_128_LEN);
    }
    APP_TRC("nb_att %d\r\n", p_list->nb_att);

#if (defined ALIOS_SUPPORT) || (defined HARMONYOS_SUPPORT)
    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
#endif
    uint8_t add_nb = service_reg_env.add_nb;
    service_reg_env.add_nb++;
    service_reg_env.reg_list[add_nb] = p_list;
    *start_hdl                       = add_nb;
    if (service_reg_env.add_nb != service_reg_env.reg_nb + 1)
    {
        service_reg_env.reg_list[add_nb]->att_desc = att_desc;
        APP_TRC("service adding %d %d\r\n", service_reg_env.add_nb, service_reg_env.reg_nb);
#if (defined ALIOS_SUPPORT) || (defined HARMONYOS_SUPPORT)
        lega_rtos_exit_critical();
#endif
        return BLE_STATUS_FAILED;
    }
#if (defined ALIOS_SUPPORT) || (defined HARMONYOS_SUPPORT)
    lega_rtos_exit_critical();
#endif
    // print_serv_env();

    sonata_ble_gatt_add_service_request(svc_hdl, perm, uuid, nb_att - 1, &att_desc[1]);
    sonata_api_free(att_desc);
    return BLE_STATUS_SUCCESS;
}

int app_set_security_io_cap(uint8_t cap)
{
    APP_TRC("APP: %s ,cap=%02X(X), \r\n", __FUNCTION__, cap);
    app_iocap = cap;
    return 0;
}

int app_set_security_auth_req(uint8_t auth_req)
{
    APP_TRC("APP: %s ,auth_req=%02X(X), \r\n", __FUNCTION__, auth_req);
    app_auth = auth_req;
    return 0;
}

uint8_t app_get_connection_state(void)
{
    return app_connection_state;
}

void app_set_connection_state(uint8_t state)
{
    app_connection_state = state;
}

static void print_peer_bond_request(struct sonata_gap_bond_req_ind * request)
{
    switch (request->request)
    {
    case SONATA_GAP_PAIRING_REQ:
        APP_TRC("PEER_PAIR: SONATA_GAP_PAIRING_REQ,auth_req=%02X(X)", request->data.auth_req);
        switch (request->data.auth_req)
        {
        case SONATA_GAP_AUTH_REQ_NO_MITM_NO_BOND:
            APP_TRC(" (GAP_AUTH_REQ_NO_MITM_NO_BOND)\r\n");
            break;
        case SONATA_GAP_AUTH_REQ_NO_MITM_BOND:
            APP_TRC(" (GAP_AUTH_REQ_NO_MITM_BOND)\r\n");
            break;
        case SONATA_GAP_AUTH_REQ_MITM_NO_BOND:
            APP_TRC(" (GAP_AUTH_REQ_MITM_NO_BOND)\r\n");
            break;
        case SONATA_GAP_AUTH_REQ_MITM_BOND:
            APP_TRC(" (GAP_AUTH_REQ_MITM_BOND)\r\n");
            break;
        case SONATA_GAP_AUTH_REQ_SEC_CON_NO_BOND:
            APP_TRC(" (GAP_AUTH_REQ_SEC_CON_NO_BOND)\r\n");
            break;
        case SONATA_GAP_AUTH_REQ_SEC_CON_BOND:
            APP_TRC(" (GAP_AUTH_REQ_SEC_CON_BOND)\r\n");
            break;
        default:
            APP_TRC(" (Default)\r\n");
            break;
        }
        break;
    case SONATA_GAP_TK_EXCH:
        APP_TRC("PEER_PAIR: SONATA_GAP_TK_EXCH,tk_type=%02X(X)\r\n", request->data.tk_type);
        switch (request->data.auth_req)
        {
        case SONATA_GAP_TK_OOB:
            APP_TRC(" (GAP_TK_OOB)\r\n");
            break;
        case SONATA_GAP_TK_DISPLAY:
            APP_TRC(" (GAP_TK_DISPLAY)\r\n");
            break;
        case SONATA_GAP_TK_KEY_ENTRY:
            APP_TRC(" (GAP_TK_KEY_ENTRY)\r\n");
            break;
        default:
            APP_TRC(" (Default)\r\n");
            break;
        }
        break;
    case SONATA_GAP_LTK_EXCH:
        APP_TRC("PEER_PAIR: SONATA_GAP_LTK_EXCH,key_size=%02X(X)\r\n", request->data.key_size);
        break;
    case SONATA_GAP_OOB_EXCH:
        APP_TRC("PEER_PAIR: \r\n");
        break;
    case SONATA_GAP_NC_EXCH:
        APP_TRC("PEER_PAIR: SONATA_GAP_NC_EXCH,NC Value:%02X %02X %02X %02X\r\n", request->data.nc_data.value[0],
                request->data.nc_data.value[1], request->data.nc_data.value[2], request->data.nc_data.value[3]);
        break;
    }
}

void app_gap_notify_pair_request_rsp(uint8_t * bd_addr, uint8_t accept)
{
    uint8_t key_size                = SONATA_GAP_SMP_MAX_ENC_SIZE_LEN;
    enum sonata_gap_oob_auth oob    = SONATA_GAP_OOB_AUTH_DATA_NOT_PRESENT;
    enum sonata_gap_kdist ikey_dist = SONATA_GAP_KDIST_LINKKEY; // Initiator key distribution
    enum sonata_gap_kdist rkey_dist = SONATA_GAP_KDIST_NONE;    // Responder key distribution
    enum sonata_gap_sec_req sec_req = SONATA_GAP_NO_SEC;
    // ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;
    //  ikey_dist = GAP_KDIST_NONE;
    //  rkey_dist = GAP_KDIST_NONE;
    ikey_dist      = SONATA_GAP_KDIST_ENCKEY | SONATA_GAP_KDIST_IDKEY; // Initiator key distribution
    rkey_dist      = SONATA_GAP_KDIST_ENCKEY | SONATA_GAP_KDIST_IDKEY; // Responder key distribution
    sec_req        = SONATA_GAP_NO_SEC;
    uint8_t conidx = app_get_conidx_by_addr(bd_addr);
    if (conidx == APP_BLE_FUN_ERROR)
    {
        APP_TRC("APP: %s  error, conidx=%d\r\n", __FUNCTION__, conidx);
        return;
    }
    sonata_ble_gap_send_bond_cfm_for_pairing_req(conidx, NULL, accept, app_iocap, oob, app_auth, key_size, ikey_dist, rkey_dist,
                                                 sec_req);
    if (accept)
    {
        app_set_connection_state(APP_BONDING);
    }
    else
    {
        APP_TRC("APP: %s  accept %d\r\n", __FUNCTION__, accept);
    }
}

static void app_gap_notify_pair_request(void)
{
    if (sec_req_call == NULL)
    {
        app_gap_notify_pair_request_rsp(gTargetAddr, 1); // todo
    }
    else
    {
        sec_req_call(gTargetAddr); // todo
    }
}

static uint16_t app_gap_bond_req_callback(uint8_t conidx, struct sonata_gap_bond_req_ind * request)
{
    APP_TRC("APP: %s  request->request=%d\r\n", __FUNCTION__, request->request);
    print_peer_bond_request(request);
    switch (request->request)
    {
    case SONATA_GAP_PAIRING_REQ: {
        APP_TRC("APP: %s  SONATA_GAP_PAIRING_REQ\r\n", __FUNCTION__);
        app_req_auth = request->data.auth_req;
        app_gap_notify_pair_request();
    }
    break;
    case SONATA_GAP_LTK_EXCH: {
        APP_TRC("APP: %s  SONATA_GAP_LTK_EXCH\r\n", __FUNCTION__);
        uint8_t counter                = 0;
        struct sonata_gap_ltk data_ltk = { 0 };
        uint8_t accept                 = 1;
        // uint8_t request = SONATA_GAP_LTK_EXCH;

        // Generate all the values
        data_ltk.ediv = (uint16_t) util_rand_word();
        for (counter = 0; counter < APP_GAP_RAND_NB_LEN; counter++)
        {
            data_ltk.ltk.key[counter]   = (uint8_t) util_rand_word();
            data_ltk.randnb.nb[counter] = (uint8_t) util_rand_word();
        }
        for (counter = APP_GAP_RAND_NB_LEN; counter < APP_GAP_KEY_LEN; counter++)
        {
            data_ltk.ltk.key[counter] = (uint8_t) util_rand_word();
        }
        APP_TRC("APP  %s, app_ltk_key  :", __FUNCTION__);
        for (int i = 0; i < APP_GAP_KEY_LEN; ++i)
        {
            APP_TRC("%02X ", data_ltk.ltk.key[i]);
        }
        APP_TRC("\r\n");
        APP_TRC("APP  %s, app_randnb_nb:", __FUNCTION__);
        for (int i = 0; i < APP_GAP_RAND_NB_LEN; ++i)
        {
            APP_TRC("%02X ", data_ltk.randnb.nb[i]);
        }
        APP_TRC("\r\n");
        if (sonata_fs_write(SONATA_TAG_LTK, SONATA_LEN_LTK, (uint8_t *) &data_ltk) != SONATA_FS_OK)
        {
            APP_TRC("LTK Save fail!!!\r\n");
        }
        sonata_ble_gap_send_bond_cfm_for_ltk_exchange(conidx, accept, data_ltk.ediv, data_ltk.randnb.nb, APP_GAP_KEY_LEN,
                                                      data_ltk.ltk.key);

        APP_TRC(" bonded_dev_info.current_dev_index = %d \r\n", bonded_dev_info.current_dev_index);
        memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].peer_addr, peer_dbaddr, APP_BD_ADDR_LEN);
        memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].ltk.ltk, data_ltk.ltk.key, APP_GAP_KEY_LEN);
        memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].ltk.randnb, data_ltk.randnb.nb, 8);
        bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].ltk.ediv = data_ltk.ediv;

        // Store the bonded dev info value in FS
        if (sonata_fs_write(SONATA_TAG_BONDED_DEV_INFO, SONATA_LEN_BONDED_DEV_INFO, (uint8_t *) &bonded_dev_info) != SONATA_FS_OK)
        {
            ASSERT_ERR(3695, 0);
        }
    }
    break;
    case SONATA_GAP_IRK_EXCH: {
        APP_TRC("APP: %s  SONATA_GAP_IRK_EXCH \r\n", __FUNCTION__);
        // Store the peer addr into FS
        uint8_t accept = true;
        /*uint8_t length = SONATA_LEN_PEER_BD_ADDRESS;
        uint8_t  addr[APP_BD_ADDR_LEN] = {0};
        if (sonata_fs_read(SONATA_TAG_PEER_BD_ADDRESS, &length, addr) == SONATA_FS_OK)
        {
            uint8_t addr_type = (addr[5] & 0xC0) ? ADDR_RAND : ADDR_PUBLIC;
            sonata_ble_gap_send_bond_cfm_for_irk_exchange(conidx, accept, app_loc_irk, addr_type,addr);
        }*/
        uint8_t * local_addr = sonata_get_bt_address();
        uint8_t addr_type    = (local_addr[5] & 0xC0) ? SONATA_ADDR_RAND : SONATA_ADDR_PUBLIC;
        sonata_ble_gap_send_bond_cfm_for_irk_exchange(conidx, accept, app_loc_irk, addr_type, local_addr);
    }
    break;
    case SONATA_GAP_TK_EXCH: {
        APP_TRC("APP: %s  SONATA_GAP_TK_EXCH\r\n", __FUNCTION__);

        uint8_t data_tk[APP_GAP_KEY_LEN] = { 0 };
        // Generate a PIN Code- (Between 100000 and 999999)
        uint32_t pin_code = (100000 + (util_rand_word() % 900000));
        uint8_t accept    = true;

        data_tk[0] = (uint8_t) ((pin_code & 0x000000FF) >> 0);
        data_tk[1] = (uint8_t) ((pin_code & 0x0000FF00) >> 8);
        data_tk[2] = (uint8_t) ((pin_code & 0x00FF0000) >> 16);
        data_tk[3] = (uint8_t) ((pin_code & 0xFF000000) >> 24);
        sonata_ble_gap_send_bond_cfm_for_tk_exchange(conidx, accept, data_tk);
        APP_TRC("APP: %s  pin_code=%lu,", __FUNCTION__, pin_code);
        APP_TRC(" TKey:");

        for (int i = 0; i < APP_GAP_KEY_LEN; ++i)
        {
            APP_TRC("%02X ", data_tk[i]);
        }
        APP_TRC("\r\n");
    }
    break;
    case SONATA_GAP_NC_EXCH: {
        APP_TRC("APP: %s  SONATA_GAP_NC_EXCH\r\n", __FUNCTION__);
        APP_TRC("APP  %s, NC Value:", __FUNCTION__);
        for (int i = 0; i < 4; ++i)
        {
            APP_TRC("%02X ", request->data.nc_data.value[i]);
        }
        APP_TRC("\r\n");
        uint8_t accept = 1;
        sonata_ble_gap_send_bond_cfm_for_nc_exchange(conidx, accept);
    }
    break;
    case SONATA_GAP_CSRK_EXCH:
        APP_TRC("APP: %s  SONATA_GAP_CSRK_EXCH\r\n", __FUNCTION__);
        uint8_t accept = true;
        sonata_ble_gap_send_bond_cfm_for_csrk_exchange(conidx, accept, app_csrk);

        break;
    case SONATA_GAP_OOB_EXCH:
        APP_TRC("APP: %s  SONATA_GAP_OOB_EXCH\r\n", __FUNCTION__);

        break;
    }

    return CB_DONE;
}

static uint16_t app_gap_bond_callback(uint8_t conidx, struct sonata_gap_bond_ind * ind)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    switch (ind->info)
    {
    case SONATA_GAP_PAIRING_SUCCEED:
        APP_TRC("APP: %s  SONATA_GAP_PAIRING_SUCCEED,", __FUNCTION__);
        APP_TRC(" app_bond = 1,");
        app_bond = 1;
        if (sonata_fs_write(SONATA_TAG_PERIPH_BONDED, SONATA_LEN_PERIPH_BONDED, (uint8_t *) &app_bond) != SONATA_FS_OK)
        {
            // An error has occurred during access to the FS
            ASSERT_ERR(3697, 0);
        }
        struct sonata_gap_bdaddr * bdaddr = sonata_ble_gap_get_bdaddr(conidx, SONATA_GAP_SMP_INFO_PEER);
        if (sonata_fs_write(SONATA_TAG_PEER_BD_ADDRESS, SONATA_LEN_PEER_BD_ADDRESS, bdaddr->addr.addr) != SONATA_FS_OK)
        {
            // An error has occurred during access to the FS
            // ASSERT_ERR(3694, 0);
        }
        else
        {
            APP_TRC("peer_addr:");
            for (int i = APP_BD_ADDR_LEN - 1; i >= 0; --i)
            {
                APP_TRC("%02X ", bdaddr->addr.addr[i]);
            }
            APP_TRC(" \r\n");
        }

        switch (ind->data.auth.info)
        {
        case SONATA_GAP_AUTH_REQ_NO_MITM_NO_BOND:
            APP_TRC("Auth:GAP_AUTH_REQ_NO_MITM_NO_BOND \r\n");
            break;
        case SONATA_GAP_AUTH_REQ_NO_MITM_BOND:
            APP_TRC("Auth:GAP_AUTH_REQ_NO_MITM_BOND \r\n");
            break;
        case SONATA_GAP_AUTH_REQ_MITM_NO_BOND:
            APP_TRC("Auth:GAP_AUTH_REQ_MITM_NO_BOND \r\n");
            break;
        case SONATA_GAP_AUTH_REQ_MITM_BOND:
            APP_TRC("Auth:GAP_AUTH_REQ_MITM_BOND \r\n");
            break;
        case SONATA_GAP_AUTH_REQ_SEC_CON_NO_BOND:
            APP_TRC("Auth:GAP_AUTH_REQ_SEC_CON_NO_BOND \r\n");
            break;
        case SONATA_GAP_AUTH_REQ_SEC_CON_BOND:
            APP_TRC("Auth:GAP_AUTH_REQ_SEC_CON_BOND \r\n");
            break;
        }
        app_set_connection_state(APP_BONDED);
        if (bonded_dev_info.current_dev_index < MAX_BONDED_DEV_INDEX)
        {
            bonded_dev_info.current_dev_index++;
        }
        else if (bonded_dev_info.current_dev_index == MAX_BONDED_DEV_INDEX)
        {
            bonded_dev_info.current_dev_index = 0;
        }
        break;
    case SONATA_GAP_PAIRING_FAILED:
        // Reason see (SONATA_GAP_SMP_REM_ERR_MASK|smp_pair_fail_reason)
        APP_TRC("APP: %s  SONATA_GAP_PAIRING_FAILED,Reason:%02X(X)\r\n", __FUNCTION__, ind->data.reason);
        // app_ble_config_scanning();
        // app_stop_scan_timer_start();
        // sonata_ble_gap_start_security(conidx, GAP_AUTH_REQ_MITM_BOND);
        sonata_ble_gap_disconnect(conidx, SONATA_CO_ERROR_CONN_REJ_SECURITY_REASONS);
        break;
    case SONATA_GAP_LTK_EXCH:
        APP_TRC("APP: %s  SONATA_GAP_LTK_EXCH\r\n", __FUNCTION__);
        if ((app_req_auth & SONATA_GAP_AUTH_SEC_CON) && (app_auth & SONATA_GAP_AUTH_SEC_CON))
        {
            memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].ltk.ltk, ind->data.ltk.ltk.key,
                   APP_GAP_KEY_LEN);
        }
        memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].ltk_in, ind->data.ltk.ltk.key,
               APP_GAP_KEY_LEN);
        // memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].ltk.randnb, ind->data.randnb.nb, 8);
        // bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].ltk.ediv = ind->data.ediv;
        // Store the app_ltk_key_in value in FS
        if (sonata_fs_write(SONATA_TAG_BONDED_DEV_INFO, SONATA_LEN_BONDED_DEV_INFO, (uint8_t *) &bonded_dev_info) != SONATA_FS_OK)
        {
            ASSERT_ERR(3698, 0);
        }
        APP_TRC("AAA  %s, app_ltk_key_in:", __FUNCTION__);
        for (int i = 0; i < APP_GAP_KEY_LEN; ++i)
        {
            APP_TRC("%02X ", ind->data.ltk.ltk.key[i]);
        }
        APP_TRC("\r\n");

        break;
    case SONATA_GAP_IRK_EXCH:
        APP_TRC("APP: %s  SONATA_GAP_IRK_EXCH\r\n", __FUNCTION__);
        memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].irk.irk_addr, ind->data.irk.addr.addr.addr,
               APP_BD_ADDR_LEN);
        memcpy(bonded_dev_info.bonded_device_info[bonded_dev_info.current_dev_index].irk.irk, ind->data.irk.irk.key,
               APP_GAP_KEY_LEN);
        // Store the irk addr value in FS
        if (sonata_fs_write(SONATA_TAG_BONDED_DEV_INFO, SONATA_LEN_BONDED_DEV_INFO, (uint8_t *) &bonded_dev_info) != SONATA_FS_OK)
        {
            ASSERT_ERR(3700, 0);
        }
        break;
    case SONATA_GAP_CSRK_EXCH:
        APP_TRC("APP: %s  SONATA_GAP_CSRK_EXCH\r\n", __FUNCTION__);
        break;
    case SONATA_GAP_REPEATED_ATTEMPT:
        APP_TRC("APP: %s  SONATA_GAP_REPEATED_ATTEMPT\r\n", __FUNCTION__);
        sonata_ble_gap_disconnect(conidx, SONATA_CO_ERROR_REMOTE_USER_TERM_CON);
        break;
    }
    return CB_DONE;
}

uint8_t app_check_device_isbonded(uint16_t in_ediv, uint8_t * in_nb)
{
    APP_TRC("APP: %s \r\n", __FUNCTION__);
    // check the latest device first
    for (int i = bonded_dev_info.current_dev_index; i < MAX_BONDED_DEV_NUM; i++)
    {
        if (in_ediv == bonded_dev_info.bonded_device_info[i].ltk.ediv &&
            !memcmp(&in_nb[0], bonded_dev_info.bonded_device_info[i].ltk.randnb, APP_GAP_RAND_NB_LEN))
        {
            // APP_TRC("APP: %s 00 i = %d\r\n", __FUNCTION__, i);
            return i;
        }
    }
    for (int i = 0; i < bonded_dev_info.current_dev_index; i++)
    {
        if (in_ediv == bonded_dev_info.bonded_device_info[i].ltk.ediv &&
            !memcmp(&in_nb[0], bonded_dev_info.bonded_device_info[i].ltk.randnb, APP_GAP_RAND_NB_LEN))
        {
            // APP_TRC("APP: %s 11 i = %d\r\n", __FUNCTION__, i);
            return i;
        }
    }
    return INVALID_BONDED_INDEX;
}

static uint16_t app_gap_encrypt_req_callback(uint8_t conidx, uint16_t in_ediv, uint8_t * in_nb)
{
    APP_TRC("APP: %s in_ediv=%X(x), in_nb:", __FUNCTION__, in_ediv);
    for (int i = 0; i < APP_GAP_RAND_NB_LEN; ++i)
    {
        APP_TRC("%02X ", in_nb[i]);
    }
    APP_TRC("\r\n");
    uint8_t length = SONATA_LEN_PERIPH_BONDED;
    if (sonata_fs_read(SONATA_TAG_PERIPH_BONDED, &length, (uint8_t *) &app_bond) != SONATA_FS_OK)
    {
        app_bond = 0;
    }
    if (app_bond)
    {
        uint8_t found    = 0;
        uint16_t keySize = APP_GAP_KEY_LEN;
        length           = SONATA_LEN_BONDED_DEV_INFO;

        if (sonata_fs_read(SONATA_TAG_BONDED_DEV_INFO, &length, (uint8_t *) &bonded_dev_info) == SONATA_FS_OK)
        {
            // Check if the provided EDIV and Rand Nb values match with the stored values
            uint8_t index = app_check_device_isbonded(in_ediv, in_nb);
            if (index != INVALID_BONDED_INDEX)
            {
                APP_TRC("APP: %s, found, send encrypt confirm, key:", __FUNCTION__);
                for (int i = 0; i < APP_GAP_KEY_LEN; ++i)
                {
                    APP_TRC("%02X ", bonded_dev_info.bonded_device_info[index].ltk.ltk[i]);
                }
                APP_TRC("\r\n");
                found = true;
                sonata_ble_gap_send_encrypt_cfm(conidx, found, keySize, bonded_dev_info.bonded_device_info[index].ltk.ltk);
                app_set_connection_state(APP_BONDED);
            }
            else
            {
                APP_TRC("APP: %s, not found, send encrypt confirm\r\n", __FUNCTION__);
                uint8_t app_ltk_key_zero[APP_GAP_KEY_LEN] = { 0 };
                sonata_ble_gap_send_encrypt_cfm(conidx, found, keySize, app_ltk_key_zero);
            }
        }
        else
        {
            return CB_REJECT;
            APP_TRC("Error when read LTK in FS!!!\r\n");
        }
    }
    else
    {
        APP_TRC("APP: %s, not bond, send encrypt confirm\r\n", __FUNCTION__);
        uint8_t app_ltk_key_zero[APP_GAP_KEY_LEN] = { 0 };
        sonata_ble_gap_send_encrypt_cfm(conidx, false, 0, app_ltk_key_zero);
    }

    return CB_DONE;
}

static uint16_t app_gap_gen_random_number_callback(uint8_t * number)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    if (app_rand_cnt == 1) // First part of IRK
    {
        memcpy(&app_loc_irk[0], number, 8);
    }
    else if (app_rand_cnt == 2) // Second part of IRK
    {
        memcpy(&app_loc_irk[8], number, 8);
    }
    APP_TRC("app_loc_irk :");
    for (int i = 0; i < APP_GAP_KEY_LEN; ++i)
    {
        APP_TRC("%02X ", app_loc_irk[i]);
    }
    APP_TRC("\r\n");
    return CB_DONE;
}

static uint16_t app_gap_security_callback(uint8_t conidx, uint8_t auth_level)
{
    APP_TRC("APP: %s  auth_level=%02X(x)\r\n", __FUNCTION__, auth_level);

    return CB_DONE;
}

static uint16_t app_gap_encrypt_callback(uint8_t conidx, uint8_t auth_level)
{
    APP_TRC("APP: %s  auth_level=%02X(x)\r\n", __FUNCTION__, auth_level);

    return CB_DONE;
}

static uint16_t app_gap_le_pke_size_callback(uint8_t conidx, uint16_t max_tx_octets, uint16_t max_tx_time, uint16_t max_rx_octets,
                                             uint16_t max_rx_time)
{
    APP_TRC("APP: %s  %d max_tx_octets=%d(d), max_rx_octets=%d(d) time %d %d\r\n", __FUNCTION__, conidx, max_tx_octets,
            max_rx_octets, max_tx_time, max_rx_time);

    return CB_DONE;
}
static uint16_t app_gap_resolving_address_callback(uint8_t operation, uint8_t addr_type, uint8_t * addr)
{
    APP_TRC("APP: %s  operation=%d(d), addr_type=%d(d)\r\n", __FUNCTION__, operation, addr_type);
    if (NULL == addr)
    {
        APP_TRC("ERROR\r\n");
    }
    return CB_DONE;
}

static uint16_t app_gatt_disc_svc_callback(uint8_t connection_id, uint16_t start_hdl, uint16_t end_hdl, uint8_t uuid_len,
                                           uint8_t * uuid)
{
    APP_TRC("APP: %s, start_hdl=0x%04X, end_hdl =0x%04X, uuid=", __FUNCTION__, start_hdl, end_hdl);
    for (int i = 0; i < uuid_len; ++i)
    {
        APP_TRC("%02X", uuid[i]);
    }
    APP_TRC("\r\n");
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg          = { 0 };
        msg.event_type                   = APP_BLE_STACK_EVENT_DISC_SVC_REPORT;
        msg.param.disc_svc_msg.conn_hdl  = connection_id;
        msg.param.disc_svc_msg.start_hdl = start_hdl;
        msg.param.disc_svc_msg.end_hdl   = end_hdl;
        msg.param.disc_svc_msg.uuid_len  = uuid_len;
        msg.param.disc_svc_msg.uuid      = uuid;
        ble_cb_fun(msg);
    }
    return CB_DONE;
}

/*!
 * @brief Discover characteristic callback
 * @param conidx
 * @param attr_hdl
 * @param pointer_hdl
 * @param prop
 * @param uuid_len
 * @param uuid
 * @return
 */
static uint16_t app_gatt_disc_char_callback(uint8_t conidx, uint16_t attr_hdl, uint16_t pointer_hdl, uint8_t prop, uint8_t uuid_len,
                                            uint8_t * uuid)
{
    uint16_t char_uuid = 0;
    // APP_TRC("APP_CB: %s, attr_hdl=0x%04X, uuid=", __FUNCTION__, attr_hdl);
    for (int i = 0; i < uuid_len; ++i)
    {
        // APP_TRC("%02X", uuid[i]);
        if (i == 0)
        {
            char_uuid = uuid[i];
        }
        if (i == 1)
        {
            char_uuid = uuid[i] << 8 | char_uuid;
        }
    }
    APP_TRC("APP: %s,char_uuid = %04X,attr_hdl=%04X,prop=%02X, target Read:%04X, Write=%04X\r\n", __FUNCTION__, char_uuid, attr_hdl,
            prop, gAppEnv.appUuids.read, gAppEnv.appUuids.write);
    if (char_uuid == gAppEnv.appUuids.read)
    {
        gAppEnv.attrHandle       = attr_hdl;
        gAppEnv.targetReadHandle = attr_hdl + 1;
        APP_TRC("APP: %s, ***Find targetReadHandle =%d, UUID=%04X\r\n", __FUNCTION__, gAppEnv.targetReadHandle, char_uuid);
    }
    if (char_uuid == gAppEnv.appUuids.write)
    {
        gAppEnv.targetWriteHandle = attr_hdl + 1;
        APP_TRC("APP: %s, ***Find targetWriteHandle=%d, UUID=%04X\r\n", __FUNCTION__, gAppEnv.targetWriteHandle, char_uuid);
    }
    if (char_uuid == gAppEnv.appUuids.ntf)
    {
        gAppEnv.targetNtfHandle = attr_hdl + 2;
        APP_TRC("APP: %s, ***Find targetNtfHandle  =%d, UUID=%04X\r\n", __FUNCTION__, gAppEnv.targetNtfHandle, char_uuid);
    }
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg             = { 0 };
        msg.event_type                      = APP_BLE_STACK_EVENT_DISC_CHAR_REPORT;
        msg.param.disc_char_msg.conn_hdl    = conidx;
        msg.param.disc_char_msg.att_hdl     = attr_hdl;
        msg.param.disc_char_msg.pointer_hdl = pointer_hdl;
        msg.param.disc_char_msg.prop        = prop;
        msg.param.disc_char_msg.uuid_len    = uuid_len;
        msg.param.disc_char_msg.uuid        = uuid;
        ble_cb_fun(msg);
    }
    return CB_DONE;
}

/*!
 * @brief Discover description callback
 * @param conidx
 * @param attr_hdl
 * @param uuid_len
 * @param uuid
 * @return
 */
static uint16_t app_gatt_disc_desc_callback(uint8_t conidx, uint16_t attr_hdl, uint8_t uuid_len, uint8_t * uuid)
{
    APP_TRC("APP: %s, attr_hdl=0x%04X, uuid=", __FUNCTION__, attr_hdl);
    for (int i = 0; i < uuid_len; ++i)
    {
        APP_TRC("%02X", uuid[i]);
    }
    APP_TRC("\r\n");
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg          = { 0 };
        msg.event_type                   = APP_BLE_STACK_EVENT_DISC_DESC_REPORT;
        msg.param.disc_desc_msg.conn_hdl = conidx;
        msg.param.disc_desc_msg.att_hdl  = attr_hdl;
        msg.param.disc_desc_msg.uuid_len = uuid_len;
        msg.param.disc_desc_msg.uuid     = uuid;
        ble_cb_fun(msg);
    }
    return CB_DONE;
}

/*!
 * @brief
 * @param connection_id
 * @param handle
 * @param offset
 * @param length
 * @param value
 */
static uint16_t app_gatt_read_callback(uint8_t connection_id, uint16_t handle, uint16_t offset, uint16_t length, uint8_t * value)
{
    APP_TRC("APP: %s, con=%d, handle=0x%04X, offset=0x%04X, VALUE=", __FUNCTION__, connection_id, handle, offset);
    for (int i = 0; i < length; ++i)
    {
        APP_TRC("%02x", value[i]);
    }
    APP_TRC("\r\n");
    return CB_DONE;
}

/*!
 * @brief
 * @param info
 */
static uint16_t app_gap_peer_att_info_callback(uint8_t conidx, sonata_gap_peer_att_info_ind_t * info)
{
    APP_TRC("APP: %s  conidx=%d\r\n", __FUNCTION__, conidx);
    sonata_gap_peer_att_info_ind_t * att_info = (sonata_gap_peer_att_info_ind_t *) info;
    APP_TRC("APP: %s, req =0x%02X\r\n", __FUNCTION__, att_info->req);
    APP_TRC("APP: %s, handle =0x%02X\r\n", __FUNCTION__, att_info->handle);
    switch (att_info->req)
    {
    case SONATA_GAP_DEV_NAME: {
        APP_TRC("APP  %s, Name:", __FUNCTION__);
        for (int i = 0; i < att_info->info.name.length; ++i)
        {
            APP_TRC("%c", att_info->info.name.value[i]);
        }
        APP_TRC("\r\n");
    }
    break;
    case SONATA_GAP_DEV_APPEARANCE: {
        APP_TRC("APP: %s, appearance =0x%04X\r\n", __FUNCTION__, att_info->info.appearance);
    }
    break;
    case SONATA_GAP_DEV_SLV_PREF_PARAMS: {
        APP_TRC("APP: %s, con_intv_min =0x%02X\r\n", __FUNCTION__, att_info->info.slv_pref_params.con_intv_min);
        APP_TRC("APP: %s, con_intv_max =0x%02X\r\n", __FUNCTION__, att_info->info.slv_pref_params.con_intv_max);
        APP_TRC("APP: %s, slave_latency =0x%02X\r\n", __FUNCTION__, att_info->info.slv_pref_params.slave_latency);
        APP_TRC("APP: %s, conn_timeout =0x%02X\r\n", __FUNCTION__, att_info->info.slv_pref_params.conn_timeout);
    }
    break;
    case SONATA_GAP_DEV_CTL_ADDR_RESOL: {
        APP_TRC("APP: %s, ctl_addr_resol =0x%02X\r\n", __FUNCTION__, att_info->info.ctl_addr_resol);
    }
    break;

    default:
        break;
    }
    return CB_DONE;
}

static uint16_t app_gap_peer_info_callback(uint8_t conidx, sonata_gap_peer_info_ind_t * info)
{
    APP_TRC("APP: %s  conidx=%d, info_type=%02X\r\n", __FUNCTION__, conidx, info->req);
    switch (info->req)
    {
    case SONATA_GET_PEER_VERSION:
        APP_TRC("APP: SONATA_GET_PEER_VERSION, compid:%04X,lmp_subvers:%04X,lmp_vers:%02X,\r\n", info->info.version.compid,
                info->info.version.lmp_subvers, info->info.version.lmp_vers);
        break;
    case SONATA_GET_PEER_FEATURES:
        APP_TRC("APP: SONATA_GET_PEER_FEATURES, features:");
        for (int i = 0; i < SONATA_GAP_LE_FEATS_LEN; ++i)
        {
            APP_TRC("%02x ", info->info.features.features[i]);
        }
        APP_TRC("\r\n");
        break;
    case SONATA_GET_PEER_CON_RSSI:
        APP_TRC("APP: SONATA_GET_PEER_CON_RSSI, rssi:%04X\r\n", info->info.rssi.rssi);

        break;
    case SONATA_GET_PEER_CON_CHANNEL_MAP:
        APP_TRC("APP: SONATA_GET_PEER_CON_CHANNEL_MAP, map:");
        for (int i = 0; i < SONATA_GAP_LE_CHNL_MAP_LEN; ++i)
        {
            APP_TRC("%02x ", info->info.channel_map.ch_map.map[i]);
        }
        APP_TRC("\r\n");
        break;
    case SONATA_GET_LE_PING_TO:
        APP_TRC("APP: SONATA_GET_LE_PING_TO, timeout:%04X,\r\n", info->info.ping_to_value.timeout);
        break;
    case SONATA_GET_PHY:
        APP_TRC("APP: SONATA_GET_PHY, tx_phy:%02X, rx_phy:%02X\r\n", info->info.le_phy.tx_phy, info->info.le_phy.rx_phy);
        break;
    case SONATA_GET_CHAN_SEL_ALGO:
        APP_TRC("APP: SONATA_GET_CHAN_SEL_ALGO, chan_sel_algo:%04X,\r\n", info->info.sel_algo.chan_sel_algo);
        break;
    default:
        break;
    }

    return CB_DONE;
}

/*!
 * @brief
 * @param conidx
 * @param intv_min
 * @param intv_max
 * @param latency
 * @param time_out
 */
static uint16_t app_gap_param_update_req_callback(uint8_t conidx, uint16_t intv_min, uint16_t intv_max, uint16_t latency,
                                                  uint16_t time_out)
{
    APP_TRC("APP: %s  conidx=%d,intv_min=%04X,intv_max=%04X,latency=%04X,time_out=%04X\r\n", __FUNCTION__, conidx, intv_min,
            intv_max, latency, time_out);
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg                = { 0 };
        msg.event_type                         = APP_BLE_STACK_EVENT_CONNECT_PARAM_UPDATE_REQ;
        msg.param.conn_param_msg.conn_hdl      = conidx;
        msg.param.conn_param_msg.conn_intv_max = intv_max;
        msg.param.conn_param_msg.conn_intv_min = intv_min;
        msg.param.conn_param_msg.slave_latency = latency;
        msg.param.conn_param_msg.timeout       = time_out;
        ble_cb_fun(msg);
    }
    else
    {
        sonata_ble_gap_send_param_update_cfm(conidx, true, 2, 4);
    }
    return CB_DONE;
}

/*!
 * @brief
 * @param conidx
 * @param con_interval
 * @param con_latency
 * @param sup_to
 * @return
 */
static uint16_t app_gap_param_updated_callback(uint8_t conidx, uint16_t con_interval, uint16_t con_latency, uint16_t sup_to)
{
    APP_TRC("APP: %s  conidx=%d,con_interval=%04X,con_latency=%04X,sup_to=%04X\r\n", __FUNCTION__, conidx, con_interval,
            con_latency, sup_to);

    return CB_DONE;
}

/*!
 * @brief
 * @param conidx
 * @param length
 * @param name
 * @return
 */
static uint16_t app_gap_peer_set_local_device_name_callback(uint8_t conidx, uint16_t length, uint8_t * name)
{
    sonata_ble_gap_send_set_dev_name_cfm(conidx, SONATA_GAP_ERR_REJECTED&0xFF);
    APP_TRC("name:");
    for (int i = 0; i < length; ++i)
    {
        APP_TRC("%02X ", name[i]);
    }
    APP_TRC(" \r\n");
    return CB_DONE;
}

void app_set_connect_flag(uint8_t vaule)
{
    need_connect_confirm = vaule;
}

static void app_gap_send_connection_confirm(uint8_t conidx, uint8_t auth)
{
    sonata_gap_connection_cfm_t connectionCfm = { 0 };
    // Get bond status from FS
    if (app_bond == 1)
    {
        connectionCfm.auth = SONATA_GAP_AUTH_REQ_NO_MITM_BOND;
        APP_TRC("APP: %s  connectionCfm.auth=GAP_AUTH_REQ_NO_MITM_BOND\r\n", __FUNCTION__);
    }
    else
    {
        connectionCfm.auth = SONATA_GAP_AUTH_REQ_NO_MITM_NO_BOND;
        APP_TRC("APP: %s  connectionCfm.auth=GAP_AUTH_REQ_SEC_CON_BOND\r\n", __FUNCTION__);
    }
    memcpy(connectionCfm.lcsrk.key, app_csrk, APP_GAP_KEY_LEN);
    connectionCfm.auth = auth;

    sonata_ble_gap_send_connection_cfm(conidx, &connectionCfm);
}

void app_gap_connect_confirm(uint8_t * addr, uint8_t auth)
{
    uint8_t conidx = app_get_conidx_by_addr(addr);
    if (conidx == APP_BLE_FUN_ERROR)
    {
        APP_TRC("APP: %s  error, conidx=%d\r\n", __FUNCTION__, conidx);
        return;
    }
    sonata_ble_gap_start_security(conidx, auth);
    APP_TRC("APP: %s ,id=%d, auth=%d, \r\n", __FUNCTION__, conidx, auth);
}

void ble_connection_callback(uint8_t conidx, sonata_gap_connection_req_ind_t * req)
{
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg                 = { 0 };
        msg.event_type                          = APP_BLE_STACK_EVENT_CONNECTION_REPORT;
        msg.param.connection_msg.conn_hdl       = conidx;
        msg.param.connection_msg.role           = req->role;
        msg.param.connection_msg.clk_accuracy   = req->clk_accuracy;
        msg.param.connection_msg.con_interval   = req->con_interval;
        msg.param.connection_msg.con_latency    = req->con_latency;
        msg.param.connection_msg.sup_to         = req->sup_to;
        msg.param.connection_msg.peer_addr_type = req->peer_addr_type;
        memcpy(msg.param.connection_msg.addr, req->peer_addr.addr, APP_GAP_ADDR_LEN);
        ble_cb_fun(msg);
    }
    if (app_evt_cb != NULL)
    {
        app_connect_status_ind_t status_ind;
        status_ind.connId = conidx;
        memmove(status_ind.addr, req->peer_addr.addr, APP_BD_ADDR_LEN);
        app_evt_cb(BLE_DEV_CONNECTED, (void *) &status_ind);
    }
}

static uint16_t app_gap_connection_req_callback(uint8_t conidx, sonata_gap_connection_req_ind_t * req)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    APP_TRC("      : conhdl      =%d  \r\n", req->conhdl);
    APP_TRC("      : con_interval=%d  \r\n", req->con_interval);
    APP_TRC("      : con_latency =%d  \r\n", req->con_latency);
    APP_TRC("      : sup_to      =%d  \r\n", req->sup_to);
    APP_TRC("      : clk_accuracy=%d  \r\n", req->clk_accuracy);
    APP_TRC("      : addr_type   =%d  \r\n", req->peer_addr_type);
    APP_TRC("      : role        =%d  \r\n", req->role);
    APP_TRC("      : addr:");
    for (int i = 0; i < APP_BD_ADDR_LEN; ++i)
    {
        APP_TRC("%02X ", req->peer_addr.addr[i]);
    }
    APP_TRC(" \r\n");
    if (!memcmp(req->peer_addr.addr, gTargetAddr, APP_BD_ADDR_LEN))
    {
        ble_connect_id = conidx;
    }
    g_curr_conn_index = conidx;
    app_connect_req_list_add(req->peer_addr.addr, conidx);
    APP_TRC("ble_connect_id %d \r\n", ble_connect_id);
    // sonata_ble_gatt_exchange_mtu(conidx);
    app_connected_state[conidx] = APP_STATE_CONNECTED;

    memcpy(&g_connection_req, req, sizeof(sonata_gap_connection_req_ind_t));

    if (req->role == SONATA_ROLE_MASTER)
    {
        ble_connection_callback(g_curr_conn_index, &g_connection_req);
    }

    if (app_bond)
    {
        sonata_gap_connection_cfm_t connectionCfm = { 0 };

        memcpy(connectionCfm.lcsrk.key, app_csrk, APP_GAP_KEY_LEN);
        connectionCfm.auth = SONATA_GAP_AUTH_REQ_SEC_CON_BOND;

        sonata_ble_gap_send_connection_cfm(conidx, &connectionCfm);
        return CB_DONE; // SDK will send connection confirm message
    }
    else
    {
        return CB_REJECT;
    }
}

/*!
 * @brief
 * @param connection_id
 * @param handle
 * @return
 */
static uint16_t app_gatt_read_request_callback(uint8_t connection_id, uint16_t handle)
{
    APP_TRC("APP: %s, handle=0x%04X,\r\n", __FUNCTION__, handle);

    uint16_t length = 250;
    uint8_t * value = sonata_api_malloc(length);
    app_ble_gatt_read_request_handler(handle, &length, value);
    sonata_ble_gatt_send_read_confirm(connection_id, handle, SONATA_GAP_ERR_NO_ERROR&0xFF, length, value);
    sonata_api_free(value);
    return CB_REJECT;
}

/*!
 * @brief
 * @param connection_id
 * @param handle
 * @param offset
 * @param length
 * @param value
 * @return
 */
static uint16_t app_gatt_write_request_callback(uint8_t connection_id, uint16_t handle, uint16_t offset, uint16_t length,
                                                uint8_t * value)
{
    // APP_TRC("APP_CB: %s, handle=0x%04X,custom_svc_start_handle=0x%04X", __FUNCTION__,handle,custom_svc_start_handle);
    APP_TRC("APP: %s, handle=0x%04X offset %d\r\n", __FUNCTION__, handle, offset);

    sonata_ble_gatt_send_write_confirm(connection_id, handle, SONATA_GAP_ERR_NO_ERROR&0xFF);
    app_ble_gatt_write_request_handler(handle, length, value);

    return CB_DONE;
}

/*!
 * @brief
 * @param connection_id
 * @param mtu
 * @return
 */
static uint16_t app_gatt_mtu_changed_callback(uint8_t connection_id, uint16_t mtu)
{
    APP_TRC("APP: %s, mtu=0x%04X\r\n", __FUNCTION__, mtu);
    ble_con_mtu[connection_id] = mtu;

    if (matter_event_cb && (matter_event_cb(APP_BLE_STACK_EVENT_MTU_CHANGED, 0, connection_id, mtu) == MATTER_EVENT_FINISHED))
    {
        return CB_DONE;
    }

    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg            = { 0 };
        msg.event_type                     = APP_BLE_STACK_EVENT_MTU_CHANGED;
        msg.param.mtu_changed_msg.conn_hdl = connection_id;
        msg.param.mtu_changed_msg.mtu      = mtu;
        ble_cb_fun(msg);
    }
    if (app_evt_cb != NULL)
    {
        app_mtu_change_ind_t status_ind;
        status_ind.connId = connection_id;
        status_ind.mtu    = mtu;
        app_evt_cb(BLE_MTU_CHANGE, (void *) &status_ind);
    }

    return CB_DONE;
}

/*!
 * @brief
 * @param connection_id
 * @param handle
 * @param offset
 * @param length
 * @param value
 * @return
 */
static uint16_t app_gatt_att_info_req_ind_callback(uint8_t connection_id, uint16_t handle)
{
    // APP_TRC("APP_CB: %s, handle=0x%04X,custom_svc_start_handle=0x%04X", __FUNCTION__,handle,custom_svc_start_handle);
    APP_TRC("APP: %s, handle=0x%04X\r\n", __FUNCTION__, handle);
    uint16_t length = 250;

    sonata_ble_gatt_send_att_info_confirm(connection_id, handle, length, SONATA_GATT_ERR_NO_ERROR&0xFF);
    return CB_DONE;
}

static uint16_t app_gatt_event_callback(uint8_t conidx, uint16_t handle, uint16_t type, uint16_t length, uint8_t * value)
{
    APP_TRC("APP: %s,handle = %04X, type = %04X,length = %04X\r\n", __FUNCTION__, handle, type, length);

    APP_TRC("APP: %s, Master get Ntf data form Slave. Data:", __FUNCTION__);
    for (int i = 0; i < length; ++i)
    {
        APP_TRC("%02X[%c] ", value[i], value[i]);
    }
    APP_TRC("\r\n");
    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg     = { 0 };
        msg.event_type              = APP_BLE_STACK_EVENT_NOTIFY_REQ;
        msg.param.gatt_msg.conn_hdl = conidx;
        msg.param.gatt_msg.data     = value;
        msg.param.gatt_msg.data_len = length;
        msg.param.gatt_msg.att_hdl  = handle;
        ble_cb_fun(msg);
    }
    return CB_DONE;
}

static uint16_t app_gatt_event_req_callback(uint8_t conidx, uint16_t handle, uint16_t type, uint16_t length, uint8_t * value)
{
    APP_TRC("APP: %s,handle = %04X, type = %04X,length = %04X\r\n", __FUNCTION__, handle, type, length);

    // APP_TRC("APP_CB: %s, Master get Ind data form Slave. Data:", __FUNCTION__);
    // for (int i = 0; i < length; ++i)
    // {
    //     APP_TRC("%02X[%c] ", value[i],value[i]);
    // }
    // APP_TRC("\r\n");
    sonata_ble_gatt_send_event_confirm(conidx, handle);

    if (ble_cb_fun != NULL)
    {
        app_ble_stack_msg_t msg     = { 0 };
        msg.event_type              = APP_BLE_STACK_EVENT_INDICATE_REQ;
        msg.param.gatt_msg.conn_hdl = conidx;
        msg.param.gatt_msg.data     = value;
        msg.param.gatt_msg.data_len = length;
        msg.param.gatt_msg.att_hdl  = handle;
        ble_cb_fun(msg);
    }
    return CB_DONE;
}

static uint16_t app_gatt_connection_info_callback(uint8_t conidx, uint16_t gatt_start_handle, uint16_t gatt_end_handle,
                                                  uint16_t svc_chg_handle, uint8_t cli_info, uint8_t cli_feat)
{
    APP_TRC("APP: %s, conidx=%d,gatt_start_handle=%d,gatt_end_handle=%d,svc_chg_handle=%d,cli_info=%d,cli_feat=%d\r\n",
            __FUNCTION__, conidx, gatt_start_handle, gatt_end_handle, svc_chg_handle, cli_info, cli_feat);

    return CB_DONE;
}

void app_ble_set_target_addr(uint8_t * target)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    if (target == NULL)
    {
        memset(gTargetAddr, 0, APP_BD_ADDR_LEN);
    }
    else
    {
        memcpy(gTargetAddr, target, APP_BD_ADDR_LEN);
    }
    for (int i = 0; i < APP_BD_ADDR_LEN; i++)
    {
        APP_TRC("%x ", gTargetAddr[i]);
    }
    APP_TRC("\r\n ");
}

bool app_ble_check_target_addr()
{
    if (gTargetAddr[0] == 0 && gTargetAddr[1] == 0 && gTargetAddr[2] == 0 && gTargetAddr[3] == 0 && gTargetAddr[4] == 0 &&
        gTargetAddr[5] == 0)
    {
        return false;
    }
    return true;
}
uint16_t app_ble_get_mtu(uint8_t conidx)
{
    return ble_con_mtu[conidx];
}
int app_ble_scan_start(app_ble_scan_callback_t cb)
{
    app_gap_set_scan_cb(cb);
    sonata_api_app_timer_set(8, 20);
    sonata_api_app_timer_active(8);
    return BLE_STATUS_SUCCESS;
}
int app_ble_scan_stop(void)
{
    app_ble_stop_scanning();
    return BLE_STATUS_SUCCESS;
}
int app_ble_set_max_mtu(uint16_t mtu)
{
    if (mtu > APP_BLE_MAX_MTU_SIZE)
    {
        return BLE_STATUS_FAILED;
    }
    return BLE_STATUS_SUCCESS;
}
/*!
 * @brief: app profile api init
 * @param none
 * @return none
 */
void app_prf_api_init(void)
{
#if CONFIG_ENABLE_ASR_APP_MESH
    sonata_mesh_api_init();
#endif
#if BLE_DIS_SERVER
    sonata_prf_diss_init();
#endif
#if BLE_BATT_SERVER
    sonata_prf_bass_init();
#endif
}

#if !defined ALIOS_SUPPORT && !defined HARMONYOS_SUPPORT
static uint8_t app_at_cmd_handler(void * p_param)
{
    // at_command_process_ble();

    return API_SUCCESS;
}
#endif

/*
 * LOCAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */

sonata_ble_hook_t app_hook = {
    assert_err,
    assert_param,
    assert_warn,
    app_init,
    platform_reset,
    get_stack_usage,
#if defined ALIOS_SUPPORT || defined HARMONYOS_SUPPORT
    printf,
#else
    __wrap_printf,
#endif
    app_prf_api_init,
#ifdef SONATA_RTOS_SUPPORT
    (void *) lega_rtos_init_semaphore,
    (void *) lega_rtos_get_semaphore,
    (void *) lega_rtos_set_semaphore,
#endif

};

uint16_t gap_active_stopped_callback(uint8_t actv_idx, uint8_t type, uint8_t reason, uint8_t per_adv_stop)
{
    APP_TRC("APP: %s,actv_idx=%d reason=0x%04X\r\n", __FUNCTION__, actv_idx, reason);
    if (SONATA_GAP_ACTV_TYPE_ADV == type)
    {
        if (ble_adv_idx_table[APP_CONN_ADV_IDX] == actv_idx)
        {
            if ((ble_adv_state[APP_CONN_ADV_IDX] == BLE_ADV_STATE_STARTED) && (g_curr_conn_index != SONATA_ADDR_NONE))
            {
                ble_connection_callback(g_curr_conn_index, &g_connection_req);
                g_curr_conn_index = SONATA_ADDR_NONE;
#ifdef MEDIA_MATTER_TEST
                app_ble_advertising_stop(APP_MATTER_ADV_IDX);
                adv_media_beacon_start();
#endif
            }
            ble_adv_state[APP_CONN_ADV_IDX] = BLE_ADV_STATE_CREATED;
            app_ble_adv_event_handler(APP_CONN_ADV_IDX, false);
        }
        else if (ble_adv_idx_table[APP_MATTER_ADV_IDX] == actv_idx)
        {
            if ((ble_adv_state[APP_MATTER_ADV_IDX] == BLE_ADV_STATE_STARTED) && (g_curr_conn_index != SONATA_ADDR_NONE))
            {
                if (matter_event_cb)
                {
                    matter_event_cb(APP_BLE_STACK_EVENT_CONNECTION_REPORT, 0, g_curr_conn_index, 0);
                }
                g_curr_conn_index = SONATA_ADDR_NONE;
#ifdef MEDIA_MATTER_TEST
                app_ble_advertising_stop(APP_CONN_ADV_IDX);
                adv_media_beacon_start();
#endif
            }
            ble_adv_state[APP_MATTER_ADV_IDX] = BLE_ADV_STATE_CREATED;
            if (matter_event_cb)
            {
                matter_event_cb(SONATA_GAP_CMP_ADVERTISING_STOP, 0, actv_idx, 0);
            }
        }
        else if (ble_adv_idx_table[APP_NON_CONN_ADV_IDX] == actv_idx)
        {
            ble_adv_state[APP_NON_CONN_ADV_IDX] = BLE_ADV_STATE_CREATED;
            app_ble_adv_event_handler(APP_NON_CONN_ADV_IDX, false);
        }
        if (app_evt_cb != NULL)
        {
            app_adv_status_ind_t status_ind;
            status_ind.advId  = actv_idx;
            status_ind.status = reason;
            app_evt_cb(BLE_ADV_STOP, (void *) &status_ind);
        }
    }
    if (SONATA_GAP_ACTV_TYPE_SCAN == type)
    {
        return CB_DONE; // not delete scan instance
    }
    if (SONATA_GAP_ACTV_TYPE_INIT == type)
    {
        ble_init_idx = APP_BLE_INIT_INVALID_IDX;
        return CB_REJECT; // delete init instance
    }
    return CB_DONE;
}

static ble_gap_callback ble_gap_callbacks = {
    /*************** GAP Manager's callback ***************/

    // Must if use scan function, peer's information will show in this callback
    .gap_scan_result = app_gap_scan_result_callback,
    // Optional, use for get local devcie informations when call sonata_ble_get_dev_info()
    .get_local_dev_info = app_get_dev_info_callback,

    /*************** GAP Controller's callback  ***************/
    // Optional
    .gap_param_update_req = app_gap_param_update_req_callback,
    // Optional
    .gap_param_updated = app_gap_param_updated_callback,
    // Optional, used for get peer att information when call  sonata_ble_gap_get_peer_info()
    .gap_get_peer_info = app_gap_peer_info_callback,
    // Optional, used for get peer att information when call  sonata_ble_gap_get_peer_info()
    .gap_get_peer_att_info = app_gap_peer_att_info_callback,
    // Optional, if peer device get local device's information, app can deal with it in this callback
    .gap_peer_get_local_info = app_gap_peer_get_local_info_callback,
    // Optional
    .gap_disconnect_ind = app_gap_disconnect_ind_callback,
    // Optional, if peer device set local device's name, app can deal with it in this callback
    .gap_peer_set_local_device_name = app_gap_peer_set_local_device_name_callback,
    // Optional, app can save peer mac address in this callback when connected
    .gap_connection_req    = app_gap_connection_req_callback,
    .gap_active_stopped    = gap_active_stopped_callback,
    .gap_bond_req          = app_gap_bond_req_callback,
    .gap_bond              = app_gap_bond_callback,
    .gap_encrypt_req       = app_gap_encrypt_req_callback,
    .gap_gen_random_number = app_gap_gen_random_number_callback,
    .gap_security          = app_gap_security_callback,
    .gap_encrypt           = app_gap_encrypt_callback,
    .gap_le_pkt_size       = app_gap_le_pke_size_callback,

};

static ble_gatt_callback ble_gatt_callbacks = {
    // Must if use discovery all servcie function
    .gatt_disc_svc = app_gatt_disc_svc_callback,
    // Must if use discovery all characteristic function
    .gatt_disc_char = app_gatt_disc_char_callback,
    // Must if use discovery all description function
    .gatt_disc_char_desc = app_gatt_disc_desc_callback,
    // Optional, add this callback if app need to save changed mtu value
    .gatt_mtu_changed = app_gatt_mtu_changed_callback,
    // Must,If app add custom service, app should add this callback to deal with peer device read request
    .gatt_read_req = app_gatt_read_request_callback,
    .gatt_read     = app_gatt_read_callback,
    // Must,If app add custom service, app should add this callback to deal with peer device write request
    .gatt_write_req       = app_gatt_write_request_callback,
    .gatt_att_info_req    = app_gatt_att_info_req_ind_callback,
    .gatt_event           = app_gatt_event_callback,
    .gatt_event_req       = app_gatt_event_req_callback,
    .gatt_connection_info = app_gatt_connection_info_callback,
};

static ble_complete_callback ble_complete_callbacks = {
    // Must, app can do next operation in this callback
    .ble_complete_event = app_ble_complete_event_handler,
};

static ble_response_callback ble_rsp_callbacks = {
    // Must,IF app add custom service, add should save this service's start handler id,
    // this id will be used in app_gatt_read_request_callback() and app_gatt_write_request_callback()
    .ble_rsp_event = app_ble_rsp_event_handler,
};

#if !defined ALIOS_SUPPORT && !defined HARMONYOS_SUPPORT
sonata_api_app_msg_t app_at_cmd_msg = {
    .operation = APP_MSG_AT_CMD,
    .function  = app_at_cmd_handler,
};
#endif

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
#if 0
void msm_ble_app_start_adv()
{
    char *name = "msm_ble_demo";
    uint8_t dev_name_len;
    uint8_t *pos;
    uint8_t buf[256];
    uint8_t advDataLen;
    uint8_t respDataLen = 0;
    pos = buf;
    dev_name_len = strlen(name);
    *pos++ = dev_name_len + 1;  //pos len;  (payload + type)
    *pos++  = '\x09';   //pos: type
    memcpy(pos, name, dev_name_len);
    pos += dev_name_len;
    advDataLen = pos - buf;
    ms_hal_ble_adv_start(buf,advDataLen,NULL,0);

}
uint16_t perm_array[10] = {0,PRD_NA,PWR_NA, PRD_NA,PRD_NA,PIND_NA,PRD_NA | PWR_NA,PRD_NA | PWR_NA};
void msm_ble_app_add_service(void)
{

    ms_test_ble_service_init();
    return;
    for(int i =0; i < 10 ;i++)
    {
        printf("array 0x%x\r\n",perm_array[i]);
    }
    /*
    ms_hal_ble_service_attrib_t  * att[BLE_SERVICE_IDX_NB] =
    {
        [BLE_SERVICE_IDX]                   =  ms_ble_service_atts,
        [BLE_SERVICE_RECIEVE_CHAR]          =  &ms_ble_service_atts[1],
        [BLE_SERVICE_RECIEVE_VAL]           =  &ms_ble_service_atts[2],
        [BLE_SERVICE_RECIEVE_DESCIPTION]    =  &ms_ble_service_atts[3],

        [BLE_SERVICE_SEND_CHAR]             =  &ms_ble_service_atts[4],
        [BLE_SERVICE_SEND_VAL]              =  &ms_ble_service_atts[5],
        [BLE_SERVICE_SEND_CONFIG]           =  &ms_ble_service_atts[6],
        [BLE_SERVICE_SEND_DESCIPTION]       =  &ms_ble_service_atts[7],
        };
        ms_hal_ble_gatt_service_add(&start_handle,att,BLE_SERVICE_IDX_NB);
        */
}

int ms_hal_ble_stack_callback_handler(ms_hal_ble_stack_event_t event)
{
    switch(event)
    {
        case MS_HAL_BLE_STACK_EVENT_STACK_READY:
            msm_ble_app_add_service();
            msm_ble_app_start_adv();
            printf("ready!!!!");
         break;
         default :  printf("ms_hal_ble_stack_callback_handler %d \r\n",event);
    }
}
#endif
void app_init(void)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    sonata_log_level_set(SONATA_LOG_VERBOSE);

    sonata_ble_register_gap_callback(&ble_gap_callbacks);
    sonata_ble_register_gatt_callback(&ble_gatt_callbacks);
    sonata_ble_register_complete_callback(&ble_complete_callbacks);
    sonata_ble_register_response_callback(&ble_rsp_callbacks);
    // ms_hal_ble_set_stack_event_register(ms_hal_ble_stack_callback_handler);
    app_data_init();
    app_ble_on();

    sonata_api_register_app_timer_callback(&app_timer_callbacks);

#if !defined ALIOS_SUPPORT && !defined HARMONYOS_SUPPORT
    // at command init
    //    at_init();

    // register app message
    sonata_api_app_msg_register(&app_at_cmd_msg);
#endif
}

uint16_t test_count      = 0;
uint16_t test_interval   = 0;
uint8_t write_value[128] = { 0x1, 0x2, 0x3, 0x4, 0x5 };
bool ble_test_mode       = false;

bool app_is_ble_test_mode(void)
{
    return ble_test_mode;
}

void app_set_ble_test_mode(bool mode)
{
    ble_test_mode = mode;
}

void app_ble_set_test_count(uint16_t counter)
{
    test_count = counter;
}

void app_ble_set_uuid(app_uuids * uuid)
{
    gAppEnv.appUuids.read    = uuid->read;
    gAppEnv.appUuids.write   = uuid->write;
    gAppEnv.appUuids.ntf     = uuid->ntf;
    gAppEnv.appUuids.service = uuid->service;
    APP_TRC("APP: %s  svc=%04X, read=%04X, write=%04X, ntf=%04X\r\n", __FUNCTION__, gAppEnv.appUuids.service, gAppEnv.appUuids.read,
            gAppEnv.appUuids.write, gAppEnv.appUuids.ntf);
}

app_uuids * app_ble_get_uuid()
{
    return &gAppEnv.appUuids;
}
void app_ble_set_test_target(uint8_t * target)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    memcpy(gTargetAddr, target, APP_BD_ADDR_LEN);
    for (int i = 0; i < APP_BD_ADDR_LEN; i++)
    {
        APP_TRC("%x ", gTargetAddr[i]);
    }
    APP_TRC("\r\n ");
}

void app_ble_set_test_write_uuid(uint8_t * uuid)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    memcpy(write_uuid, uuid, 2);
    for (int i = 0; i < 2; i++)
    {
        APP_TRC("%x ", write_uuid[i]);
    }
    APP_TRC("\r\n ");
}

void app_ble_set_test_read_uuid(uint8_t * uuid)
{
    APP_TRC("APP: %s  \r\n", __FUNCTION__);

    memcpy(read_uuid, uuid, 2);
    for (int i = 0; i < 2; i++)
    {
        APP_TRC("%x ", read_uuid[i]);
    }
    APP_TRC("\r\n ");
}

void app_ble_set_test_interval(uint16_t interval)
{
    test_interval = interval;
}

static uint8_t app_timer_handler(uint16_t id)
{
    APP_TRC("APP: %s  . id=%d\r\n", __FUNCTION__, id);

    if (id == 6)
    {
        static uint8_t adv = 0;
        if (test_count > 0)
        {
            if (!adv)
            {
                adv_value[9] = test_count;
#ifdef HARMONYOS_TEMP
#else
                // ms_hal_ble_adv_start(adv_value,adv_length,NULL,0);
#endif
            }
            else
            {
#ifdef HARMONYOS_TEMP
#else
                // ms_hal_ble_adv_stop();
#endif
            }
            adv = 1 - adv;
            test_count--;
            sonata_api_app_timer_set(6, 20);
            sonata_api_app_timer_active(6);
        }
        else
        {

            APP_TRC("APP: %s  . id=%d test done \r\n", __FUNCTION__, id);
            sonata_api_app_timer_clear(1);
        }
    }
    if (id == 7)
    {
        static uint8_t flag = 0;
        if (test_count > 0)
        {
            if (flag == 0)
            {
                flag = 1;
            }
            else
            {
                test_count--;
            }

            adv_value[9] = test_count;
            sonata_ble_set_advertising_data(adv_length - 3, adv_value + 3);
            sonata_api_app_timer_set(7, 20);
            sonata_api_app_timer_active(7);
        }
        else
        {

            flag = 0;
            APP_TRC("APP: %s  . id=%d test done \r\n", __FUNCTION__, id);
            sonata_api_app_timer_clear(2);
        }
    }

    if (id == 8)
    {
        static uint8_t scan_flag = 0;

        if (scan_flag == 0)
        {
            scan_flag = 1;
            app_ble_config_scanning();
            sonata_api_app_timer_set(8, 100);
            sonata_api_app_timer_active(8);
        }
        else
        {
            app_ble_start_scanning();
            scan_flag = 0;
            sonata_api_app_timer_clear(8);
        }
    }

    if (id == 3)
    {
        app_ble_config_initiating();
    }

    if (id == 4)
    {
        sonata_ble_gatt_write(ble_connect_id, write_handle + 1, 0, 0, 128, write_value);

        // sonata_ble_gatt_write_no_response(ble_connect_id, write_handle + 1,0,0,128,write_value);
        sonata_api_app_timer_set(4, test_interval);
        sonata_api_app_timer_active(4);
    }
    if (id == 5)
    {
        sonata_api_app_timer_set(5, 500);
        sonata_api_app_timer_active(5);
    }
    if (id == 9)
    {
        scan_set_state = APP_BLE_SCAN_ON;
        scan_state     = BLE_SCAN_STATE_CREATING;
        app_ble_config_scanning();
    }
    return 0;
}

extern int init_ble_task(void);
int ble_close(void);

uint8_t ble_open;

void app_ble_set_ble_open(uint16_t value)
{
    APP_TRC("APP: %s, value=%d\r\n", __FUNCTION__, value);
    ble_open = value;
}

// 连接相关
int app_ble_stack_start(ble_stack_opr_module_id_t module)
{

    if (0 != ble_open)
    {
        ble_open |= (1UL << module);
        return 0;
    }
    ble_open |= (1UL << module);
    printf("%s init_ble_task() \r\n", __FUNCTION__);
    init_ble_task();
    return 0;
}

int app_ble_stack_stop(ble_stack_opr_module_id_t module)
{

    ble_open &= ~(1UL << module);
    if (0 != ble_open)
    {
        return 0;
    }
    APP_TRC("APP: %s  \r\n", __FUNCTION__);
    ble_close();
    return 0;
}

int app_ble_disconnect_by_addr(uint8_t * addr)
{
    uint8_t conidx = app_get_conidx_by_addr(addr);
    if (conidx != ble_connect_id)
    {

        APP_TRC("APP: %s  not found device\r\n", __FUNCTION__);
        app_print_hex(addr, APP_BD_ADDR_LEN);
    }
    app_ble_disconnect();
    return 0;
}

void app_register_core_evt_ind(app_core_evt_ind_cb cb)
{
    app_evt_cb = cb;
}

void app_register_sec_cb(app_sec_req_cb cb)
{
    sec_req_call = cb;
}
#if 0
extern CRITICAL_FUNC_SEG void sonata_ble_isr(void);
CRITICAL_FUNC_SEG void BLE_IRQHandler(void)
{
    sonata_ble_isr();
}
#endif

/// @} APP
