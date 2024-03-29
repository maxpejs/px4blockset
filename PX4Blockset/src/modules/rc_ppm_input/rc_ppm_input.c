#include "defines.h"
#include "rc_ppm_input.h"

static rc_ppm_data_st rc_in_data_storage[2];
static uint32_t 			storage_idx 	= 0;
static FunctionalState		module_init = DISABLE;

void px4_rc_ppm_get(rc_ppm_data_st * data)
{
	memcpy(data, &rc_in_data_storage[storage_idx], sizeof(rc_ppm_data_st));
}

void px4_rc_ppm_init(void)
{
	memset(&rc_in_data_storage, 0, sizeof(rc_in_data_storage));
	pxio_driver_init();
	module_init = ENABLE;
	px4debug("rc_in init ok\n");
}

void px4_rc_ppm_update()
{
	if (module_init == DISABLE)
	{
		return;
	}

	// same implementation as in official pixhawk driver px4io.cpp
	const unsigned prolog = PX4IO_P_RAW_RC_BASE - PX4IO_P_RAW_RC_COUNT;
	uint16_t regs[RC_MAX_CHANNELS + prolog];

	// Read channel data (channel count, ...) and the some first channels
	int ret = pxio_driver_reg_get( PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_COUNT, &regs[0], prolog + 9);
	if (ret != SUCCESS)
	{
		px4debug("req ch cnt + ch part err!\n");
		return;
	}

	// get the channel count
	uint16_t channel_count = regs[PX4IO_P_RAW_RC_COUNT];
	
	if (channel_count > RC_MAX_CHANNELS) 
	{
		channel_count = RC_MAX_CHANNELS;
	}

	// if there are more channels than we read first time, so get the values of the next channel
	if (channel_count > 9)
	{
		ret = pxio_driver_reg_get(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_BASE + 9,
									&regs[prolog + 9], channel_count - 9);
		if (ret != SUCCESS)
		{
			px4debug("req next chs err!\n");
			return;
		}
	}

	// get the storage bank index
	uint32_t idx = (storage_idx + 1) % 2;

	// Get the channel count
	rc_in_data_storage[idx].rc_failsafe = (regs[PX4IO_P_RAW_RC_FLAGS] & PX4IO_P_RAW_RC_FLAGS_FAILSAFE);
	rc_in_data_storage[idx].rc_lost = !(regs[PX4IO_P_RAW_RC_FLAGS] & PX4IO_P_RAW_RC_FLAGS_RC_OK);
	rc_in_data_storage[idx].rc_lost_frame_count = regs[PX4IO_P_RAW_LOST_FRAME_COUNT];
	rc_in_data_storage[idx].rc_total_frame_count = regs[PX4IO_P_RAW_FRAME_COUNT];
	rc_in_data_storage[idx].channel_cnt = channel_count;
	
	memcpy(&rc_in_data_storage[idx].channels, &regs[prolog], rc_in_data_storage[idx].channel_cnt * 2);

	// switch index to other storage bank
	storage_idx = idx;
}
