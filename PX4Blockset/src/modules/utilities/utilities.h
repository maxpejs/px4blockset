#ifndef ASCII_CONVERT_H
#define ASCII_CONVERT_H
#include <inttypes.h>
#include <stddef.h>

uint8_t ascii_2_nibble(uint8_t c);

void uppercase(char * buff);

size_t min(size_t a, size_t b);

size_t max(size_t a, size_t b);

#endif // ASCII_CONVERT_H
