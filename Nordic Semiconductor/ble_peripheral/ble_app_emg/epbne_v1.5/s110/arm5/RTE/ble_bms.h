/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ble_sdk_srv_bms Biopotential Measurement Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Biopotential Measurement Service module.
 *
 * @details This module implements the Biopotential Measurement Service with the Voltage characteristic.
 *          During initialization it adds the Biopotential Measurement Service and Voltage characteristic
 *          to the BLE stack dataBISe. Optionally it can also add a Report Reference descriptor
 *          to the Battery Level characteristic (used when including the Biopotential Measurement Service in
 *          the HID service).
 *
 *          If specified, the module will support notification of the Battery Level characteristic
 *          through the ble_bms_battery_level_update() function.
 *          If an event handler is supplied by the application, the Biopotential Measurement Service will
 *          generate Biopotential Measurement Service events to the application.
 *
 * @note The application must propagate BLE stack events to the Biopotential Measurement Service module by calling
 *       ble_bms_on_ble_evt() from the from the @ref ble_stack_handler callback.
 */

#ifndef BLE_BMS_H__
#define BLE_BMS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

// Base UUID
#define BMS_UUID_BASE {0x57, 0x80, 0xD2, 0x94, 0xA3, 0xB2, 0xFE, 0x39, 0x5F, 0x87, 0xFD, 0x35, 0x00, 0x00, 0x8B, 0x22};

// Service UUID
#define BLE_UUID_BIOPOTENTIAL_MEASUREMENT_SERVICE	0x3260

// Characteristic UUIDs
#define BLE_UUID_BODY_VOLTAGE_MEASUREMENT_CHAR		0x3261
#define BLE_UUID_GAIN_CHAR												0x3262
#define BLE_UUID_SAMPLE_RATE_CHAR									0x3263

// Body Sensor Location values
#define BLE_BMS_BODY_SENSOR_LOCATION_OTHER      	0
#define BLE_BMS_BODY_SENSOR_LOCATION_CHEST      	1
#define BLE_BMS_BODY_SENSOR_LOCATION_WRIST      	2
#define BLE_BMS_BODY_SENSOR_LOCATION_FINGER     	3
#define BLE_BMS_BODY_SENSOR_LOCATION_HAND       	4
#define BLE_BMS_BODY_SENSOR_LOCATION_EAR_LOBE   	5
#define BLE_BMS_BODY_SENSOR_LOCATION_FOOT       	6

// Maximum number of body voltage measurement bytes buffered by the application
#define BLE_BMS_MAX_BUFFERED_MEASUREMENTS					30

/**@brief Biopotential Measurement Service event type. */
typedef enum
{
    BLE_BMS_EVT_NOTIFICATION_ENABLED,                   /**< Voltage value notification enabled event. */
    BLE_BMS_EVT_NOTIFICATION_DISABLED,                   /**< Voltage value notification disabled event. */
		BLE_BMS_EVT_GAIN_CHANGE,
		BLE_BMS_EVT_SAMPLE_RATE_CHANGE
} ble_bms_evt_type_t;

/**@brief Biopotential Measurement Service event. */
typedef struct
{
    ble_bms_evt_type_t evt_type;                        /**< Type of event. */
} ble_bms_evt_t;

// Forward declaration of the ble_bms_t type. 
typedef struct ble_bms_s ble_bms_t;

/**@brief Biopotential Measurement Service event handler type. */
typedef void (*ble_bms_evt_handler_t) (ble_bms_t * p_hrs, ble_bms_evt_t * p_evt);

/**@brief Biopotential Measurement Service init structure. This contains all options and data needed for
 *        initialization of the service. */
typedef struct
{
    ble_bms_evt_handler_t        evt_handler;                                          /**< Event handler to be called for handling events in the Biopotential Measurement Service. */
    uint8_t *										 p_gain;																							 /**< If not NULL, initial value of the Gain characteristic. */
		uint8_t *										 p_sample_rate;																				 /**< If not NULL, initial value of the Sample Rate characteristic. */
		uint8_t *                    p_body_sensor_location;                               /**< If not NULL, initial value of the Body Sensor Location characteristic. */
		ble_srv_cccd_security_mode_t bms_bvm_attr_md;                                      /**< Initial security level for Body Voltage Measurement attribute */
    ble_srv_security_mode_t			 bms_gain_attr_md;																		 /**< Initial security level for Gain attribute */
		ble_srv_security_mode_t			 bms_sample_rate_attr_md;															 /**< Initial security level for Sample Rate attribute */
		ble_srv_security_mode_t      bms_bsl_attr_md;                                      /**< Initial security level for Body Sensor Location attribute */
} ble_bms_init_t;


/**@brief Biopotential Measurement Service structure. This contains various status information for the service. */
typedef struct ble_bms_s
{  
    ble_bms_evt_handler_t        evt_handler;                                          /**< Event handler to be called for handling events in the Biopotential Measurement Service. */	
    uint16_t                     service_handle;                                       /**< Handle of Biopotential Measurement Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t     bvm_handles;                                          /**< Handles related to the Body Voltage Measurement characteristic. */
    ble_gatts_char_handles_t		 gain_handles;																				 /**< Handles related to the Gain characteristic. */
		ble_gatts_char_handles_t		 sample_rate_handles;																 	 /**< Handles related to the Sample Rate characteristic. */    
		ble_gatts_char_handles_t     bsl_handles;                                          /**< Handles related to the Body Sensor Location characteristic. */
		uint8_t                      uuid_type;             
    uint16_t                     conn_handle;  
		uint16_t										 bvm_buffer[BLE_BMS_MAX_BUFFERED_MEASUREMENTS];
		uint8_t											 bvm_count;	
		uint8_t 	 									 gain;
		uint8_t 										 sample_rate;
} ble_bms_t;

/**@brief Initialize the Biopotential Measurement Service.
 *
 * @param[out]  p_bms       
 * @param[in]   p_bms_init  
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_bms_init(ble_bms_t * p_bms, const ble_bms_init_t * p_bms_init);

/**@brief Biopotential Measurement Service BLE stack event handler.
 *
 * @details Handles all events from the BLE stack of interest to the Biopotential Measurement Service.
 *
 * @param[in]   p_bms      Biopotential Measurement Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_bms_on_ble_evt(ble_bms_t * p_bms, ble_evt_t * p_ble_evt);

/**@brief Function for sending body voltage measurement if notification has been enabled.
 *
 * @details The application calls this function after having performed a voltage measurement.
 *          If notification has been enabled, the voltage measurement data is encoded and sent to
 *          the client.
 *
 * @param[in]   p_bms                    Biopotential Measurement Service structure.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_bms_body_voltage_measurement_send(ble_bms_t * p_bms);

/**@brief Function for adding a Body Voltage Measurement to the buffer.
 *
 * @details All buffered voltage measurements will be included in the next biopotential
 *          measurement message, up to the maximum number of measurements that will fit into the
 *          message. If the buffer is full, the oldest measurement in the buffer will be deleted.
 *
 * @param[in]   p_bms        Biopotential Measurement Service structure.
 * @param[in]   bvm_val 	   New voltage measurement (will be buffered until the next
 *                           connection interval).
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_bms_bvm_add(ble_bms_t * p_bms, int16_t bvm_val);

/**@brief Function for checking if Body Voltage Measurement buffer is full.
 *
 * @param[in]   p_bms        Biopotential Measurement Service structure.
 *
 * @return      true if Body Voltage Measurement buffer is full, false otherwise.
 */
bool ble_bms_bvm_buffer_is_full(ble_bms_t * p_bms);


/**@brief Function for setting the Body Sensor Location.
 *
 * @details Sets a new value of the Body Sensor Location characteristic. The new value will be sent
 *          to the client the next time the client reads the Body Sensor Location characteristic.
 *
 * @param[in]   p_bms                 Biopotential Measurement Service structure.
 * @param[in]   body_sensor_location  New Body Sensor Location.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_bms_body_sensor_location_set(ble_bms_t * p_bms, uint8_t body_sensor_location);

#endif // BLE_BMS_H__

/** @} */
