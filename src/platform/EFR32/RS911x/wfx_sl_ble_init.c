/*******************************************************************************
* @file  wfx_sl_ble_init.c
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

 /*================================================================================
 * @brief : This file contains example application for Wlan Station BLE
 * Provisioning
 * @section Description :
 * This application explains how to get the WLAN connection functionality using
 * BLE provisioning.
 * Silicon Labs Module starts advertising and with BLE Provisioning the Access Point
 * details are fetched.
 * Silicon Labs device is configured as a WiFi station and connects to an Access Point.
 =================================================================================*/

#include "wfx_sl_ble_init.h"

// application defines
extern uint8_t ble_connected;
// Memory to initialize driver
uint8_t bt_global_buf[BT_GLOBAL_BUFF_LEN];

static uint8_t wfx_rsi_drv_buf[WFX_RSI_BUF_SZ];

/* Rsi driver Task will use as its stack */
StackType_t driverRsiTaskStack[WFX_RSI_WLAN_TASK_SZ] = { 0 };

/* Structure that will hold the TCB of the wfxRsi Task being created. */
StaticTask_t driverRsiTaskBuffer;

int32_t wfx_sl_module_init(void)
{
    int32_t status;
    uint8_t buf[RSI_RESPONSE_HOLD_BUFF_SIZE];
    extern void rsi_hal_board_init(void);

    WFX_RSI_LOG("%s: starting(HEAP_SZ = %d)", __func__, SL_HEAP_SIZE);
    //! Driver initialization
    status = rsi_driver_init(wfx_rsi_drv_buf, WFX_RSI_BUF_SZ);
    if ((status < RSI_DRIVER_STATUS) || (status > WFX_RSI_BUF_SZ))
    {
        WFX_RSI_LOG("%s: error: RSI drv init failed with status: %02x", __func__, status);
        return status;
    }

    WFX_RSI_LOG("%s: rsi_device_init", __func__);
    /* ! Redpine module intialisation */
    if ((status = rsi_device_init(LOAD_NWP_FW)) != RSI_SUCCESS)
    {
        WFX_RSI_LOG("%s: error: rsi_device_init failed with status: %02x", __func__, status);
        return status;
    }
    WFX_RSI_LOG("%s: start wireless drv task", __func__);
    /*
     * Create the driver task
     */
    wfx_rsi.drv_task = xTaskCreateStatic((TaskFunction_t) rsi_wireless_driver_task, "rsi_drv", WFX_RSI_WLAN_TASK_SZ, NULL,
                                         WLAN_TASK_PRIORITY, driverRsiTaskStack, &driverRsiTaskBuffer);
    if (NULL == wfx_rsi.drv_task)
    {
        WFX_RSI_LOG("%s: error: rsi_wireless_driver_task failed", __func__);
        return RSI_ERROR_INVALID_PARAM;
    }

    /* Initialize WiSeConnect or Module features. */
    WFX_RSI_LOG("%s: rsi_wireless_init", __func__);
    if ((status = rsi_wireless_init(OPER_MODE_0, COEX_MODE_0)) != RSI_SUCCESS)
    {
        WFX_RSI_LOG("%s: error: rsi_wireless_init failed with status: %02x", __func__, status);
        return status;
    }

    WFX_RSI_LOG("%s: get FW version..", __func__);
    /*
     * Get the MAC and other info to let the user know about it.
     */
    if (rsi_wlan_get(RSI_FW_VERSION, buf, sizeof(buf)) != RSI_SUCCESS)
    {
        WFX_RSI_LOG("%s: error: rsi_wlan_get(RSI_FW_VERSION) failed with status: %02x", __func__, status);
        return status;
    }

    buf[sizeof(buf) - 1] = 0;
    WFX_RSI_LOG("%s: RSI firmware version: %s", __func__, buf);
    //! Send feature frame
    if ((status = rsi_send_feature_frame()) != RSI_SUCCESS)
    {
        WFX_RSI_LOG("%s: error: rsi_send_feature_frame failed with status: %02x", __func__, status);
        return status;
    }

    WFX_RSI_LOG("%s: sent rsi_send_feature_frame", __func__);
    /* initializes wlan radio parameters and WLAN supplicant parameters.
     */
    (void) rsi_wlan_radio_init(); /* Required so we can get MAC address */
    if ((status = rsi_wlan_get(RSI_MAC_ADDRESS, &wfx_rsi.sta_mac.octet[0], RESP_BUFF_SIZE)) != RSI_SUCCESS)
    {
        WFX_RSI_LOG("%s: error: rsi_wlan_get failed with status: %02x", __func__, status);
        return status;
    }
    return RSI_SUCCESS;
}