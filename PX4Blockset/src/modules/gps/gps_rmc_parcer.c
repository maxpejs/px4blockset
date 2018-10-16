#include "gps_rmc_parcer.h"

uint8_t get_next_field(uint8_t * src, uint8_t * dst, uint8_t idx)
{
	uint8_t i = 0;
	uint8_t k = idx;

	while (src[k] != ',' && src[k] != '*')
	{
		dst[i++] = src[k++];
	}
	dst[i] = 0;

	// return start index of the next field
	return k+1;
}

void parse_nmea_rmc_sentence(uint8_t * rxMsgBuffer, gps_rmc_packet_st * rmcData)
{
	uint32_t idx = 7;
	uint8_t tmpBuff[20];

	// first check if we received a valid sentence
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);	// skip time field
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->Valid = tmpBuff[0] == 'A'; // A=Active, V=void

	if(!rmcData->Valid)
	{
		return;
	}

	// data is valid, so we can parse the received sentence from begin
	idx = 7;
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);

	// utc time
	rmcData->hour	 = ascii_2_nibble(tmpBuff[0]) * 10 + ascii_2_nibble(tmpBuff[1]);
	rmcData->Minute  = ascii_2_nibble(tmpBuff[2]) * 10 + ascii_2_nibble(tmpBuff[3]);
	rmcData->Second  = ascii_2_nibble(tmpBuff[4]) * 10 + ascii_2_nibble(tmpBuff[5]);

	// validity
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx); // skip, already parsed

	// Latitude
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->Latitude = (float) (ascii_2_nibble(tmpBuff[0]) * 10 + ascii_2_nibble(tmpBuff[1]));
	tmpBuff[0] = '0';
	tmpBuff[1] = '0';
	rmcData->Latitude += ((float)atof((const char *)tmpBuff) / 60.0f);

	// Nord/South
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->Latitude *= (tmpBuff[0] == 'S') ? -1 : 1;

	// Longitude
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);

	rmcData->Longitude = (float)(ascii_2_nibble(tmpBuff[0]) * 100 + ascii_2_nibble(tmpBuff[1]) * 10 + ascii_2_nibble(tmpBuff[2]));
	tmpBuff[0] = '0';
	tmpBuff[1] = '0';
	tmpBuff[2] = '0';
	rmcData->Longitude += ((float)atof((const char *)tmpBuff) / 60.0f);

	// East/West
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->Longitude *= (tmpBuff[0] == 'W') ? -1 : 1;

	// speed in knotes
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->Ground_speed_ms = (float)atof((const char *)tmpBuff) * SPEED_FACTOR_KNOTS_TO_MS;

	// direction in degrees
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->Ground_course = (float)atof((const char *)tmpBuff);

	// utc date
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->Day	= ascii_2_nibble(tmpBuff[0]) * 10 + ascii_2_nibble(tmpBuff[1]);
	rmcData->Month  = ascii_2_nibble(tmpBuff[2]) * 10 + ascii_2_nibble(tmpBuff[3]);
	rmcData->Year   = ascii_2_nibble(tmpBuff[4]) * 10 + ascii_2_nibble(tmpBuff[5]);

	// declination
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->MagVar = (float)atof((const char *)tmpBuff);

	// declination direction (E/W)
	idx = get_next_field(rxMsgBuffer, tmpBuff, idx);
	rmcData->MagVar *= (tmpBuff[0] == 'W') ? -1 : 1 ;

	if(rxMsgBuffer[idx] == ',')
	{
		// quality, additional field according to nmea 2.3
		rmcData->Quality = rxMsgBuffer[idx+1];
	}
}
