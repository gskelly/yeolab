/* Copyright (c) 2015 Graham Kelly
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ble_bms.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "ads1291-2.h"

#define MAX_BVM_LEN   		60																								 /**< Maximum size in bytes of a transmitted Body Voltage Measurement. */
#define INITIAL_VALUE_BVM	0																									 /**< Initial value of Body Voltage Measurement. */

/**@brief Connect event handler.
 *
 * @param[in]   p_bms       Biopotential Measurement Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_bms_t * p_bms, ble_evt_t * p_ble_evt)
{
    p_bms->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Disconnect event handler.
 *
 * @param[in]   p_bms       Biopotential Measurement Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_bms_t * p_bms, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_bms->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling write events to the Body Voltage Measurement CCCD.
 *
 * @param[in]   p_bms         Biopotential Measurement Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */

static void on_bvm_cccd_write(ble_bms_t * p_bms, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update notification state
        if (p_bms->evt_handler != NULL)
        {
            ble_bms_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_BMS_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_BMS_EVT_NOTIFICATION_DISABLED;
            }

            p_bms->evt_handler(p_bms, &evt);
        }
    }
}

/**@brief Function for handling write events to the Gain characteristic.
 *
 * @param[in]   p_bms         Biopotential Measurement Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */

static void on_gain_write(ble_bms_t * p_bms, ble_gatts_evt_write_t * p_evt_write)
{   
		// Pass event to main application for implementation-specific handling
		if (p_bms->evt_handler != NULL)
		{
				ble_bms_evt_t evt;

				evt.evt_type = BLE_BMS_EVT_GAIN_CHANGE;
				p_bms->evt_handler(p_bms, &evt);
		}
}


/**@brief Function for handling write events to the Sample Rate characteristic.
 *
 * @param[in]   p_bms         Biopotential Measurement Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */

static void on_sample_rate_write(ble_bms_t * p_bms, ble_gatts_evt_write_t * p_evt_write)
{
 		// Pass event to main application for implementation-specific handling
		if (p_bms->evt_handler != NULL)
		{
				ble_bms_evt_t evt;

				evt.evt_type = BLE_BMS_EVT_SAMPLE_RATE_CHANGE;
				p_bms->evt_handler(p_bms, &evt);
		}
}

/**@brief Write event handler.
 *
 * @param[in]   p_bms       Biopotential Measurement Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_bms_t * p_bms, ble_evt_t * p_ble_evt)
{
		uint8_t* p_data = p_ble_evt->evt.gatts_evt.params.write.data;
		//uint16_t length = p_ble_evt->evt.gatts_evt.params.write.len;
		ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

		if (p_evt_write->handle == p_bms->bvm_handles.cccd_handle)
		{
			 on_bvm_cccd_write(p_bms, p_evt_write);
		}
		else if (p_evt_write->handle == p_bms->gain_handles.value_handle)
		{
			 p_bms->gain = *p_data;
			 on_gain_write(p_bms, p_evt_write);
		}
		else if (p_evt_write->handle == p_bms->sample_rate_handles.value_handle)
		{
			 p_bms->sample_rate = *p_data;
			 on_sample_rate_write(p_bms, p_evt_write);
		}	 
}



void ble_bms_on_ble_evt(ble_bms_t * p_bms, ble_evt_t * p_ble_evt)
{

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_bms, p_ble_evt);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_bms, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_bms, p_ble_evt);
            break;
            
        default:
            break;
    }
}

/**@brief Function for encoding int16 Body Voltage Measurement buffer to a byte array.
 *
 * @param[in]   p_bms              Biopotential Measurement Service structure.
 * @param[in]   body_voltage       Measurement to be encoded.
 * @param[out]  p_encoded_buffer   Buffer where the encoded data will be written.
 *
 * @return      Size of encoded data.
 */
static uint8_t bvm_encode(ble_bms_t * p_bms, uint8_t * p_encoded_buffer)
{
    uint8_t len   = 0;
    int     i;

    // Encode body voltage measurement
    for (i = 0; i < p_bms->bvm_count; i++)
    {			
        if (len + sizeof(uint16_t) > MAX_BVM_LEN)
        {
            // Not all stored voltage values can fit into the packet, so
            // move the remaining values to the start of the buffer.
            memmove(&p_bms->bvm_buffer[0],
                    &p_bms->bvm_buffer[i],
                    (p_bms->bvm_count - i) * sizeof(uint16_t));
            break;
        }
        len += uint16_encode(p_bms->bvm_buffer[i], &p_encoded_buffer[len]);
    }
    p_bms->bvm_count -= i;
		

    return len;
}

/**@brief Function for adding the Body Voltage Measurement characteristic.
 *
 * @param[in]   p_bms        Biopotential Measurement Service structure.
 * @param[in]   p_bms_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t body_voltage_measurement_char_add(ble_bms_t            * p_bms,
                                                  const ble_bms_init_t * p_bms_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             encoded_initial_bvm[MAX_BVM_LEN];

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    cccd_md.write_perm = p_bms_init->bms_bvm_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
		char_md.char_props.read  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BODY_VOLTAGE_MEASUREMENT_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_bms_init->bms_bvm_attr_md.read_perm;
    attr_md.write_perm = p_bms_init->bms_bvm_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = bvm_encode(p_bms, encoded_initial_bvm);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_BVM_LEN;
    attr_char_value.p_value   = encoded_initial_bvm;

    return sd_ble_gatts_characteristic_add(p_bms->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_bms->bvm_handles);
}

/**@brief Function for adding the Gain characteristic.
 *
 * @param[in]   p_bms        Biopotential Measurement Service structure.
 * @param[in]   p_bms_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t gain_char_add(ble_bms_t            * p_bms,
                              const ble_bms_init_t * p_bms_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = 1;
		char_md.char_props.write = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_GAIN_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_bms_init->bms_gain_attr_md.read_perm;
    attr_md.write_perm = p_bms_init->bms_gain_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof (uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof (uint8_t);
    attr_char_value.p_value   = p_bms_init->p_gain;

    return sd_ble_gatts_characteristic_add(p_bms->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_bms->gain_handles);
}

/**@brief Function for adding the Sample Rate characteristic.
 *
 * @param[in]   p_bms        Biopotential Measurement Service structure.
 * @param[in]   p_bms_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t sample_rate_char_add(ble_bms_t            * p_bms,
																		 const ble_bms_init_t * p_bms_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = 1;
		char_md.char_props.write = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_SAMPLE_RATE_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_bms_init->bms_sample_rate_attr_md.read_perm;
    attr_md.write_perm = p_bms_init->bms_sample_rate_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof (uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof (uint8_t);
    attr_char_value.p_value   = p_bms_init->p_sample_rate;

    return sd_ble_gatts_characteristic_add(p_bms->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_bms->sample_rate_handles);
}

/**@brief Function for adding the Body Sensor Location characteristic.
 *
 * @param[in]   p_bms        Biopotential Measurement Service structure.
 * @param[in]   p_bms_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t body_sensor_location_char_add(ble_bms_t * p_bms, const ble_bms_init_t * p_bms_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BODY_SENSOR_LOCATION_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_bms_init->bms_bsl_attr_md.read_perm;
    attr_md.write_perm = p_bms_init->bms_bsl_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof (uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof (uint8_t);
    attr_char_value.p_value   = p_bms_init->p_body_sensor_location;

    return sd_ble_gatts_characteristic_add(p_bms->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_bms->bsl_handles);
}

uint32_t ble_bms_init(ble_bms_t * p_bms, const ble_bms_init_t * p_bms_init)
{
    uint32_t   		err_code;
    ble_uuid_t 		ble_uuid;
		ble_uuid128_t	base_uuid = BMS_UUID_BASE;

    // Initialize service structure
    p_bms->evt_handler                 = p_bms_init->evt_handler;
    p_bms->conn_handle                 = BLE_CONN_HANDLE_INVALID;
    p_bms->bvm_count           				 = 0;

		// Add custom base UUID
		err_code = sd_ble_uuid_vs_add(&base_uuid, &p_bms->uuid_type);
		if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
    // Add service
		ble_uuid.type = p_bms->uuid_type;
		ble_uuid.uuid = BLE_UUID_BIOPOTENTIAL_MEASUREMENT_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_bms->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add body voltage measurement characteristic
    err_code = body_voltage_measurement_char_add(p_bms, p_bms_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
		
		if (p_bms_init->p_gain != NULL)
		{
			// Add gain characteristic
			err_code = gain_char_add(p_bms, p_bms_init);
			if (err_code != NRF_SUCCESS)
			{
					return err_code;
			}
		}
		
		if (p_bms_init->p_sample_rate != NULL)
		{
			// Add sample rate characteristic
			err_code = sample_rate_char_add(p_bms, p_bms_init);
			if (err_code != NRF_SUCCESS)
			{
					return err_code;
			}
		}

    if (p_bms_init->p_body_sensor_location != NULL)
    {
        // Add body sensor location characteristic
        err_code = body_sensor_location_char_add(p_bms, p_bms_init);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }

    return NRF_SUCCESS;
}

uint32_t ble_bms_body_voltage_measurement_send(ble_bms_t * p_bms)
{
    uint32_t err_code;

    // Send value if connected and notifying
    if (p_bms->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint8_t                encoded_bvm[MAX_BVM_LEN];
        uint16_t               len;
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params;

        len     = bvm_encode(p_bms, encoded_bvm);
        hvx_len = len;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_bms->bvm_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &hvx_len;
        hvx_params.p_data = encoded_bvm;

        err_code = sd_ble_gatts_hvx(p_bms->conn_handle, &hvx_params);
        if ((err_code == NRF_SUCCESS) && (hvx_len != len))
        {
            err_code = NRF_ERROR_DATA_SIZE;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t ble_bms_body_sensor_location_set(ble_bms_t * p_bms, uint8_t body_sensor_location)
{
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &body_sensor_location;

    return sd_ble_gatts_value_set(p_bms->conn_handle, p_bms->bsl_handles.value_handle, &gatts_value);
}

uint32_t ble_bms_gain_set(ble_bms_t * p_bms, uint8_t gain)
{
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &gain;

    return sd_ble_gatts_value_set(p_bms->conn_handle, p_bms->gain_handles.value_handle, &gatts_value);
}

uint32_t ble_bms_sample_rate_set(ble_bms_t * p_bms, uint8_t sample_rate)
{
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &sample_rate;

    return sd_ble_gatts_value_set(p_bms->conn_handle, p_bms->sample_rate_handles.value_handle, &gatts_value);
}

uint32_t ble_bms_bvm_add(ble_bms_t * p_bms, int16_t body_voltage)
{
			uint32_t err_code = NRF_SUCCESS;
			ble_gatts_value_t gatts_value;
	
			// Initialize value struct.
			memset(&gatts_value, 0, sizeof(gatts_value));

			gatts_value.len     = sizeof(uint16_t);
			gatts_value.offset  = 0;
			gatts_value.p_value = (uint8_t *)&body_voltage;
	
    if (p_bms->bvm_count == BLE_BMS_MAX_BUFFERED_MEASUREMENTS)
    {
        // The voltage measurement buffer is full, delete the oldest value
        memmove(&p_bms->bvm_buffer[0],
                &p_bms->bvm_buffer[1],
                (BLE_BMS_MAX_BUFFERED_MEASUREMENTS - 1) * sizeof(uint16_t));
        p_bms->bvm_count--;
    }

    // Add new value
    p_bms->bvm_buffer[p_bms->bvm_count++] = body_voltage;
		
		// Update database.
		err_code = sd_ble_gatts_value_set(p_bms->conn_handle,
																			p_bms->bvm_handles.value_handle,
																			&gatts_value);

		return err_code;
}


bool ble_bms_bvm_buffer_is_full(ble_bms_t * p_bms)
{
    return (p_bms->bvm_count == BLE_BMS_MAX_BUFFERED_MEASUREMENTS);
}

