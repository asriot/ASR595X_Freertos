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
#ifndef SONATA_STACK_FIXED_CONFIG_H_
#define SONATA_STACK_FIXED_CONFIG_H_


/*
 * DEFINES
 ****************************************************************************************
 */
//////////////////////////////////////////
/*do not alter*/
#define CFG_BLE

#define CFG_EMB

#define CFG_ALLROLES

#if ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_H) )
#define CFG_ACT            11
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_M) )
#define CFG_ACT            8
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_L2) )
#define CFG_ACT            5
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_L) )
#define CFG_ACT            2
#elif defined(CFG_PLF_RV32)
#define CFG_ACT            12
#else
#define CFG_ACT            8
#endif // #if ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_H) )

#if ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_H) )
#define CFG_RAL            10
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_M) )
#define CFG_RAL            10
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_L2) )
#define CFG_RAL            5
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_L) )
#define CFG_RAL            1
#elif defined(CFG_PLF_RV32)
#define CFG_RAL            50
#else
#define CFG_RAL            10
#endif // #if ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_H) )

#define CFG_CON_CTE_REQ

#define CFG_CON_CTE_RSP

#define CFG_CONLESS_CTE_TX

#define CFG_CONLESS_CTE_RX

#define CFG_AOD

#define CFG_AOA

#define CFG_WLAN_COEX

#if ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_H) )
#define CFG_NB_PRF         10
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_M) )
#define CFG_NB_PRF         10
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_L2) )
#define CFG_NB_PRF         10
#elif ( defined(CFG_PLF_SONATA) && defined(CFG_BLE_HL_LL_ROM_L) )
#define CFG_NB_PRF         10
#elif defined(CFG_PLF_RV32)
#define CFG_NB_PRF         10
#else
#define CFG_NB_PRF         10
#endif // #if defined(CFG_PLF_SONATA)

#endif /*SONATA_STACK_FIXED_CONFIG_H_*/
