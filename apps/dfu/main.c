// https://devzone.nordicsemi.com/guides/short-range-guides/b/software-development-kit/posts/getting-started-with-nordics-secure-dfu-bootloader
/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @example examples/ble_peripheral/ble_app_buttonless_dfu
 *
 * @brief Secure DFU Buttonless Service Application main file.
 *
 * This file contains the source code for a sample application using the proprietary
 * Secure DFU Buttonless Service. This is a template application that can be modified
 * to your needs. To extend the functionality of this application, please find
 * locations where the comment "// YOUR_JOB:" is present and read the comments.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_bootloader_info.h"
#include "nrfx_gpiote.h"

#include "id130c_pins.h"

#define DEVICE_NAME                     "Nordic_Buttonless"                         /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                       /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                300                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_DURATION                6000                                        /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                            /**< Handle of the current connection. */

// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}};

static bool shutdown_handler(nrf_pwr_mgmt_evt_t event) {
  NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
  return true;
}
NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, 0);

static void dfu_state_observer(nrf_sdh_state_evt_t state, void * p_context) {
  if (state == NRF_SDH_EVT_STATE_DISABLED) {
    // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
    nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);
    //Go to system off.
    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
  }
}
NRF_SDH_STATE_OBSERVER(m_dfu_state_observer, 0) = { .handler = dfu_state_observer, };

static void advertising_config_get(ble_adv_modes_config_t * p_config) {
  memset(p_config, 0, sizeof(ble_adv_modes_config_t));
  
  p_config->ble_adv_fast_enabled  = true;
  p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
  p_config->ble_adv_fast_timeout  = APP_ADV_DURATION;
}

static void disconnect(uint16_t conn_handle, void * p_context) {
  UNUSED_PARAMETER(p_context);
  
  ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
  if (err_code != NRF_SUCCESS) {
    NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
  } else {
    NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
  }
}

static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event) {
  switch (event) {
  case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE: {
    NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
      
    // Prevent device from advertising on disconnect.
    ble_adv_modes_config_t config;
    advertising_config_get(&config);
    config.ble_adv_on_disconnect_disabled = true;
    ble_advertising_modes_config_set(&m_advertising, &config);
      
    // Disconnect all other bonded devices that currently are connected.
    // This is required to receive a service changed indication
    // on bootup after a successful (or aborted) Device Firmware Update.
    uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
    NRF_LOG_INFO("Disconnected %d links.", conn_count);
    break;
  }
  case BLE_DFU_EVT_BOOTLOADER_ENTER:
    NRF_LOG_INFO("Device will enter bootloader mode.");
    break;
  case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
    NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
    break;
  case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
    NRF_LOG_ERROR("Request to send a response to client failed.");
    APP_ERROR_CHECK(false);
    break;
  default:
    NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
    break;
  }
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name) {
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void pm_evt_handler(pm_evt_t const * p_evt) {
  pm_handler_on_pm_evt(p_evt);
  pm_handler_disconnect_on_sec_failure(p_evt);
  pm_handler_flash_clean(p_evt);
}

static void gap_params_init(void) {
  uint32_t                err_code;
  ble_gap_conn_params_t   gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;
  
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  
  err_code = sd_ble_gap_device_name_set(&sec_mode,
					(const uint8_t *)DEVICE_NAME,
					strlen(DEVICE_NAME));
  APP_ERROR_CHECK(err_code);
  
  err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_WATCH_SPORTS_WATCH);
  APP_ERROR_CHECK(err_code);

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
  
  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  APP_ERROR_CHECK(err_code);
}

static void nrf_qwr_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

static void services_init(void) {
  uint32_t                  err_code;
  nrf_ble_qwr_init_t        qwr_init  = {0};
  ble_dfu_buttonless_init_t dfus_init = {0};
  
  // Initialize Queued Write Module.
  qwr_init.error_handler = nrf_qwr_error_handler;
  
  err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
  APP_ERROR_CHECK(err_code);
  
  dfus_init.evt_handler = ble_dfu_evt_handler;
  
  err_code = ble_dfu_buttonless_init(&dfus_init);
  APP_ERROR_CHECK(err_code);
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
  uint32_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}

static void conn_params_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

static void conn_params_init(void) {
  uint32_t               err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params                  = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail             = false;
  cp_init.evt_handler                    = on_conn_params_evt;
  cp_init.error_handler                  = conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
  switch (ble_adv_evt) {
  case BLE_ADV_EVT_FAST:
    break;
    
  case BLE_ADV_EVT_IDLE:
    NRF_LOG_INFO("Advertising stopped");
    break;
    
  default:
    break;
  }
}

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context) {
  uint32_t err_code = NRF_SUCCESS;
    
  switch (p_ble_evt->header.evt_id) {
  case BLE_GAP_EVT_DISCONNECTED:
    break;
    
  case BLE_GAP_EVT_CONNECTED:
    m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
    APP_ERROR_CHECK(err_code);
    break;
    
  case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
    NRF_LOG_DEBUG("PHY update request.");
    ble_gap_phys_t const phys =
      {
	.rx_phys = BLE_GAP_PHY_AUTO,
	.tx_phys = BLE_GAP_PHY_AUTO,
      };
    err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
    APP_ERROR_CHECK(err_code);
    break;
  }
  case BLE_GATTC_EVT_TIMEOUT:
    // Disconnect on GATT Client timeout event.
    NRF_LOG_DEBUG("GATT Client Timeout.");
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
				     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;
  case BLE_GATTS_EVT_TIMEOUT:
    // Disconnect on GATT Server timeout event.
    NRF_LOG_DEBUG("GATT Server Timeout.");
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
				     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;
  default:
    break;
  }
}

static void ble_stack_init(void) {
  ret_code_t err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);
  
  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);
  
  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);
  
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

static void peer_manager_init() {
  ble_gap_sec_params_t sec_param;
  ret_code_t           err_code;

  err_code = pm_init();
  APP_ERROR_CHECK(err_code);

  memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

  // Security parameters to be used for all security procedures.
  sec_param.bond           = SEC_PARAM_BOND;
  sec_param.mitm           = SEC_PARAM_MITM;
  sec_param.lesc           = SEC_PARAM_LESC;
  sec_param.keypress       = SEC_PARAM_KEYPRESS;
  sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
  sec_param.oob            = SEC_PARAM_OOB;
  sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
  sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
  sec_param.kdist_own.enc  = 1;
  sec_param.kdist_own.id   = 1;
  sec_param.kdist_peer.enc = 1;
  sec_param.kdist_peer.id  = 1;

  err_code = pm_sec_params_set(&sec_param);
  APP_ERROR_CHECK(err_code);

  err_code = pm_register(pm_evt_handler);
  APP_ERROR_CHECK(err_code);
}

static void advertising_init(void) {
  uint32_t               err_code;
  ble_advertising_init_t init;

  memset(&init, 0, sizeof(init));

  init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
  init.advdata.include_appearance      = true;
  init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
  init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

  advertising_config_get(&init.config);

  init.evt_handler = on_adv_evt;

  err_code = ble_advertising_init(&m_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

void pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  ret_code_t err_code;

  switch(pin) {
  case ID130C_PIN_TOUCH_INT_N :
  case ID130C_PIN_CHARGE_PRESENT_N:
    if (m_advertising.adv_mode_current == BLE_ADV_MODE_IDLE) {
      err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
      APP_ERROR_CHECK(err_code);
      NRF_LOG_INFO("Touch detected - advertising started");
    } else {
      NRF_LOG_INFO("Touch detected");
    }
    break;
  default:
    break;
  }
}

int main(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();

  // Initialize the async SVCI interface to bootloader before any interrupts are enabled.
  err_code = ble_dfu_buttonless_async_svci_init();
  APP_ERROR_CHECK(err_code);

  APP_ERROR_CHECK(app_timer_init());
  APP_ERROR_CHECK(nrf_pwr_mgmt_init());
  ble_stack_init();
  peer_manager_init();
  gap_params_init();
  APP_ERROR_CHECK(nrf_ble_gatt_init(&m_gatt, NULL));
  advertising_init();
  services_init();
  conn_params_init();
  APP_ERROR_CHECK(nrfx_gpiote_init());

  NRF_LOG_INFO("Buttonless DFU Application started.");

  // Start execution.
  if (m_advertising.adv_mode_current == BLE_ADV_MODE_IDLE) {
    err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEBUG("advertising is started");
  }
#if 0
  nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(false);
  out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
  nrfx_gpiote_out_init(ID130C_PIN_TOUCH_EN_N, &out_config);
  nrfx_gpiote_out_init(ID130C_PIN_CHARGE_EN_N, &out_config);
  nrfx_gpiote_in_config_t in_config_hitolo = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
  in_config_hitolo.pull = NRF_GPIO_PIN_PULLUP;
  APP_ERROR_CHECK(nrfx_gpiote_in_init(ID130C_PIN_TOUCH_INT_N, &in_config_hitolo, pin_handler));
  in_config_hitolo.pull = NRF_GPIO_PIN_NOPULL;
  APP_ERROR_CHECK(nrfx_gpiote_in_init(ID130C_PIN_CHARGE_PRESENT_N, &in_config_hitolo, pin_handler));
  nrfx_gpiote_in_event_enable(ID130C_PIN_TOUCH_INT_N, true);

  for (unsigned ix=0; ix < 32; ++ix) {
    nrfx_gpiote_in_init(ix, &in_config_hitolo, pin_handler);
  }
#else
  nrf_gpio_range_cfg_input(0, 31, NRF_GPIO_PIN_PULLUP);
#endif
  // Enter main loop.
  unsigned ix=0;
  for (;;) {
    if (NRF_LOG_PROCESS() == false) {
      nrf_pwr_mgmt_run();
    }
    NRF_LOG_INFO("%d: %08x", ix++, NRF_P0->IN);
  }
}
