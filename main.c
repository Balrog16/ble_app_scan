#include "app_timer.h"
#include "ble.h"
#include "bsp.h" //For BSP functions
#include "nordic_common.h"
#include "nrf_ble_scan.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define APP_BLE_CONN_CFG_TAG                                                   \
  1 /**< Tag that refers to the BLE stack configuration set with @ref          \
       sd_ble_cfg_set. The default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
#define APP_BLE_OBSERVER_PRIO                                                  \
  3 /**< BLE observer priority of the application. There is no need to modify  \
       this value. */

#define SCAN_WINDOW MSEC_TO_UNITS(5, UNIT_0_625_MS)   /** SCAN WINDOW **/
#define SCAN_INTERVAL MSEC_TO_UNITS(5, UNIT_0_625_MS) /** SCAN Interval **/

NRF_BLE_SCAN_DEF(m_scan); /**< Scanning Module instance. */
// Scan parameters requested for scanning
static ble_gap_scan_params_t const m_scan_param = {
    .extended = 1,  // Ready for exended advertisements and so receive adv
                    // packets on secondary adv channels
    .active = 0x01, // Decide to send a scan request or not (0x01)
    .interval = SCAN_INTERVAL,
    .window = SCAN_WINDOW,
    .timeout = 0x0000, // No timeout, scan forever!!
    .scan_phys = BLE_GAP_PHY_1MBPS,
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL, // No filters
};

/**@brief Function for handling Scanning Module events.
 */
static void scan_evt_handler(scan_evt_t const *p_scan_evt) {

  switch (p_scan_evt->scan_evt_id) {
    case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
      NRF_LOG_INFO("Scan event timed-out");
      break;
    default:
      break;
  }
}

/**@brief Function to start scanning. */
static void scan_start(void) {
  ret_code_t ret;

  ret = nrf_ble_scan_start(&m_scan); // Start Scan
  APP_ERROR_CHECK(ret);

  ret = bsp_indication_set(BSP_INDICATE_SCANNING); // blink LED
  APP_ERROR_CHECK(ret);
}

/**@brief Function for intializing scan.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
void scan_init() {
  ret_code_t err_code;
  nrf_ble_scan_init_t scan_init;

  memset(&scan_init, 0, sizeof(scan_init));

  scan_init.p_scan_param = &m_scan_param; // Supply scan params
  scan_init.connect_if_match = false;     // Don't connect
  scan_init.conn_cfg_tag = APP_BLE_CONN_CFG_TAG;

  err_code = nrf_ble_scan_init(&m_scan, &scan_init, scan_evt_handler);
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context) {
  // GAP EVTS can be found here
  // https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.s110.api.v8.0.0%2Fgroup___b_l_e___g_a_p.html
  ret_code_t err_code;
  switch (p_ble_evt->header.evt_id) {
  case BLE_GAP_EVT_ADV_REPORT:
    // This event is triggered as sooon as the scanner receives an advertisement
    // packet More information on scanning -
    // https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.2.0%2Flib_ble_scan.html
    NRF_LOG_INFO("Advertisement received");
    //  If this event is caught here, then nrf_ble_scan_start has to be called
    err_code = nrf_ble_scan_start(&m_scan); // Start Scan
    APP_ERROR_CHECK(err_code);
    break;
  default:
    break;
  }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void) {
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler,
                       NULL);
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the timer. */
static void timer_init(void) {
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the nrf log module. */
static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void) {
  log_init();
  timer_init();
  power_management_init();

  /** BLE stack and other related worl **/
  ble_stack_init();
  scan_init();
  scan_start();
}