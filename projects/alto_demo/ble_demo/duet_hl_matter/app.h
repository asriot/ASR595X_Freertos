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
 * @file app.h
 *
 * @brief Application entry point
 *
 ****************************************************************************************
 */

#ifndef APP_H_
#define APP_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "sonata_ble_hook.h"
#include "sonata_gap_api.h"
#include "sonata_gatt_api.h"

/*
 * DEFINES
 ****************************************************************************************
 */



/*
 * MACROS
 ****************************************************************************************
 */
/// debug trace
#define APP_DBG    0
#if APP_DBG
#define APP_TRC    printf
#else
#define APP_TRC(...)
#endif //APP_DBG

#define APP_TRC_ERR    printf



#define APP_MAX_WILTELIST_NUM   (10)

#define APP_CONN_ADV_IDX        (0)
#define APP_NON_CONN_ADV_IDX    (1)
#define APP_MATTER_ADV_IDX      (2)
#define APP_MAX_ADV_IDX         (3)
#define APP_DEFAULT_MTU_SIZE    (23)
#define APP_GAP_ADDR_LEN        (6)
/*
 * ENUMERATIONS
 ****************************************************************************************
 */
#define MAX_BONDED_DEV_NUM      (3)
#define MAX_BONDED_DEV_INDEX    (MAX_BONDED_DEV_NUM - 1)
#define INVALID_BONDED_INDEX    (MAX_BONDED_DEV_NUM)
#define APP_GAP_KEY_LEN         (0x10)
#define APP_GAP_RAND_NB_LEN     (0x08)
#define APP_BD_ADDR_LEN         (6)
#define APP_UUID_LEN            (16)

#define MAX_ADV_DATA_LEN        (32)

#define  MATTER_EVENT_FINISHED  (1)

enum app_connect_state
{
    ///Connection succeeded
    APP_STATE_CONNECTED = 0,
    /// Link is disconnected
    APP_STATE_DISCONNECTED,
};

typedef enum
{
    APP_DISCONNECTED,
    APP_CONNECTED,
    APP_BONDING,
    APP_BONDED,
}bound_conn_state;

typedef enum APP_BLE_STACK_EVENT_T
{
    APP_BLE_STACK_EVENT_STACK_READY = 0,
    APP_BLE_STACK_EVENT_STACK_FAIL,
    APP_BLE_STACK_EVENT_ADV_ON,
    APP_BLE_STACK_EVENT_ADV_OFF,
    ///@deprecated Use APP_BLE_STACK_EVENT_CONNECTION_REPORT
    APP_BLE_STACK_EVENT_PERIPHERAL_CONNECTED, //!< Bluetooth connected as peripheral
    ///@deprecated Use APP_BLE_STACK_EVENT_CONNECTION_REPORT
    APP_BLE_STACK_EVENT_CENTRAL_CONNECTED,               //!< Bluetooth connected as central
    APP_BLE_STACK_EVENT_DISCONNECTED,                    //!< Bluetooth disconnect
    APP_BLE_STACK_EVENT_SCAN_ON,                         //!< ble scan start
    APP_BLE_STACK_EVENT_SCAN_OFF,                        //!< ble scan stops
    APP_BLE_STACK_EVENT_NONCONN_ADV_ON,                  //!< ble nonconn advertising start
    APP_BLE_STACK_EVENT_NONCONN_ADV_OFF,                 //!< ble nonconn advertising stops
    //Part 2
    APP_BLE_STACK_EVENT_NOTIFY_REQ,                      //!< ble notify data callback  //GATT_EVENT,Central get Ntf data form peripheral
    APP_BLE_STACK_EVENT_INDICATE_REQ,                    //!< ble indicate data callback //GATT_REQ_EVENT,Central get Ind data form peripheral
    APP_BLE_STACK_EVENT_DISC_SVC_REPORT,                 //!< discovery all service report
    APP_BLE_STACK_EVENT_DISC_CHAR_REPORT,                //!< discovery all char report
    APP_BLE_STACK_EVENT_DISC_DESC_REPORT,                //!< discovery all descriptor
    APP_BLE_STACK_EVENT_MTU_CHANGED,                     //!< MTU changed callback
    APP_BLE_STACK_EVENT_CONNECTION_REPORT,               //!< Connection information
    APP_BLE_STACK_EVENT_CONNECT_PARAM_UPDATE_REQ,        //!< Connection parameter update request

    APP_BLE_STACK_EVENT_CMP                      = 0X80, //!< Complete event start ID, Do not use.
    APP_BLE_STACK_EVENT_CMP_MTU,                         //!< MTU change complete
    APP_BLE_STACK_EVENT_CMP_SVC_DISC,                    //!< discovery service done
    APP_BLE_STACK_EVENT_CMP_CHAR_DISC,                   //!< discovery char done
    APP_BLE_STACK_EVENT_CMP_DISC_DESC_CHAR,              //!< discover characteristic descriptor done
    APP_BLE_STACK_EVENT_CMP_WRITE_REQ,                   //!< write request complete
    APP_BLE_STACK_EVENT_CMP_WRITE_CMD,                   //!< write cmd complete
    APP_BLE_STACK_EVENT_CMP_NOTIFY,                      //!< Notify complete //Event when peripheral send data via NTF
    APP_BLE_STACK_EVENT_CMP_INDICATE,                    //!< Indicate complete//Event when peripheral send data via IND
} app_ble_stack_event_t;

enum BLE_ADV_STATE
{
    /// Advertising activity does not exists
    BLE_ADV_STATE_IDLE = 0,
    /// Creating advertising activity
    BLE_ADV_STATE_CREATING,
    /// Setting advertising data
    BLE_ADV_STATE_SETTING_ADV_DATA,
    /// Setting scan response data
    BLE_ADV_STATE_SETTING_SCAN_RSP_DATA,
    /// Advertising activity created
    BLE_ADV_STATE_CREATED,
    /// Starting advertising activity
    BLE_ADV_STATE_STARTING,
    /// Advertising activity started
    BLE_ADV_STATE_STARTED,
    /// Stopping advertising activity
    BLE_ADV_STATE_STOPPING,
};

enum BLE_SCAN_STATE
{
    BLE_SCAN_STATE_IDLE = 0,
    BLE_SCAN_STATE_WHILTELIST_SETTING,
    BLE_SCAN_STATE_CREATING,
    BLE_SCAN_STATE_CREATED,
    BLE_SCAN_STATE_WHILTELIST_AGAIN,
    BLE_SCAN_STATE_STARTING,
    BLE_SCAN_STATE_STARTED,
    BLE_SCAN_STATE_STOPPING,
};


enum BLE_SCAN_EVENT
{
    BLE_SCAN_EVENT_WT_CMP = 0,
    BLE_SCAN_EVENT_CONFIG_CMP,
    BLE_SCAN_EVENT_START_CMP,
    BLE_SCAN_EVENT_STOP_CMP,
    BLE_SCAN_EVENT_DELETE_CMP,
};


typedef enum
{
    BLE_ENABLE_ADV,
    BLE_DISABLE_ADV,
} ble_adv_enable;

typedef enum
{
    BLE_GATTC_INDICATE,
    BLE_GATTC_NOTIFY,
} ble_att_op_t;

typedef enum
{
    BLE_STATUS_SUCCESS,
    BLE_STATUS_INVALID_PARAM,
    BLE_STATUS_DISCONNETED,
    BLE_STATUS_FAILED,
} ble_status_t;


typedef struct{
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint16_t data_len;
    uint8_t *data;
}app_ble_gatt_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t start_hdl;
    uint16_t end_hdl;
    uint8_t uuid_len;
    uint8_t *uuid;
}app_ble_disc_svc_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint16_t pointer_hdl;
    uint8_t prop;
    uint8_t uuid_len;
    uint8_t *uuid;
}app_ble_disc_char_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint8_t uuid_len;
    uint8_t *uuid;
}app_ble_disc_desc_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t mtu;
}app_ble_mtu_changed_msg_t;

typedef struct{
    uint16_t conn_hdl;
    /// Peer BT address
    uint8_t addr[APP_GAP_ADDR_LEN];
    /// Connection interval
    uint16_t con_interval;
    /// Connection latency
    uint16_t con_latency;
    /// Link supervision timeout
    uint16_t sup_to;
    /// Clock accuracy
    uint8_t clk_accuracy;
    /// Peer address type
    uint8_t peer_addr_type;
    /// Role of device in connection (0 = Master / 1 = Slave)
    uint8_t role;
}app_ble_connetion_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint8_t reason;
}app_ble_disconnect_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t conn_intv_min;
    uint16_t conn_intv_max;
    uint16_t slave_latency;
    uint16_t timeout;
}app_ble_conn_param_msg_t;

typedef struct{
    app_ble_stack_event_t event_type;
    union{
        uint16_t conn_hdl;
        app_ble_gatt_msg_t gatt_msg;
        app_ble_disc_svc_msg_t disc_svc_msg;
        app_ble_disc_char_msg_t disc_char_msg;
        app_ble_disc_desc_msg_t disc_desc_msg;
        app_ble_mtu_changed_msg_t mtu_changed_msg;
        app_ble_connetion_msg_t connection_msg;
        app_ble_disconnect_msg_t disconnect_msg;
        app_ble_conn_param_msg_t conn_param_msg;
    }param;
}app_ble_stack_msg_t;

typedef uint16_t (*cb_fun)(app_ble_stack_msg_t);
typedef  void (*app_ble_scan_callback_t)(void *adv_ind,void *rsp_ind);
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Scanning parameters
typedef struct app_ble_scan_param {
    uint8_t own_addr_type;
    uint8_t addr_count;
    sonata_gap_scan_param_t scan_param;
    struct sonata_gap_bdaddr trans_addr_list[APP_MAX_WILTELIST_NUM];
}app_ble_scan_param_t;

typedef struct app_uuid_t{
    uint16_t service;
    uint16_t read;
    uint16_t write;
    uint16_t ntf;
}app_uuids;
/// Long Term Key information
typedef struct app_sonata_gap_ltk
{
    /// Long Term Key
    uint8_t ltk[APP_GAP_KEY_LEN];
    /// Encryption Diversifier
    uint16_t ediv;
    /// Random Number
    uint8_t randnb[APP_GAP_RAND_NB_LEN];
}app_sonata_gap_ltk_t;

/// Short Term Key information
typedef struct app_sonata_gap_irk
{
    /// Short Term Key
    uint8_t irk[APP_GAP_KEY_LEN];
    /// irk addr
    uint8_t irk_addr[APP_BD_ADDR_LEN];
}app_sonata_gap_irk_t;

typedef struct bonded_dev_info
{
    uint8_t              peer_addr[APP_BD_ADDR_LEN];
    app_sonata_gap_ltk_t ltk;
    uint8_t              ltk_in[APP_GAP_KEY_LEN];
    app_sonata_gap_irk_t irk;
    uint8_t              periph_bond;
}bonded_dev_info_t;

typedef struct bonded_dev_info_list
{
    uint8_t           total_dev;
    uint8_t           current_dev_index;
    bonded_dev_info_t bonded_device_info[MAX_BONDED_DEV_NUM];
}bonded_dev_info_list_t;

typedef struct peer_conn_param
{
    /// Connection interval maximum
    uint16_t intv_max;
    /// Latency
    uint16_t latency;
    /// Supervision timeout
    uint16_t time_out;
}peer_conn_param_t;


typedef struct connect_req_info
{
    uint8_t           conidx;
    uint8_t           bd_addr[APP_BD_ADDR_LEN];
}connect_req_info_t;

typedef struct adv_idx_info
{
    uint8_t           local_idx;
    uint8_t           adv_id;
}adv_idx_info_t;

typedef struct BLE_ADV_PARAM_T
{
    uint16_t advertising_interval_min;
    uint16_t advertising_interval_max;
} ble_adv_param_t;


typedef struct BLE_ADV_DATA_T
{
    uint8_t ble_advdata[MAX_ADV_DATA_LEN];
    uint8_t ble_advdataLen;

} ble_adv_data_t;

typedef struct BLE_SCAN_DATA_T
{
    uint8_t ble_respdata[MAX_ADV_DATA_LEN];
    uint8_t ble_respdataLen;

} ble_scan_data_t;



typedef struct
{
    int status;
    int len;
    int handler;
    uint8_t uuid[APP_UUID_LEN];
}app_reg_service_cmp_t;

typedef struct
{
    int connId;
    uint8_t addr[APP_BD_ADDR_LEN];
}app_connect_status_ind_t;

typedef struct
{
    int connId;
    int status;
}app_ind_sent_ind_t;

typedef struct
{
    int connId;
    int mtu;
}app_mtu_change_ind_t;

typedef struct
{
    int advId;
    int status;
}app_adv_status_ind_t;

/**
* @brief enum core evt indicate type
*/
typedef enum
{
    BLE_SERVICE_ADD_CMP,
    BLE_DEV_CONNECTED,
    BLE_DEV_DISCONNECTED,
    BLE_IND_SENT,
    BLE_MTU_CHANGE,
    BLE_ADV_START,
    BLE_ADV_STOP,
}app_core_evt_ind_t;

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */
extern sonata_ble_hook_t app_hook;

///app Core Event indicate Callback
typedef int (*app_core_evt_ind_cb)(app_core_evt_ind_t evt ,void * p_param);

///
typedef int (*app_sec_req_cb)(uint8_t * addr);

typedef uint16_t (*matter_event_callback_t)(int opt_id, uint8_t status, uint16_t param, uint32_t dwparam);

typedef enum
{
    USER_INVALID_MODULE_ID ,
    USER_MIDEA_MODULE_ID   ,
    USER_OHOS_MODULE_ID   ,
    USER_MATTER_MODULE_ID  ,
    APP_USER_MAX_MODULE_ID    ///confilct with other MICROS
}ble_stack_opr_module_id_t;



typedef void (*ble_gatt_service_att_wirte_cb)(uint8_t *data, uint16_t size);
typedef void (*ble_gatt_service_att_read_cb)(uint8_t *data,  uint16_t* size);

typedef struct ble_gatt_att_opr
{
    ble_gatt_service_att_wirte_cb write_request;
    ble_gatt_service_att_read_cb  read_request;
}ble_gatt_att_opr_t;

typedef struct ble_gatt_att_reg
{
    sonata_gatt_att_desc_t att_desc;
    ble_gatt_att_opr_t  att_opr;
}ble_gatt_att_reg_t;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the BLE demo application.
 ****************************************************************************************
 */
void app_init(void);
void app_ble_config_legacy_advertising(uint8_t adv_idx);
uint8_t app_get_adv_status(uint8_t adv_idx);
uint8_t app_get_connect_status(uint16_t conn_hdl);
uint16_t app_ble_start_advertising(uint8_t adv_idx);
bool app_is_ble_test_mode(void);
void app_set_ble_test_mode(bool mode);
void app_ble_set_target_addr(uint8_t * target);
uint16_t app_ble_advertising_stop(uint8_t adv_idx);
int app_ble_advertising_start(uint8_t adv_idx,ble_adv_data_t *data,ble_scan_data_t *scan_data);
int app_ble_start_advertising_with_param(uint8_t adv_idx,uint8_t own_addr_type,sonata_gap_directed_adv_create_param_t * param, ble_adv_data_t * data,ble_scan_data_t * scan_data);
void app_gap_set_scan_cb(app_ble_scan_callback_t cb);
int app_ble_stack_stop(ble_stack_opr_module_id_t module);
int app_ble_stack_start(ble_stack_opr_module_id_t module);
void app_ble_set_device_name(uint8_t *name, uint32_t len);
int app_set_security_io_cap(uint8_t cap);
int app_ble_disconnect_by_addr(uint8_t *addr);
int  app_ble_disable_service_by_handler(uint16_t start_hdl);
void app_register_core_evt_ind(app_core_evt_ind_cb cb);
void app_gap_notify_pair_request_rsp(uint8_t *bd_addr,uint8_t accept);
void app_register_sec_cb(app_sec_req_cb  cb);
int app_set_security_auth_req(uint8_t auth_req);
void app_set_connect_flag(uint8_t vaule);
void app_gap_connect_confirm(uint8_t *addr, uint8_t auth);
int32_t app_ble_gatt_data_send(uint8_t conidx,uint16_t local_handle,uint16_t idx, uint16_t length, uint8_t *p_value);
int32_t app_ble_gatt_data_send_notify(uint16_t local_handle,uint16_t idx, uint16_t length, uint8_t *p_value);
uint16_t app_ble_stop_adv_without_id(void);
void app_ble_start_scan_with_param(app_ble_scan_param_t *param);
uint16_t app_ble_stop_adv(uint8_t adv_idx);
void ble_set_callback(cb_fun cb);
uint8_t app_ble_get_scan_status(void);
void app_ble_set_uuid(app_uuids *uuid);
bool app_ble_check_target_addr();
app_uuids* app_ble_get_uuid();
app_uuids* app_ble_get_uuid_handle();
void app_ble_start_connect(uint8_t own_addr_type);
uint16_t app_ble_get_mtu(uint8_t conidx);
int app_ble_scan_start(app_ble_scan_callback_t cb);
int app_ble_scan_stop(void);
int app_ble_set_max_mtu(uint16_t mtu);
void app_ble_set_ble_open(uint16_t value);
void app_ble_config_scanning();
int  app_ble_gatt_add_svc_helper(uint16_t *start_hdl, uint8_t nb_att, uint8_t vendor,ble_gatt_att_reg_t *atts);
bool is_matter_activity(uint16_t param);
void adv_media_connect_start();
void adv_media_beacon_start();
void ble_matter_event_callback_reg(matter_event_callback_t cb);
#if CONFIG_ENABLE_ASR_APP_MESH
void app_mesh_control_fan(uint8_t on_off);
void app_mesh_control_light(uint8_t on_off);
#endif
/// @} APP

#endif // APP_H_
