#ifndef GPS_RMC_PARCER_H
#define GPS_RMC_PARCER_H

#define SPEED_FACTOR_KNOTS_TO_MS	0.51444f // Speed converting factor knots->m/s

#include <inttypes.h>
#include <timestamp.h>
#include <utilities.h>
#include <string.h>
#include <stdlib.h>

/**
* The struct with data of the NMEA0183 RMC sentence format
*/
typedef struct
{
	uint8_t  hour;
	uint8_t  Minute;
	uint8_t  Second;
	uint8_t  Day;
	uint8_t  Month;
	uint8_t  Year;
	uint8_t  Valid;			    // A - Data valid, V - data not valid
	float  	 Latitude;			// latitude
	float  	 Longitude;			// longitude
	float    Ground_speed_ms;	// speed over ground (m/s) * 10^-2
	float 	 Ground_course;		// course over ground in degrees * 10^-2
	float  	 MagVar;			// magnetic variation, East(+)/West(-) in degrees
	uint8_t  Quality;
}gps_rmc_packet_st;

void parse_nmea_rmc_sentence(uint8_t * rxMsgBuffer, gps_rmc_packet_st * rmcData);

#endif // GPS_RMC_PARCER_H
