#ifndef QUNERUM_QFONT_H
#define QUNERUM_QFONT_H

#include <stdint.h>

#define QFONT_REGULAR       0b00000001
#define QFONT_BOLD          0b00000010
#define QFONT_ITALIC        0b00000100
#define QFONT_UNDERLINE     0b00001000
#define QFONT_STRIKETHROUGH 0b00010000
#define QFONT_UPPERCASE     0b00100000
#define QFONT_LOWERCASE     0b01000000

typedef struct {
	uint8_t sizeX, sizeY;
	uint32_t capacity, charCount;
	int8_t* chars; // code, skip Y, lines...
	// line:
	// count, ...
} qFont;

#endif
