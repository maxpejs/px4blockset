#ifndef GPS_H
#define GPS_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <inttypes.h>
#include <error_handler.h>
#include <comm_itf.h>
#include <timestamp.h>
#include <gps_rmc_parcer.h>

#define GPS_RECEIVE_TIMEOUT				1500000	// timeout im µs after that the rmc data set to invalid if no new sentence are incoming
#define RMC_DATA_UPDATE_FREQ		1		// update frequence of NMEA 0183 RMC

#define RMC_HEADER					"$GPRMC"

#define GPS_SENTENCE_BUFF_SIZE		100
#define GPS_RING_BUFF_SIZE			500
#define GPS_RX_PACKAGE_LENGTH		50	// call "received"-interrupt after receiving this amount of chars

#if GPS_RING_BUFF_SIZE % GPS_RX_PACKAGE_LENGTH != 0
#error "Wrong size of GPS_RING_BUFF_SIZE or GPS_RX_PACKAGE_LENGTH. Should be: GPS_RING_BUFF_SIZE % GPS_RX_PACKAGE_LENGTH = 0"
#endif

/**
*	TODO
*/
void px4_gps_init(uint32_t baud);

/**
*	TODO
*/
void px4_gps_get(gps_rmc_packet_st * pData);

/**
*	TODO
*/
void px4_gps_get_raw(uint8_t * buff);

/**
*	TODO
*/
void px4_gps_update(void);

/**
*	TODO
*/
void px4_gps_rx_complete_event(void);

#endif // GPS_H
