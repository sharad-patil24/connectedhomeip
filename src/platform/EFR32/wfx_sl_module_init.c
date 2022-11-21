/*******************************************************************************
* @file  wfx_sl_module_init.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @brief : This file contains the code to init the rs911x all driver and modules.
 =================================================================================*/

#include "wfx_sl_module_init.h"

rsi_semaphore_handle_t sl_ble_sem;
rsi_semaphore_handle_t sl_wfx_sem;


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

    rsi_semaphore_create(&sl_ble_sem, 0);
    rsi_semaphore_create(&sl_wfx_sem, 0);

    WFX_RSI_LOG("%s: starting(HEAP_SZ = %d)", __func__, SL_HEAP_SIZE);

    //! Driver initialization
    status = rsi_driver_init(wfx_rsi_drv_buf, WFX_RSI_BUF_SZ);
    if ((status < RSI_DRIVER_STATUS) || (status > WFX_RSI_BUF_SZ))
    {
        WFX_RSI_LOG("%s: error: RSI Driver initialization failed with status: %02x", __func__, status);
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
                                         1, driverRsiTaskStack, &driverRsiTaskBuffer);
    if (NULL == wfx_rsi.drv_task)
    {
        WFX_RSI_LOG("%s: error: Create the driver task failed", __func__);
        return RSI_ERROR_INVALID_PARAM;
    }

    /* Initialize WiSeConnect or Module features. */
    WFX_RSI_LOG("%s: rsi_wireless_init", __func__);
    if ((status = rsi_wireless_init(OPER_MODE_0, RSI_OPERMODE_WLAN_BLE)) != RSI_SUCCESS)
    {
        WFX_RSI_LOG("%s: error: Initialize WiSeConnect failed with status: %02x", __func__, status);
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

    rsi_semaphore_post(&sl_ble_sem);
    rsi_semaphore_post(&sl_wfx_sem);

    WFX_RSI_LOG("%s complete", __func__);
    rsi_task_destroy((rsi_task_handle_t *)wfx_rsi.init_task);
    return RSI_SUCCESS;
}