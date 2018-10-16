#include "rc_ppm_input.h"

static rc_ppm_input_data_st rc_in_data_storage;
static uint32_t _rc_ppm_input_runtime 	= 0;
static FunctionalState	_module_init 	= DISABLE;

void px4_rc_ppm_input_get(rc_ppm_input_data_st * data)
{
	memcpy(data, &rc_in_data_storage, sizeof(rc_in_data_storage));
}

void px4_rc_ppm_input_init(void)
{
	memset(&rc_in_data_storage, 0, sizeof(rc_in_data_storage));
	pxio_driver_init();
	_module_init = ENABLE;
	debug_print_string("rc_ppm_input init ok\r\n");
}

void px4_rc_ppm_input_task_function(void const * argv)
{
	uint64_t start = tic();

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
		debug_print_string("req ch cnt + ch part err!\r\n");
		return;
	}

	// Get the channel count
	rc_in_data_storage.channel_cnt = regs[PX4IO_P_RAW_RC_COUNT];

	if (rc_in_data_storage.channel_cnt > CHANNEL_PART)
	{
		ret = pxio_driver_reg_get(	PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_BASE + CHANNEL_PART,
									&regs[reg_offset + CHANNEL_PART], rc_in_data_storage.channel_cnt - CHANNEL_PART);
		if (ret != SUCCESS)
		{
			debug_print_string("req next chs err!\r\n");
			return;
		}
	}

	memcpy(&rc_in_data_storage.channels, &regs[reg_offset], rc_in_data_storage.channel_cnt * 2);
	
	_rc_ppm_input_runtime = (uint32_t)toc(start);
}

uint32_t px4_rc_ppm_input_getruntime(void)
{
	return _rc_ppm_input_runtime;
}
