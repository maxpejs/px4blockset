#include <utilities.h>

uint8_t ascii_2_nibble(uint8_t c)
{
	return (c < 'A') ? (c - '0') : (c + 10) - 'A';
}

double my_pow(int32_t base, int32_t exp)
{
	double ret = 1.0;

	if (exp > 0)
	{
		while (exp--)
		{
			ret *= base;
		}
	}
	else
	{
		while (exp++)
		{
			ret /= base;
		}
	}

	return ret;
}

int32_t my_i64toa(int64_t val, char * str)
{
	int32_t idx = 0;

	if (val == 0)
	{
		str[idx++] = '0';
	}
	else
	{
		if (val < 0)
		{
			str[idx++] = '-';
			val *= -1;
		}

		uint32_t dig_idx = 0;
		uint32_t digits[30];

		while (val > 0)
		{
			digits[dig_idx] = val % 10;
			val /= 10;
			dig_idx++;
		}

		while (dig_idx > 0)
		{
			dig_idx--;
			str[idx++] = '0' + digits[dig_idx];
		}
	}
	str[idx] = 0;
	return idx;
}

int32_t my_i32toa(int32_t val, char * str)
{
	int32_t idx = 0;

	if (val == 0)
	{
		str[idx++] = '0';
	}
	else
	{
		if (val < 0)
		{
			str[idx++] = '-';
			val *= -1;
		}

		uint32_t dig_idx = 0;
		uint32_t digits[12];

		while (val > 0)
		{
			digits[dig_idx] = val % 10;
			val /= 10;
			dig_idx++;
		}

		while (dig_idx > 0)
		{
			dig_idx--;
			str[idx++] = '0' + digits[dig_idx];
		}
	}
	str[idx] = 0;
	return idx;
}

int32_t my_dtoa(double val, char * str, uint32_t precision)
{
	int32_t idx = 0;

	if (val == 0)
	{
		str[idx++] = '0';
		str[idx] = 0;
	}
	else
	{
		if (val < 0)
		{
			str[idx++] = '-';
			val *= -1.0;
		}

		int64_t part1 = (int64_t)val;

		if (part1 == 0)
		{
			str[idx++] = '0';
		}

		double prec = my_pow(10, (int32_t)precision * -1);
		uint32_t c = 0;

		// Solange Stellen nach der Kommma vorhanden
		// UND
		// aktuelle Genauigkeit nicht über erforderliche ist
		while (((val - part1) > prec) && (c < precision))
		{
			c++;
			val *= 10;
			part1 = (int64_t)val;

			if (part1 == 0)
			{
				str[idx++] = '0';
			}
		}

		idx += my_i64toa(part1, &str[idx]);

		if (c > 0)
		{
			for (uint32_t i = idx; i >= (idx - c); i--)
			{
				str[i] = str[i - 1];
			}

			str[idx - c] = '.';
		}

		idx++;
		str[idx] = 0;
	}
	return idx;
}

int32_t my_ftoa(float val, char * str, uint32_t precision)
{
	int32_t idx = 0;

	if (val == 0)
	{
		str[idx++] = '0';
		str[idx] = 0;
	}
	else
	{
		if (val < 0)
		{
			str[idx++] = '-';
			val *= -1.0;
		}

		int32_t part1 = (int32_t)val;

		if (part1 == 0)
		{
			str[idx++] = '0';
		}

		float prec = my_pow(10, (int32_t)precision * -1);
		uint32_t c = 0;

		// Solange Stellen nach der Kommma vorhanden
		// UND
		// aktuelle Genauigkeit nicht über erforderliche ist
		while (((val - part1) > prec) && (c < precision))
		{
			c++;
			val *= 10;
			part1 = (int32_t)val;

			if (part1 == 0)
			{
				str[idx++] = '0';
			}
		}

		idx += my_i32toa(part1, &str[idx]);

		if (c > 0)
		{
			for (uint32_t i = idx; i >= (idx - c); i--)
			{
				str[i] = str[i - 1];
			}
			str[idx - c] = '.';
		}

		idx++;
		str[idx] = 0;
	}
	return idx;
}

void uppercase(char * buff)
{
	while (*buff != 0)
	{
		if ((*buff) >= 97 && (*buff) <= 122)
		{
			*buff -= 32;
		}
		buff++;
	}
}


