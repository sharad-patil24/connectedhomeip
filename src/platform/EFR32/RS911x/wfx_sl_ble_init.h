/*******************************************************************************
* @file  wfx_sl_ble_init.h
* @brief
*******************************************************************************
* # License
* <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/
/*************************************************************************
 *
 */

/**
 * Include files
 * */

#ifndef WFX_SL_BLE_INIT
    #define WFX_SL_BLE_INIT
#define RSI_BLE_ENABLE 1


// BLE include file to refer BLE APIs
#include <stdbool.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "timers.h"
#include "wfx_host_events.h"
#include <rsi_driver.h>
#include <rsi_wlan_non_rom.h>
#include <rsi_bt_common_apis.h>
#include <rsi_ble.h>
#include <rsi_ble_common_config.h>
#include <rsi_bootup_config.h>
#include <rsi_ble_apis.h>
#include <rsi_wlan_apis.h>
#include <rsi_wlan_config.h>
#include <rsi_bt_common.h>
#include <rsi_common_apis.h>
#include <rsi_ble_config.h>
#include <string.h>
#include "rsi_ble_config.h"
#include "wfx_rsi.h"

#ifdef RSI_M4_INTERFACE
#include "rsi_board.h"
#endif

int32_t wfx_sl_module_init(void);


#endif