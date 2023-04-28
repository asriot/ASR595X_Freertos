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
 * @file user_platform.h
 *
 * @brief This file contains the user's hardware platform definition
 *
 ****************************************************************************************
 */
 #ifndef _USER_PLF_H_
 #define _USER_PLF_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "arch.h"


/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */

/** hardware platform **/
#define ASR5601A0V1_EVB_Q48PIN      0
#define ASR5601A0V1_EVB_Q32PIN      0     // default 32PIN for auto test
#define ASR5601A0V1_EVB_Q56PIN      0

/** supported peripherals **/

/// UART 1
#define PLF_UART1             0

/// UART 2
#define PLF_UART2             0

/// BLE TRANSPORT
#if defined(SONATA_CFG_BLE_TRANSPORT)
#define PLF_BLE_TRANSPORT     1
#else
#define PLF_BLE_TRANSPORT     0
#endif //CFG_SONATA_BLE_TRANSPORT

/// DIRECTION FINDING
#if defined(SONATA_CFG_BLE_DIRECTION_FINDING)
#define PLF_DF                1
#else
#define PLF_DF                0
#endif //CFG_SONATA_BLE_TRANSPORT


/// WLAN COEXIT
#if defined(SONATA_CFG_BLE_WLAN_COEX)
#define PLF_PTA               1
#else
#define PLF_PTA               0
#endif //CFG_SONATA_BLE_WLAN_COEX

///user platform debug
#define USER_PLF_DBG          1 // default open
#if (USER_PLF_DBG)
#define USER_PLF_TRC    printf
#else
#define USER_PLF_TRC(...)
#endif //(USER_PLF_DBG)


/*
 * EXPORTED VARIABLES DECLARATION
 ****************************************************************************************
 */
extern uint32_t app_clk_sel;
extern void (*pf_app_clk_reconfig)(void);

/*
 * EXPORTED FUNCTION DECLARATION
 ****************************************************************************************
 */
extern void user_peri_init(void);
extern void user_peri_deinit(void);

 #endif //_USER_PLF_H_

