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

#ifdef __cplusplus
extern "C" {
#endif	

#ifdef SPI_MASTER_0_ENABLE
	
#include "ads1291-2.h"
#include "app_error.h"
#include "spi_master.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

uint8_t ads1291_2_default_regs[] = 
{
	ADS1291_2_REGDEFAULT_CONFIG1,
	ADS1291_2_REGDEFAULT_CONFIG2,
	ADS1291_2_REGDEFAULT_LOFF,
	ADS1291_2_REGDEFAULT_CH1SET,
	ADS1291_2_REGDEFAULT_CH2SET,
	ADS1291_2_REGDEFAULT_RLD_SENS,
	ADS1291_2_REGDEFAULT_LOFF_SENS,
	ADS1291_2_REGDEFAULT_LOFF_STAT,
	ADS1291_2_REGDEFAULT_RESP1,
	ADS1291_2_REGDEFAULT_RESP2,
	ADS1291_2_REGDEFAULT_GPIO 
};


/**************************************************************************************************************************************************
 *               Function Definitions                                                                                                              *
 **************************************************************************************************************************************************/

/* REGISTER READ/WRITE FUNCTIONS *****************************************************************************************************************/

uint32_t ads1291_2_rreg(uint8_t reg_addr, uint8_t num_to_read, uint8_t* read_reg_val_ptr)
{
	uint32_t err_code;
	uint32_t i;
	uint8_t tx_data_spi[2+num_to_read];
	
	tx_data_spi[0] = ADS1291_2_OPC_RREG | reg_addr;
	tx_data_spi[1] = num_to_read - 1;
	for (i = 2; i < 2+num_to_read; i++)
	{
		tx_data_spi[i] = 0;
	}		
	err_code = spi_master_send_recv(SPI_MASTER_0, tx_data_spi, 2+num_to_read, read_reg_val_ptr, 2+num_to_read);	
	return err_code;
}

uint32_t ads1291_2_wreg(uint8_t reg_addr, uint8_t num_to_write, uint8_t* write_reg_val_ptr)
{
	uint32_t err_code;
	uint32_t i;
	uint8_t tx_data_spi[ADS1291_2_NUM_REGS+2];
	uint8_t rx_data_spi[ADS1291_2_NUM_REGS+2];
	
	if (num_to_write < ADS1291_2_NUM_REGS)
	{
		for (i = 0; i < ADS1291_2_NUM_REGS+2; i++)
		{
			tx_data_spi[i] = 0;
		}
	}
	
	tx_data_spi[0] = ADS1291_2_OPC_WREG | reg_addr;
	tx_data_spi[1] = num_to_write - 1;
	for (i = 0; i < num_to_write; i++)
	{
		tx_data_spi[i+2] = write_reg_val_ptr[i];
	}
		
	err_code = spi_master_send_recv(SPI_MASTER_0, 
																	tx_data_spi, 
																	2+num_to_write,
																	rx_data_spi,
																	2+num_to_write); 	
	return err_code;
}

/* SYSTEM CONTROL FUNCTIONS **********************************************************************************************************************/

uint32_t ads1291_2_init_regs(void)
{
	return ads1291_2_wreg(ADS1291_2_REGADDR_CONFIG1, ADS1291_2_NUM_REGS-1, ads1291_2_default_regs);
}

uint32_t ads1291_2_standby(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	tx_data_spi = ADS1291_2_OPC_STANDBY;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	return err_code;
}

uint32_t ads1291_2_wake(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	tx_data_spi = ADS1291_2_OPC_WAKEUP;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	nrf_delay_ms(10);	// Allow time to wake up
	return err_code;
}

uint32_t ads1291_2_soft_start_conversion(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	tx_data_spi = ADS1291_2_OPC_START;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	return err_code;
}

uint32_t ads1291_2_soft_stop_conversion(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	tx_data_spi = ADS1291_2_OPC_STOP;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	
	return err_code;
}

uint32_t ads1291_2_start_rdatac(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	tx_data_spi = ADS1291_2_OPC_RDATAC;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	
	return err_code;
}

uint32_t ads1291_2_stop_rdatac(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	tx_data_spi = ADS1291_2_OPC_SDATAC;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	
	return err_code;
}

void ads1291_2_calibrate(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	/*
	uint8_t resp2_regval_old, resp2_regval_new;
	
	// Enable offset calibration by setting bit 7 in RESP2 register
	err_code = ads1291_2_rreg(ADS1291_2_REGADDR_RESP2, 1, &resp2_regval_old);
	APP_ERROR_CHECK(err_code);
	resp2_regval_new = resp2_regval_old | 0x80;
	err_code = ads1291_2_wreg(ADS1291_2_REGADDR_RESP2, 1, &resp2_regval_new);
	APP_ERROR_CHECK(err_code);
	*/
	
	tx_data_spi = ADS1291_2_OPC_OFFSETCAL;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(32);	// Delay to allow calibration to complete

	/*
	// Disable offset calibration again (necessary?)
	err_code = ads1291_2_wreg(ADS1291_2_REGADDR_RESP2, 1, &resp2_regval_old);
	APP_ERROR_CHECK(err_code);
	*/
}

void ads1291_2_powerdn(void)
{
	nrf_gpio_pin_clear(ADS1291_2_PWDN_PIN);
}

void ads1291_2_powerup(void)
{
	nrf_gpio_pin_set(ADS1291_2_PWDN_PIN);
	nrf_delay_ms(1000);		// Allow time for power-on reset
}

uint32_t ads1291_2_soft_reset(void)
{
	uint32_t err_code;
	uint8_t tx_data_spi;
	uint8_t rx_data_spi;
	
	tx_data_spi = ADS1291_2_OPC_RESET;
	err_code = spi_master_send_recv(SPI_MASTER_0, &tx_data_spi, 1, &rx_data_spi, 1);
	
	return err_code;
}

#ifdef ADS1291_2_START_PIN
void ads1291_2_hard_start_conversion(void)
{
	nrf_gpio_pin_set(ADS1291_2_START_PIN);
}

void ads1291_2_hard_stop_conversion(void)
{
	nrf_gpio_pin_clear(ADS1291_2_START_PIN);
} 
#endif // ADS1291_2_START_PIN

uint8_t ads1291_2_check_id(void)
{
		uint32_t err_code;
		uint8_t id_reg_val;
		uint8_t device_id;
	
		#if defined(ADS1291)
		device_id = ADS1291_DEVICE_ID;
		#elif defined(ADS1292)
		device_id = ADS1292_DEVICE_ID;
		#endif		
	
		err_code = ads1291_2_rreg(ADS1291_2_REGADDR_ID, 1, &id_reg_val);
		APP_ERROR_CHECK(err_code);

		if (id_reg_val == device_id)
		{
			return 1;
		}
		else
		{
			return 0;
		}		
}

/* DATA RETRIEVAL FUNCTIONS **********************************************************************************************************************/

#endif // SPI_MASTER_0_ENABLE

#ifdef __cplusplus
}
#endif
