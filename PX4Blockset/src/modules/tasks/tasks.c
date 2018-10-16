#include "tasks.h"

void px4_tasks_read_inputs()
{
	px4_rc_ppm_input_update();
	px4_gps_update();
	px4_mpu6000_update();
	px4_ms5611_update();
	px4_hmc5883_update();
}

void px4_tasks_write_outputs()
{
	px4_color_power_led_update();
	px4_pwm_main_out_update();
	px4_signal_output_task();
	px4_sd_card_logger_task();

	// pwm_aux_out => wird direkt ausgegeben
	// fmu_amber_led => wird direkt ausgegeben
}

