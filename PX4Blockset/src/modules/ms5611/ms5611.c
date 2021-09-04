#include "ms5611.h"

static uint32_t _module_state = DISABLE;
static uint32_t _convert_D1 = DISABLE;
static uint32_t _D1 = 0U;
static uint32_t _D2 = 0U;
static uint32_t _converting_start = 0U;

static uint16_t _C[7];

static ms5611_data_st _ms5611_data_storage[2];
static uint32_t storageIdx = 0;

void _send_cmd(uint8_t cmd);
void _calc_values();
uint32_t _get_meas_value();

void px4_ms5611_update()
{
	if (_module_state == DISABLE)
	{
		return;
	}

	// still converting value
	if (toc(_converting_start) < 10000U)
	{
		return; // nothing to do
	}

	// set spi speed (~20MHZ)
	px4_spi_drv_set_clock_speed(PX4_SPI1, SPI_DRV_PRESCALER_4);
	
	if (_convert_D1 == ENABLE) //	D1 conversion is over => switch
	{
		_D1 = _get_meas_value();
		if (_D1 != 0)
		{
			_convert_D1 = DISABLE;
		}
	}
	else	// D2 conversion is over => switch
	{
		_D2 = _get_meas_value();
		if (_D2 != 0)
		{
			_convert_D1 = ENABLE;
		}
	}

	_converting_start = tic();

	if (_convert_D1)
	{
		_send_cmd(REG_MS5611_D1_OSR_4096);
	}
	else
	{
		_send_cmd(REG_MS5611_D2_OSR_4096);
	}

	if (_D1 != 0 && _D2 != 0)
	{
		_calc_values();
		_D1 = 0;
		_D2 = 0;
	}
}

void _calc_values()
{
	int32_t dT, TEMP, P;
	int64_t OFF, SENS;

	dT = _D2 - ((uint32_t) _C[5] << 8);
	TEMP = 2000 + ((int32_t)dT * ((int32_t)_C[6]) >> 23);

	OFF  = ((uint64_t)_C[2] <<16) + (((int64_t)_C[4] * dT) >> 7);
	SENS = ((uint64_t)_C[1] <<15) + (((int64_t)_C[3] * dT) >> 8);
	P = (((_D1 * SENS) >> 21) - OFF) >> 15;

	uint32_t idx = (storageIdx + 1)%2;

	_ms5611_data_storage[idx].temp = (float)TEMP / 100.0f;
	_ms5611_data_storage[idx].baroValue = (float)P / 100.0f;
	_ms5611_data_storage[idx].isNew = 1;

	storageIdx = idx; // switch storage banks
}

void px4_ms5611_get(ms5611_data_st * data)
{
	memcpy(data, &_ms5611_data_storage[storageIdx], sizeof(ms5611_data_st));
	_ms5611_data_storage[storageIdx].isNew = 0;
}

void _send_cmd(uint8_t cmd)
{
	uint8_t buff[1] = { cmd };
	MS5611_ENA;
	px4_spi_drv_transmit(PX4_SPI1, buff, buff, sizeof(buff));
	MS5611_DISA;
}

uint16_t _get_prom_value(uint8_t addr)
{
	uint8_t buff[3] = { addr, 0, 0 };
	MS5611_ENA;
	px4_spi_drv_transmit(PX4_SPI1, buff, buff, sizeof(buff));
	MS5611_DISA;
	return (((uint16_t) buff[1] << 8) | ((uint16_t) buff[2]));
}

uint32_t _get_meas_value()
{
	uint8_t cmd[4] = { 0, 0, 0, 0 };

	MS5611_ENA;
	px4_spi_drv_transmit(PX4_SPI1, cmd, cmd, sizeof(cmd));
	MS5611_DISA;

	return ((uint32_t) cmd[1] << 16) | ((uint32_t) cmd[2] << 8) | ((uint32_t) cmd[3]);
}

static uint32_t _ms5611_init_sensor()
{
	// set spi speed (~20MHZ)
	px4_spi_drv_set_clock_speed(PX4_SPI1, SPI_DRV_PRESCALER_4);

	// reset baro chip for init
	_send_cmd(CMD_MS5611_RESET);
	HAL_Delay(10);

	//	read prom calibration values
	_C[1] = _get_prom_value(REG_MS5611_PROM_C1);	// Pressure sensitivity
	_C[2] = _get_prom_value(REG_MS5611_PROM_C2);	// Pressure offset
	_C[3] = _get_prom_value(REG_MS5611_PROM_C3);	// Temperature coefficient of pressure sensitivity
	_C[4] = _get_prom_value(REG_MS5611_PROM_C4);	// Temperature coefficient of pressure offset
	_C[5] = _get_prom_value(REG_MS5611_PROM_C5);	// Reference temperature
	_C[6] = _get_prom_value(REG_MS5611_PROM_C6);	// Temperature coefficient of the temperature

//	px4debug(eMS5611," C1: ");
//	debug_print_int(_C[1]);
//	px4debug(eMS5611," C2: ");
//	debug_print_int(_C[2]);
//	px4debug(eMS5611," C3: ");
//	debug_print_int(_C[3]);
//	px4debug(eMS5611," C4: ");
//	debug_print_int(_C[4]);
//	px4debug(eMS5611," C5: ");
//	debug_print_int(_C[5]);
//	px4debug(eMS5611," C6: ");
//	debug_print_int(_C[6]);
//	px4debug(eMS5611,"\r\n");

	return SUCCESS;
}

void px4_ms5611_init()
{
	memset(&_ms5611_data_storage, 0, sizeof(_ms5611_data_storage));
	memset(_C, 0, sizeof(_C));

	px4_spi_drv_init(PX4_SPI1);
	_ms5611_init_sensor();
	_module_state = ENABLE;
	px4debug("ms5611 init ok \r\n");
}
