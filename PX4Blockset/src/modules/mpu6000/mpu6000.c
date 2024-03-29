#include "mpu6000.h"

static uint32_t 			_module_state = DISABLE;
static mpu6000_data_st 		_mpu6000_data_storage[2];
static uint32_t _storage_idx;
static mpu6000_settings_st 	mpu6000_settings;

static float _accelScale 	= 1.f;
static float _gyroScale 	= 1.f;

static uint32_t _mpu6000_runtime = 0;

static float calc_accel(uint8_t hb, uint8_t lb);
static float calc_gyro(uint8_t hb, uint8_t lb);
static float calc_temp(uint8_t hb, uint8_t lb);

static float getAccelScale(uint8_t conf);
static float getGyroScale(uint8_t conf);

static void mpu6000_init_sensor();


void px4_mpu6000_init(mpu6000_settings_st * in_settings)
{
	px4debug("mpu6000 init ... \n");
	memcpy(&mpu6000_settings, in_settings, sizeof(mpu6000_settings_st));
	memset(&_mpu6000_data_storage, 0, sizeof(_mpu6000_data_storage));

	px4_spi_drv_init(PX4_SPI1);
	mpu6000_init_sensor();

	_accelScale = getAccelScale(mpu6000_settings.accel_range);
	_gyroScale  = getGyroScale(mpu6000_settings.gyro_range);

	_storage_idx = 0;

	_module_state = ENABLE;
	px4debug("mpu6000 init ok \n");
}

void px4_mpu6000_update()
{
	uint32_t start = tic();

	if (_module_state == DISABLE)
	{
		return;
	}

	uint8_t rxBuff[MPU_MSG_BUFFERSIZE];
	memset(rxBuff, 0, sizeof(rxBuff));

	rxBuff[0] = (ACCEL_XOUT_H | 0x80);

	// set spi to speed (21Mhz)
	px4_spi_drv_set_clock_speed(PX4_SPI1, SPI_DRV_PRESCALER_4);

	// switch the storage bank
	uint32_t idx = (_storage_idx + 1) % 2;

	// perform data send + receive
	MPU6000_ENA;
	ErrorStatus status = px4_spi_drv_transmit(PX4_SPI1, rxBuff, rxBuff, MPU_MSG_BUFFERSIZE);
	MPU6000_DISA;

	if (status == SUCCESS)
	{
		// register values are stored beginning from rxBuff[1]
		_mpu6000_data_storage[idx].accel_x = (calc_accel(rxBuff[1], rxBuff[2]) * mpu6000_settings.scale_accel_x) + mpu6000_settings.offset_accel_x;
		_mpu6000_data_storage[idx].accel_y = (calc_accel(rxBuff[3], rxBuff[4]) * mpu6000_settings.scale_accel_y) + mpu6000_settings.offset_accel_y;
		_mpu6000_data_storage[idx].accel_z = (calc_accel(rxBuff[5], rxBuff[6]) * mpu6000_settings.scale_accel_z) + mpu6000_settings.offset_accel_z;
		_mpu6000_data_storage[idx].temp 	 = calc_temp(rxBuff[7],  rxBuff[8]);
		_mpu6000_data_storage[idx].gyro_x = calc_gyro(rxBuff[9],  rxBuff[10]);
		_mpu6000_data_storage[idx].gyro_y = calc_gyro(rxBuff[11], rxBuff[12]);
		_mpu6000_data_storage[idx].gyro_z = calc_gyro(rxBuff[13], rxBuff[14]);
	}

	_storage_idx = idx;
	_mpu6000_runtime = toc(start);
}

static float getAccelScale(uint8_t conf)
{
	switch(conf)
	{
		case BITS_FS_2G:  return 16384.0f; break;
		case BITS_FS_4G:  return 8192.0f;  break;
		case BITS_FS_8G:  return 4096.0f;  break;
		case BITS_FS_16G: return 2048.0f;  break;
		default: 		  return 16384.0f;
	}
}

static float getGyroScale(uint8_t conf)
{
	switch(conf)
	{
		case BITS_FS_250DPS:  return 131.0f; break;
		case BITS_FS_500DPS:  return 65.5f;  break;
		case BITS_FS_1000DPS: return 32.8f;  break;
		case BITS_FS_2000DPS: return 16.4f;  break;
		default:              return 131.0f;
	}
}

static float calc_accel(uint8_t hb, uint8_t lb)
{
	return (float) TWO_UINT8_TO_INT16(hb, lb) / _accelScale;
}

static float calc_gyro(uint8_t hb, uint8_t lb)
{
	return (float) TWO_UINT8_TO_INT16(hb, lb) / _gyroScale;
}

static float calc_temp(uint8_t hb, uint8_t lb)
{
	return ((float)TWO_UINT8_TO_INT16(hb, lb) / 340.0f) + 36.53f;
}

void px4_mpu6000_get(mpu6000_data_st * data)
{
	memcpy(data, &_mpu6000_data_storage[_storage_idx], sizeof(mpu6000_data_st));
}

static void mpu6000_reg_set(uint8_t reg, uint8_t val)
{
	uint8_t cmd[2] = { reg, val };
	MPU6000_ENA;
	px4_spi_drv_transmit(PX4_SPI1, cmd, cmd, sizeof(cmd));
	MPU6000_DISA;
}

static uint8_t mpu6000_reg_get(uint8_t reg)
{
	uint8_t cmd[2] = {reg|BIT_READ_REG , 0};
	MPU6000_ENA;
	px4_spi_drv_transmit(PX4_SPI1, cmd, cmd, sizeof(cmd));
	MPU6000_DISA;
	return cmd[1];
}

static void mpu6000_init_sensor()
{
	uint8_t tries = 5;
	uint8_t r1, r2, r3, r4, r5, r6, r7;

	// set spi to low speed ca. 650Khz
	px4_spi_drv_set_clock_speed(PX4_SPI1, SPI_DRV_PRESCALER_128);

	while (tries-- != 0)
	{
		mpu6000_reg_set(PWR_MGMT_1, BIT_H_RESET);
		HAL_Delay(100);

		mpu6000_reg_set(SIGNAL_PATH_RESET, BIT_GYRO_ACCEL_TEMP_RESET);
		HAL_Delay(100);

		uint8_t c = mpu6000_reg_get(WHOAMI);

		if (c !=WHOAMI_MPU6000_ID)
			px4debug("WHOAMI\n");

		HAL_Delay(100);

		// wite settings to register
		mpu6000_reg_set(PWR_MGMT_1, 			MPU_CLK_SEL_PLLGYROZ); 			HAL_Delay(10);
		mpu6000_reg_set(USER_CTRL, 				BIT_I2C_IF_DIS);				HAL_Delay(10);
		mpu6000_reg_set(PWR_MGMT_2, 			0x00);							HAL_Delay(10);
		mpu6000_reg_set(MPU6000_SMPLRT_DIV, 	mpu6000_settings.smplrt_cfg);	HAL_Delay(10);
		mpu6000_reg_set(MPU6000_CFG, 			mpu6000_settings.dlpf_cfg);		HAL_Delay(10);
		mpu6000_reg_set(MPU6000_GYRO_CONFIG, 	mpu6000_settings.gyro_range);	HAL_Delay(10);
		mpu6000_reg_set(MPU6000_ACCEL_CONFIG,	mpu6000_settings.accel_range);	HAL_Delay(10);

		// read settings from sensor
		r1 = mpu6000_reg_get(PWR_MGMT_1); 			HAL_Delay(10);
		r2 = mpu6000_reg_get(USER_CTRL);			HAL_Delay(10);
		r3 = mpu6000_reg_get(PWR_MGMT_2);			HAL_Delay(10);
		r4 = mpu6000_reg_get(MPU6000_SMPLRT_DIV);	HAL_Delay(10);
		r5 = mpu6000_reg_get(MPU6000_CFG);			HAL_Delay(10);
		r6 = mpu6000_reg_get(MPU6000_GYRO_CONFIG);	HAL_Delay(10);
		r7 = mpu6000_reg_get(MPU6000_ACCEL_CONFIG);

		// check if settings were set correct
		if (r1 != MPU_CLK_SEL_PLLGYROZ)
			px4debug("PWR_MGMT_1\n");

		if (r2 != BIT_I2C_IF_DIS)
			px4debug("USER_CTRL\n");

		if (r3 != 0x00)
			px4debug("PWR_MGMT_2\n");

		if (r4 != mpu6000_settings.smplrt_cfg)
			px4debug("SMPLRT_DIV\n");

		if (r5 != mpu6000_settings.dlpf_cfg)
			px4debug("CFG\n");

		if (r6 != mpu6000_settings.gyro_range)
			px4debug("GYRO_CONFIG\n");

		if (r7 != mpu6000_settings.accel_range)
			px4debug("ACCEL_CONFIG\n");

		if ((c == WHOAMI_MPU6000_ID) && (r1 == MPU_CLK_SEL_PLLGYROZ) && (r2 == BIT_I2C_IF_DIS) && (r3 == 0x00)
				&& (r4 == mpu6000_settings.smplrt_cfg) && (r5 == mpu6000_settings.dlpf_cfg) && (r6 == mpu6000_settings.gyro_range)
				&& (r7 == mpu6000_settings.accel_range))
		{
			break;	// correct settings were stored
		}

		HAL_Delay(100);
	}

	if (tries == 0)
	{
		px4debug("mpu6000 sensor init error!\n");
		error_handler(0);
	}
}

