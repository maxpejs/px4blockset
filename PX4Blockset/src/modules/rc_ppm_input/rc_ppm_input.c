#include "rc_ppm_input.h"

static rc_ppm_input_data_st 	rc_in_data_storage[2];
static uint32_t 				_idxsection 			= 0;
static FunctionalState			_module_init 			= DISABLE;

void px4_rc_ppm_input_get(rc_ppm_input_data_st * data)
{
	memcpy(data, &rc_in_data_storage[_idxsection], sizeof(rc_ppm_input_data_st));
}

void px4_rc_ppm_input_init(void)
{
	px4debug(ePPM_INPUT, "rc_ppm_input init ...\r\n");
	memset(&rc_in_data_storage, 0, sizeof(rc_in_data_storage));
	pxio_driver_init();
	_module_init = ENABLE;
	px4debug(ePPM_INPUT, "rc_ppm_input init ok\r\n");
}

void px4_rc_ppm_input_update()
{
	if (_module_init == DISABLE)
	{
		return;
	}

	int ret;

	const unsigned reg_offset = PX4IO_P_RAW_RC_BASE - PX4IO_P_RAW_RC_COUNT;
	uint16_t regs[MAX_RC_CHANNEL_CNT + reg_offset];

	// Read channel data (channel count, ...) and the some first channels
	ret = pxio_driver_reg_get( PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_COUNT, &regs[0], reg_offset + CHANNEL_PART);
	if (ret != SUCCESS)
	{
		px4debug(ePPM_INPUT, "req ch cnt + ch part err!\r\n");
		return;
	}

	// get index from last updated storage bank
	uint32_t idx = _idxsection;

	// switch index to other storage bank
	_idxsection = (_idxsection + 1) % 2;

	// Get the channel count
	rc_in_data_storage[idx].channel_cnt = regs[PX4IO_P_RAW_RC_COUNT];

	if (rc_in_data_storage[idx].channel_cnt > CHANNEL_PART)
	{
		ret = pxio_driver_reg_get(	PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_BASE + CHANNEL_PART,
									&regs[reg_offset + CHANNEL_PART], rc_in_data_storage[idx].channel_cnt - CHANNEL_PART);
		if (ret != SUCCESS)
		{
			px4debug(ePPM_INPUT, "req next chs err!\r\n");
			return;
		}
	}

	memcpy(&rc_in_data_storage[idx].channels, &regs[reg_offset], rc_in_data_storage[idx].channel_cnt * 2);
}
