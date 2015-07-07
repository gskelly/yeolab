/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_adc.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_bms.h"
#include "ble_dis.h"
#include "ble_bas.h"
#include "spi_master.h"
#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif // BLE_DFU_APP_SUPPORT
#if (defined(ADS1291) || defined(ADS1292))
#include "ads1291-2.h"
#endif // ADS1291 or ADS1292
#ifndef SPI_MASTER_0_ENABLE
#include "sensorsim.h"
#endif // SPI_MASTER_0_ENABLE
#include "ble_conn_params.h"
#include "app_scheduler.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_gpiote.h"
#include "bsp.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 1                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define WAKEUP_BUTTON_ID                0                                           /**< Button used to wake up the application. */

#define DEVICE_NAME                     "EPSensor"                           				/**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME								"VCU-BNE-Lab"																/**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                3200                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 2 s). */
#define APP_ADV_TIMEOUT_IN_SECONDS      0                                           /**< The advertising timeout (in units of seconds). */

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#ifdef BSP_APP_TIMERS_NUMBER
#define APP_TIMER_MAX_TIMERS            (6 + BSP_APP_TIMERS_NUMBER)                 /**< Maximum number of simultaneously created timers. */
#else
#define APP_TIMER_MAX_TIMERS						6
#endif // BSP_APP_TIMERS_NUMBER
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL     APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)/**< Battery level measurement interval (ticks). */

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS     1200                                     /**< Reference voltage (in millivolts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION      3                                        /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
/**@brief Macro to convert the result of ADC conversion in millivolts.
 *
 * @param[in]  ADC_VALUE   ADC result.
 *
 * @retval     Result converted to millivolts.
 */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 255) * ADC_PRE_SCALING_COMPENSATION)


#define BODY_VOLTAGE_SAMPLE_INTERVAL   	APP_TIMER_TICKS(4, APP_TIMER_PRESCALER) 	 /**< Biopotential measurement sampling interval (ticks). */
#define BODY_VOLTAGE_TX_INTERVAL      	APP_TIMER_TICKS(30, APP_TIMER_PRESCALER) 	 /**< Biopotential measurement transmission interval (ticks). */
#define MIN_BODY_VOLTAGE                (0xFF800000)                               /**< Minimum voltage as returned by the simulated measurement function. */
#define MAX_BODY_VOLTAGE                (0x007FFFFF)                               /**< Maximum voltage as returned by the simulated measurement function. */
#define BODY_VOLTAGE_INCREMENT          12000		                                     /**< Value by which the voltage is incremented/decremented for each call to the simulated measurement function. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)            	/**< Minimum acceptable connection interval (8 msec). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)           	/**< Maximum acceptable connection interval (8 msec). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_GPIOTE_MAX_USERS            3                                           /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)    /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT);                                     /** When having DFU Service support in application the Service Changed Characteristic should always be present. */
#endif // BLE_DFU_APP_SUPPORT

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#if (defined(ADS1292) && defined(ADS1291))
#error "Only one of ADS1291 or ADS1292 should be defined"
#endif

#ifdef BOARD_PCA10028
#undef SPIM0_SCK_PIN
#undef SPIM0_MOSI_PIN
#undef SPIM0_MISO_PIN
#undef SPIM0_SS_PIN
#define SPIM0_SCK_PIN       						29     /**< SPI clock GPIO pin number - D13 on Arduino */
#define SPIM0_MOSI_PIN      						25     /**< SPI Master Out Slave In GPIO pin number - D11 on Arduino */
#define SPIM0_MISO_PIN      						28     /**< SPI Master In Slave Out GPIO pin number - D12 on Arduino */
#define SPIM0_SS_PIN        						13     /**< SPI Slave Select GPIO pin number - D1 on Arduino */
#endif

static ble_gap_sec_params_t             m_sec_params;                               /**< Security requirements for this application. */
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */

static ble_bms_t												m_bms;
static ble_bas_t												m_bas;
static dm_application_instance_t        m_app_handle;                              /**< Application identifier allocated by device manager */

static app_timer_id_t                   m_battery_timer_id;                        /**< Battery timer. */
static app_timer_id_t                   m_bvm_send_timer_id;                   		 /**< Biopotential measurement transmission timer. */
static app_timer_id_t										m_sampling_timer_id;											 /**< Biopotential sample acquisition timer. */

static app_gpiote_user_id_t							m_sampling_gpiote_id;
#ifndef SPI_MASTER_0_ENABLE
static sensorsim_cfg_t              		m_body_voltage_sim_cfg;                    /**< Biopotential sensor simulator configuration. */
static sensorsim_state_t            		m_body_voltage_sim_state;                  /**< Biopotential sensor simulator state. */
#endif

static bool                             m_memory_access_in_progress = false;       /**< Flag to keep track of ongoing operations on persistent memory. */
#ifdef BLE_DFU_APP_SUPPORT    
static ble_dfu_t                        m_dfus;                                    /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT 

#if defined(SPI_MASTER_0_ENABLE)

/** @def  TX_RX_MSG_LENGTH
 * Max number of bytes to transmit and receive in one chained SPI transaction.
 * This should be at least enough for a single-sample read. (3+3*Nchannels bytes) */
#if defined(ADS1291)
#define TX_RX_MSG_LENGTH         				7
#elif defined(ADS1292)
#define TX_RX_MSG_LENGTH								10
#endif

static uint8_t 													m_tx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master TX buffer. */
static uint8_t 													m_rx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master RX buffer. */

#if defined(ADS1291) || defined(ADS1292)
//static bool 													m_rdatac 			= false;
//static uint8_t 												m_gain 				= (ADS1291_2_REGDEFAULT_CH1SET  & 0x70);
//static uint8_t  											m_sample_rate = (ADS1291_2_REGDEFAULT_CONFIG1 & 0x07);
static bool 														m_register_rewrite_in_progress = false;
static bool															m_drdy = false;
#endif // ADS1291 or ADS1292
#endif // defined(SPI_MASTER_0_ENABLE)

// YOUR_JOB: Modify these according to requirements (e.g. if other event types are to pass through
//           the scheduler).
#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)                   /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE                10                                          /**< Maximum number of events in the scheduler queue. */



/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}



/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
/*
// YOUR_JOB: Uncomment this function and make it handle error situations sent back to your
//           application by the services it uses.
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
} */



/**@brief Function for handling the ADC interrupt.
 *
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
void ADC_IRQHandler(void)
{
    if (nrf_adc_conversion_finished())
    {
        uint8_t  adc_result;
        uint16_t batt_lvl_in_milli_volts;
        uint8_t  percentage_batt_lvl;
        uint32_t err_code;

        nrf_adc_conversion_event_clean();

        adc_result = nrf_adc_result_get();

        batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result);
        percentage_batt_lvl     = battery_level_in_percent(batt_lvl_in_milli_volts) + 270;

        err_code = ble_bas_battery_level_update(&m_bas, percentage_batt_lvl);
        if (
            (err_code != NRF_SUCCESS)
            &&
            (err_code != NRF_ERROR_INVALID_STATE)
            &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS)
            &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
           )
        {
            APP_ERROR_HANDLER(err_code);
        }
    }
}

/**@brief Function for configuring ADC to do battery level conversion.
 */
static void adc_configure(void)
{
    uint32_t err_code;
    nrf_adc_config_t adc_config = NRF_ADC_CONFIG_DEFAULT;

    // Configure ADC
    adc_config.reference  = NRF_ADC_CONFIG_REF_VBG;
    adc_config.resolution = NRF_ADC_CONFIG_RES_8BIT;
    adc_config.scaling    = NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD;
    nrf_adc_configure(&adc_config);

    // Enable ADC interrupt
    nrf_adc_int_enable(ADC_INTENSET_END_Msk);
    err_code = sd_nvic_ClearPendingIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_EnableIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    nrf_adc_start();
}

/**@brief Function for handling the Body Voltage Measurement timer timeout.
 *
 * @details This function will be called each time the voltage measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void body_voltage_meas_timeout_handler(void * p_context)
{
    static uint32_t cnt = 0;
		uint32_t        err_code;

    UNUSED_PARAMETER(p_context);
	
    cnt++;
    err_code = ble_bms_body_voltage_measurement_send(&m_bms);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        )
    {
        APP_ERROR_HANDLER(err_code);
    }
}

/**@brief Function for acquiring a Body Voltage Measurement sample.
 *
 * @details If SPI is enabled, this function will use it. Otherwise, it will use the
 *          sensor simulator.
 */
static __inline void get_bvm_sample(void)
{
	if (!m_register_rewrite_in_progress)
	{
		uint32_t body_voltage;
#ifdef SPI_MASTER_0_ENABLE	
		uint32_t err_code;
		
		// Load TX buffer with RDATA command
		m_tx_data_spi[0] = ADS1291_2_OPC_RDATA;
		
		// Clock out RDATA command + dummy bytes. In same transaction, clock in one dummy byte and channel data.	
		err_code = spi_master_send_recv(SPI_MASTER_0, m_tx_data_spi, TX_RX_MSG_LENGTH, m_rx_data_spi, TX_RX_MSG_LENGTH);
		APP_ERROR_CHECK(err_code);

		body_voltage = (m_rx_data_spi[4] << 8) | m_rx_data_spi[5];
#else
		body_voltage = sensorsim_measure(&m_body_voltage_sim_state, &m_body_voltage_sim_cfg);
		body_voltage = body_voltage >> 8;
#endif // SPI_MASTER_0_ENABLE
		/* 
	   * Input range is, in general for signed integers: 
	   * 					[-(LSB*(2^(bits-1))):+(LSB*(2^(bits-1)))]/gain
	   * where LSB is the minimum detectable change (resolution) and equals Vref/(2^(bits-1)-1).
	   * For signed int16, Vref = 2.42 V and gain = 12, this gives an input range of +/-201 mV and resolution of 6 uV.
	   *
	   */
		ble_bms_bvm_add(&m_bms, (int16_t)body_voltage);
	}
}

/**@brief Function for acquiring a Body Voltage Measurement sample when continuous data read is enabled.
 *
 * @details If SPI is enabled, this function will use it. Otherwise, it will use the
 *          sensor simulator.
 */
static void __inline get_bvm_sample_rdatac(void)
{
	if (!m_register_rewrite_in_progress)
	{
		uint32_t body_voltage;
#ifdef SPI_MASTER_0_ENABLE	
		uint32_t err_code;
		
		// Clock out RDATA command + dummy bytes. In same transaction, clock in one dummy byte and channel data.	
		err_code = spi_master_send_recv(SPI_MASTER_0, m_tx_data_spi, TX_RX_MSG_LENGTH-1, m_rx_data_spi, TX_RX_MSG_LENGTH-1);
		APP_ERROR_CHECK(err_code);

		body_voltage = (m_rx_data_spi[3] << 8) | m_rx_data_spi[4];
#else
		body_voltage = sensorsim_measure(&m_body_voltage_sim_state, &m_body_voltage_sim_cfg);
		body_voltage = body_voltage >> 8;
#endif // SPI_MASTER_0_ENABLE
		/* 
	   * Input range is, in general for signed integers: 
	   * 					[-(LSB*(2^(bits-1))):+(LSB*(2^(bits-1)))]/gain
	   * where LSB is the minimum detectable change (resolution) and equals Vref/(2^(bits-1)-1).
	   * For signed int16, Vref = 2.42 V and gain = 12, this gives an input range of +/-201 mV and resolution of 6 uV.
	   *
	   */
		ble_bms_bvm_add(&m_bms, (int16_t)body_voltage);
	}
}

static void drdy_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low)
{
	UNUSED_PARAMETER(event_pins_low_to_high);
	UNUSED_PARAMETER(event_pins_high_to_low);
	m_drdy = true;
}

/**@brief Function for handling the sample acquisition timer timeout.
 *
 * @details This function will be called each time the sampling timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void sampling_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    get_bvm_sample();
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
		uint32_t err_code;
	
    // Initialize timer module, making it use the scheduler
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, true);

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
		*/
    err_code = app_timer_create(&m_battery_timer_id, 
																APP_TIMER_MODE_REPEATED,
																battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
		
		err_code = app_timer_create(&m_bvm_send_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                body_voltage_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
		
		err_code = app_timer_create(&m_sampling_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                sampling_timer_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t             err_code;
		uint32_t						 count;
    ble_gap_adv_params_t adv_params;

	  // Verify if there is any flash access pending, if yes delay starting advertising until
    // it's complete.
    err_code = pstorage_access_status_get(&count);
    APP_ERROR_CHECK(err_code);

    if (count != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }
	
    // Start advertising
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.interval    = APP_ADV_INTERVAL;
    adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = sd_ble_gap_adv_start(&adv_params);
    APP_ERROR_CHECK(err_code);
    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case. */
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_HEART_RATE_SENSOR);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // YOUR_JOB: Use UUIDs for service(s) used in your application.
	
    ble_uuid_t adv_uuids[] =
    {
        {BLE_UUID_BIOPOTENTIAL_MEASUREMENT_SERVICE, BLE_UUID_TYPE_BLE},
        {BLE_UUID_BATTERY_SERVICE,            			BLE_UUID_TYPE_BLE},
        {BLE_UUID_DEVICE_INFORMATION_SERVICE, 			BLE_UUID_TYPE_BLE}
    };

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = flags;
    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
}

#ifdef BLE_DFU_APP_SUPPORT    
static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for loading application-specific context after establishing a secure connection.
 *
 * @details This function will load the application context and check if the ATT table is marked as 
 *          changed. If the ATT table is marked as changed, a Service Changed Indication
 *          is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * @param[in] p_handle The Device Manager handle that identifies the connection for which the context 
 *                     should be loaded.
 */
static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS)
    {
        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
        {
            err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
            {
                APP_ERROR_HANDLER(err_code);
            }
        }

        err_code = dm_application_context_delete(p_handle);
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code == DM_NO_APP_CONTEXT)
    {
        // No context available. Ignore.
    }
    else
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/** @snippet [DFU BLE Reset prepare] */
static void reset_prepare(void)
{
    uint32_t err_code;
    
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        err_code = bsp_indication_set(BSP_INDICATE_IDLE);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, then the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);
}
/** @snippet [DFU BLE Reset prepare] */
#endif // BLE_DFU_APP_SUPPORT   



/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the Battery Service events.
 *
 * @details This function will be called for all Battery Service events which are passed to the
 |          application.
 *
 * @param[in]   p_bas  Battery Service structure.
 * @param[in]   p_evt  Event received from the Battery Service.
 */
static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t *p_evt)
{
    uint32_t err_code;

    switch (p_evt->evt_type)
    {
        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
            // Start battery timer
            err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
            err_code = app_timer_stop(m_battery_timer_id);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}



/**@brief Function for handling the Biopotential Measurement Service events.
 *
 * @details This function will be called for all Biopotential Measurement Service events which
 |          are passed to the application.
 *
 * @param[in]   p_bms  Biopotential Measurement Service structure.
 * @param[in]   p_evt  Event received from the Biopotential Measurement Service.
 */
static void on_bms_evt(ble_bms_t * p_bms, ble_bms_evt_t *p_evt)
{
    uint32_t err_code;
#if (defined(ADS1291) || defined(ADS1292))
		uint8_t register_value;
#endif // ADS1291 or ADS1292

    switch (p_evt->evt_type)
    {
        case BLE_BMS_EVT_NOTIFICATION_ENABLED:
            // Start transmit timer
            err_code = app_timer_start(m_bvm_send_timer_id, BODY_VOLTAGE_TX_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_BMS_EVT_NOTIFICATION_DISABLED:
						// Stop transmit timer
            err_code = app_timer_stop(m_bvm_send_timer_id);
            APP_ERROR_CHECK(err_code);
            break;
				
				case BLE_BMS_EVT_GAIN_CHANGE:
#if (defined(ADS1291) || defined(ADS1292))
						/*
						if (m_rdatac)
						{
							ads1291_2_stop_rdatac();
						}
						*/
						m_register_rewrite_in_progress = true;
						// Read channel 1 settings register
						ads1291_2_rreg(ADS1291_2_REGADDR_CH1SET, 1, &register_value);
						// Modify gain bits
						switch (p_bms->gain)
						{
							case ADS1291_2_REG_CHNSET_GAIN_1:
							case ADS1291_2_REG_CHNSET_GAIN_2:
							case ADS1291_2_REG_CHNSET_GAIN_3:
							case ADS1291_2_REG_CHNSET_GAIN_4:
							case ADS1291_2_REG_CHNSET_GAIN_6:
							case ADS1291_2_REG_CHNSET_GAIN_8:	
							case ADS1291_2_REG_CHNSET_GAIN_12:
								//m_gain = *gatts_value.p_value;
								register_value = (register_value & ~0x70) | (p_bms->gain & 0x70); // Set bits 6:4 to chosen value
								break;
							default:
								break;
						}
						// Write modified register back to device
						ads1291_2_wreg(ADS1291_2_REGADDR_CH1SET, 1, &register_value);
						// Recalibrate offset for new gain
						ads1291_2_calibrate();
						m_register_rewrite_in_progress = false;
						/*
						if (m_rdatac)
						{
							ads1291_2_start_rdatac();
						}
						*/
#endif
						break;
				
				case BLE_BMS_EVT_SAMPLE_RATE_CHANGE:
#if (defined(ADS1291) || defined(ADS1292))
						/*
						if (m_rdatac)
						{
							ads1291_2_stop_rdatac();
						}
						*/
						m_register_rewrite_in_progress = true;
						// Read config register 1
						ads1291_2_rreg(ADS1291_2_REGADDR_CONFIG1, 1, &register_value);
						// Modify sample rate bits						
						switch (p_bms->sample_rate)
						{
							case ADS1291_2_REG_CONFIG1_125_SPS:
								register_value = (register_value & ~0x70) | (p_bms->sample_rate & 0x70); // Set bits 6:4 to chosen value
								err_code = app_timer_stop(m_sampling_timer_id);
								APP_ERROR_CHECK(err_code);
								err_code = app_timer_start(m_sampling_timer_id, APP_TIMER_TICKS(8, APP_TIMER_PRESCALER), NULL);
								APP_ERROR_CHECK(err_code);
								break;
							case ADS1291_2_REG_CONFIG1_250_SPS:
								register_value = (register_value & ~0x70) | (p_bms->sample_rate & 0x70); // Set bits 6:4 to chosen value
								err_code = app_timer_stop(m_sampling_timer_id);
								APP_ERROR_CHECK(err_code);
								err_code = app_timer_start(m_sampling_timer_id, APP_TIMER_TICKS(4, APP_TIMER_PRESCALER), NULL);
								APP_ERROR_CHECK(err_code);
								break;
							case ADS1291_2_REG_CONFIG1_500_SPS:
								register_value = (register_value & ~0x70) | (p_bms->sample_rate & 0x70); // Set bits 6:4 to chosen value
								err_code = app_timer_stop(m_sampling_timer_id);
								APP_ERROR_CHECK(err_code);
								err_code = app_timer_start(m_sampling_timer_id, APP_TIMER_TICKS(2, APP_TIMER_PRESCALER), NULL);
								APP_ERROR_CHECK(err_code);
								break;
							case ADS1291_2_REG_CONFIG1_1000_SPS:
								register_value = (register_value & ~0x70) | (p_bms->sample_rate & 0x70); // Set bits 6:4 to chosen value
								err_code = app_timer_stop(m_sampling_timer_id);
								APP_ERROR_CHECK(err_code);
								err_code = app_timer_start(m_sampling_timer_id, APP_TIMER_TICKS(1, APP_TIMER_PRESCALER), NULL);
								APP_ERROR_CHECK(err_code);
								break;
							case ADS1291_2_REG_CONFIG1_2000_SPS:
								break;
							case ADS1291_2_REG_CONFIG1_4000_SPS:
								break;
							case ADS1291_2_REG_CONFIG1_8000_SPS:
								break;
							default:
								break;
						}
						// Write modified register back to device
						ads1291_2_wreg(ADS1291_2_REGADDR_CONFIG1, 1, &register_value); 
						m_register_rewrite_in_progress = false;
						/*
						if (m_rdatac)
						{
							ads1291_2_start_rdatac();
						}
						*/
#endif
						break;
						

        default:
            // No implementation needed.
            break;
    }
}



/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    static ble_gap_evt_auth_status_t m_auth_status;
    bool                             master_id_matches;
    ble_gap_sec_kdist_t *            p_distributed_keys;
    ble_gap_enc_info_t *             p_enc_info;
    ble_gap_irk_t *                  p_id_info;
    ble_gap_sign_info_t *            p_sign_info;

    static ble_gap_enc_key_t         m_enc_key;           /**< Encryption Key (Encryption Info and Master ID). */
    static ble_gap_id_key_t          m_id_key;            /**< Identity Key (IRK and address). */
    static ble_gap_sign_info_t       m_sign_key;          /**< Signing Key (Connection Signature Resolving Key). */
    static ble_gap_sec_keyset_t      m_keys = {.keys_periph = {&m_enc_key, &m_id_key, &m_sign_key}};
		
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
				
#if ((defined(ADS1291) || defined (ADS1292)) && defined(SPI_MASTER_0_ENABLE))
						// Wake up AFE				
						ads1291_2_wake();				
#endif
						// Start sample acquisition timer	
						//err_code = app_timer_start(m_sampling_timer_id, BODY_VOLTAGE_SAMPLE_INTERVAL, NULL);
						//APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
				
						// Stop sample acquisition timer
						err_code = app_timer_stop(m_sampling_timer_id);
						APP_ERROR_CHECK(err_code);
#if ((defined(ADS1291) || defined (ADS1292)) && defined(SPI_MASTER_0_ENABLE))						
						// Put AFE back to sleep
						ads1291_2_standby();
#endif     					
            advertising_start();
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                   BLE_GAP_SEC_STATUS_SUCCESS,
                                                   &m_sec_params,
                                                   &m_keys);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle,
                                                 NULL,
                                                 0,
                                                 BLE_GATTS_SYS_ATTR_FLAG_SYS_SRVCS | BLE_GATTS_SYS_ATTR_FLAG_USR_SRVCS);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            m_auth_status = p_ble_evt->evt.gap_evt.params.auth_status;
            break;

        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            master_id_matches  = memcmp(&p_ble_evt->evt.gap_evt.params.sec_info_request.master_id,
                                        &m_enc_key.master_id,
                                        sizeof(ble_gap_master_id_t)) == 0;
            p_distributed_keys = &m_auth_status.kdist_periph;

            p_enc_info  = (p_distributed_keys->enc  && master_id_matches) ? &m_enc_key.enc_info : NULL;
            p_id_info   = (p_distributed_keys->id   && master_id_matches) ? &m_id_key.id_info   : NULL;
            p_sign_info = (p_distributed_keys->sign && master_id_matches) ? &m_sign_key         : NULL;

            err_code = sd_ble_gap_sec_info_reply(m_conn_handle, p_enc_info, p_id_info, p_sign_info);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISING)
            {
#if LEDS_NUM > 0
                err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                APP_ERROR_CHECK(err_code);
#endif
#if BUTTONS_NUM > 0
                // Configure buttons with sense level low as wakeup source.
                err_code = bsp_buttons_enable(1 << WAKEUP_BUTTON_ID);
                APP_ERROR_CHECK(err_code);
#endif
                // Go to system-off mode (this function will not return; wakeup will cause a reset)                
                err_code = sd_power_system_off();
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
		dm_ble_evt_handler(p_ble_evt);
		on_ble_evt(p_ble_evt);
		ble_conn_params_on_ble_evt(p_ble_evt);
    ble_bms_on_ble_evt(&m_bms, p_ble_evt);
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
#ifdef BLE_DFU_APP_SUPPORT    
    /** @snippet [Propagating BLE Stack events to DFU Service] */
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
    /** @snippet [Propagating BLE Stack events to DFU Service] */
#endif // BLE_DFU_APP_SUPPORT    
}

/**@brief Function for handling the Application's system events.
 *
 * @param[in]   sys_evt   system event.
 */
static void on_sys_evt(uint32_t sys_evt)
{
    switch(sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
        case NRF_EVT_FLASH_OPERATION_ERROR:

            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                advertising_start();
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    on_sys_evt(sys_evt);
}



/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
		uint32_t       err_code;
    ble_bms_init_t bms_init;
    ble_bas_init_t bas_init;
    ble_dis_init_t dis_init;
    uint8_t        body_sensor_location;
		uint8_t				 gain;
		uint8_t				 sample_rate;

    // Initialize Biopotential Measurement Service.
    body_sensor_location = BLE_BMS_BODY_SENSOR_LOCATION_OTHER;
#if defined(ADS1291) || defined(ADS1292)	
		gain 								 = 12;
		sample_rate 				 = ADS1291_2_REG_CONFIG1_125_SPS;
#else
		gain								 = 12;
		sample_rate					 = 150;
#endif

    memset(&bms_init, 0, sizeof(bms_init));

    bms_init.evt_handler                 = on_bms_evt;
    bms_init.p_body_sensor_location      = &body_sensor_location;
		bms_init.p_gain											 = &gain;
		bms_init.p_sample_rate							 = &sample_rate;

    // Here the sec level for the Biopotential Measurement Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bms_init.bms_bvm_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bms_init.bms_bvm_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bms_init.bms_bvm_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bms_init.bms_bsl_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bms_init.bms_bsl_attr_md.write_perm);
		
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bms_init.bms_gain_attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bms_init.bms_gain_attr_md.write_perm);
		
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bms_init.bms_sample_rate_attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bms_init.bms_sample_rate_attr_md.write_perm);

    err_code = ble_bms_init(&m_bms, &bms_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));
		
		bas_init.evt_handler          = on_bas_evt;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
    
#ifdef BLE_DFU_APP_SUPPORT    
    /** @snippet [DFU BLE Service initialization] */
    ble_dfu_init_t   dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler    = dfu_app_on_dfu_evt;
    dfus_init.error_handler  = NULL; //service_error_handler - Not used as only the switch from app to DFU mode is required and not full dfu service.
    dfus_init.evt_handler    = dfu_app_on_dfu_evt;
    dfus_init.revision       = DFU_REVISION;

    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);
    
    dfu_app_reset_prepare_set(reset_prepare);
    /** @snippet [DFU BLE Service initialization] */
#endif // BLE_DFU_APP_SUPPORT    
}


/**@brief Function for initializing security parameters.
 */
static void sec_params_init(void)
{
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}

#ifndef SPI_MASTER_0_ENABLE
/**@brief Function for initializing the sensor simulators.
 */
static void sensor_sim_init(void)
{
    m_body_voltage_sim_cfg.min          = MIN_BODY_VOLTAGE;
    m_body_voltage_sim_cfg.max          = MAX_BODY_VOLTAGE;
    m_body_voltage_sim_cfg.incr         = BODY_VOLTAGE_INCREMENT;
    m_body_voltage_sim_cfg.start_at_max = false;

    sensorsim_init(&m_body_voltage_sim_state, &m_body_voltage_sim_cfg);
}
#endif

/**@brief Function for starting timers.
*/
static void timers_start(void)
{
    uint32_t err_code;
	
		err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);	
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = m_bms.bvm_handles.cccd_handle;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_TEMP_4000MS_CALIBRATION, false);

#ifdef S110
    // Enable BLE stack 
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
#endif
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for handling a bsp event.
 *
 * @param[in]     evt                        BSP event.
 */
/* YOUR_JOB: Uncomment this function if you need to handle button events.
static void bsp_event_handler(bsp_event_t evt)
{
        switch (evt)
        {
            case BSP_EVENT_KEY_0:
                // Code to handle BSP_EVENT_KEY_0
                break;

            // Handle any other event

            default:
                APP_ERROR_HANDLER(evt);
                break;
        }
    }
}
*/


/**@brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{	
		uint32_t err_code;
	
		APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
		err_code = app_gpiote_user_register(&m_sampling_gpiote_id, 0x00000000, (1 << ADS1291_2_DRDY_PIN), drdy_event_handler);
		APP_ERROR_CHECK(err_code);		
}

#if BUTTONS_NUM > 0
/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{   
        uint32_t err_code;
        // Note: Before start using buttons, assign events to buttons, as shown below.
        //      err_code = bsp_event_to_button_assign(BUTTON_0_ID, BSP_EVENT_KEY_0);
        //      APP_ERROR_CHECK(err_code);
        // Note: Enable buttons which you want to use.
        //      err_code = bsp_buttons_enable((1 << WAKEUP_BUTTON_ID) | (1 << BUTTON_0_ID)); 
        //      APP_ERROR_CHECK(err_code);
        // Note: If the only use of buttons is to wake up, the bsp module can be omitted, and
        // the wakeup button can be configured by
        err_code = bsp_buttons_enable(1 << WAKEUP_BUTTON_ID);
        APP_ERROR_CHECK(err_code);
}
#endif

/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    APP_ERROR_CHECK(event_result);

#ifdef BLE_DFU_APP_SUPPORT
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        app_context_load(p_handle);
    }
#endif // BLE_DFU_APP_SUPPORT

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 */
static void device_manager_init(void)
{
    uint32_t               err_code;
    dm_init_param_t        init_data;
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    // Clear all bonded centrals if the Bonds Delete button is pushed.
    //err_code = bsp_button_is_pressed(BOND_DELETE_ALL_BUTTON_ID,&(init_data.clear_persistent_data));
    //APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_data);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

#ifdef SPI_MASTER_0_ENABLE

/**@brief The function initializes TX buffer to values to be sent and clears RX buffer.
 *
 * @note Function clears RX and TX buffers.
 *
 * @param[out] p_tx_data    A pointer to a buffer TX.
 * @param[out] p_rx_data    A pointer to a buffer RX.
 * @param[in] len           A length of the data buffers.
 */
static void init_buf(uint8_t * const p_tx_buf,
                     uint8_t * const p_rx_buf,
                     const uint16_t  len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        p_tx_buf[i] = 0;
        p_rx_buf[i] = 0;
    }
}

/**@brief Handler for SPI0 master events.
 *
 * @param[in] spi_master_evt    SPI master event.
 */
void spi_master_0_event_handler(spi_master_evt_t spi_master_evt)
{
    //uint32_t err_code = NRF_SUCCESS;

    switch (spi_master_evt.evt_type)
    {
        case SPI_MASTER_EVT_TRANSFER_COMPLETED:

            //err_code = bsp_indication_set(BSP_INDICATE_RCV_OK);
            //APP_ERROR_CHECK(err_code);

            //m_transfer_completed = true;
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing a SPI master driver.
 *
 * @param[in] spi_master_instance       An instance of SPI master module.
 * @param[in] spi_master_event_handler  An event handler for SPI master events.
 * @param[in] lsb                       Bits order LSB if true, MSB if false.
 */
static void spi_master_init(spi_master_hw_instance_t   spi_master_instance,
                            spi_master_event_handler_t spi_master_event_handler,
                            const bool                 lsb)
{
    uint32_t err_code = NRF_SUCCESS;

    // Configure SPI master.
    spi_master_config_t spi_config = SPI_MASTER_INIT_DEFAULT;
	
		spi_config.SPI_CONFIG_CPHA = SPI_CONFIG_CPHA_Trailing;
		//spi_config.SPI_PriorityIRQ = APP_IRQ_PRIORITY_LOW;
	
    switch (spi_master_instance)
    {
				case SPI_MASTER_0:
        {
            spi_config.SPI_Pin_SCK  = SPIM0_SCK_PIN;
            spi_config.SPI_Pin_MISO = SPIM0_MISO_PIN;
            spi_config.SPI_Pin_MOSI = SPIM0_MOSI_PIN;
            spi_config.SPI_Pin_SS   = SPIM0_SS_PIN;
        }
        break;

        default:
            break;
    }

    spi_config.SPI_CONFIG_ORDER = (lsb ? SPI_CONFIG_ORDER_LsbFirst : SPI_CONFIG_ORDER_MsbFirst);

    err_code = spi_master_open(spi_master_instance, &spi_config);
    APP_ERROR_CHECK(err_code);

    // Register event handler for SPI master.
    spi_master_evt_handler_reg(spi_master_instance, spi_master_event_handler);
}

#endif /* SPI_MASTER_0_ENABLE */

/**@brief Function for initializing bsp module.
 */
static void bsp_module_init(void)
{
		uint32_t err_code;

		// Note: If the only use of buttons is to wake up, bsp_event_handler can be NULL.
		err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
		APP_ERROR_CHECK(err_code);
		// Note: If the buttons will be used to do some task, assign bsp_event_handler, as shown below.
		// err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), bsp_event_handler);
		// APP_ERROR_CHECK(err_code);
		// Buttons initialization.
#if BUTTONS_NUM > 0
		buttons_init();
#endif
}

/**@brief Function for application main entry.
 */
int main(void)
{	
		//uint8_t regs[ADS1291_2_NUM_REGS];
		uint32_t err_code;
	
    // Initialize
		ble_stack_init();
    timers_init();
    gpiote_init();
    bsp_module_init();
		device_manager_init();
    scheduler_init();
    gap_params_init();
    advertising_init();
    services_init();
#ifndef SPI_MASTER_0_ENABLE
		sensor_sim_init();
#endif
		adc_configure();
    conn_params_init();
    sec_params_init();

// AFE initialization	
#if defined(ADS1291) || defined(ADS1292)
		nrf_gpio_pin_dir_set(ADS1291_2_DRDY_PIN, NRF_GPIO_PIN_DIR_INPUT);
		nrf_gpio_pin_dir_set(ADS1291_2_PWDN_PIN, NRF_GPIO_PIN_DIR_OUTPUT);
		ads1291_2_powerdn();
		ads1291_2_powerup();

		spi_master_init(SPI_MASTER_0, spi_master_0_event_handler, false);
		init_buf(m_tx_data_spi, m_rx_data_spi, TX_RX_MSG_LENGTH);
		
		// Stop continuous data conversion and initialize registers to default values
		ads1291_2_stop_rdatac();
		ads1291_2_init_regs();
		ads1291_2_soft_start_conversion();
		ads1291_2_start_rdatac();
		
		// Put AFE to sleep while we're not connected
		ads1291_2_standby();
		
		// Enable DRDY event handler
		err_code = app_gpiote_user_enable(m_sampling_gpiote_id);
		APP_ERROR_CHECK(err_code);
#endif // ADS1291 or ADS1292
	
    // Start execution
    timers_start();
    advertising_start();

    // Enter main loop
    for (;;)
    {	
				if (m_drdy)
				{
					m_drdy = false;
					get_bvm_sample_rdatac();
				}
        app_sched_execute();
        power_manage();
    }
}

/**
 * @}
 */
