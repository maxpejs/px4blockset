#include <utilities.h>

uint8_t ascii_2_nibble(uint8_t c)
{
	return (c < 'A') ? (c - '0') : (c + 10) - 'A';
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

size_t min(size_t a, size_t b)
{
	return a < b ? a : b;
}
