#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "qfont.h"

static float
	pivX = 0.0f, pivY = 0.0f,
	qfSize = 32,
	qfR = 1.0f, qfG = 1.0f, qfB = 1.0f, qfA = 1.0f;
static uint8_t qfStyle = QFONT_REGULAR;
static void (*drawFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a);
void qfSetDrawRect(const float pivotX, const float pivotY, void (*drawRectFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a)) {
	pivX = pivotX; pivY = pivotY;
	drawFunc = drawRectFunc;
}
void qfConfigure(const float fontSize, const uint8_t style, const float r, const float g, const float b, const float a) {
	qfSize = fontSize;
	qfStyle = style;
	qfR = r;
	qfG = g;
	qfB = b;
	qfA = a;
}

static void* mc(void* dest, const void* src, const uint32_t n) {
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;
	for (uint32_t i = 0; i < n; i++) d[i] = s[i];
	return dest;
}

qfCharData qfCreate(const uint8_t sizeX, const uint8_t sizeY) { return (qfCharData) { .sizeX = sizeX, .sizeY = sizeY, }; }
static int8_t** allChars = 0;
static uint32_t highestID = 0;
void qfAddChar(const qfCharData charData, const int8_t* d) {
	if (!d) return;
	const uint32_t id =
		((uint32_t)(uint8_t)d[0] << 24) |
		((uint32_t)(uint8_t)d[1] << 16) |
		((uint32_t)(uint8_t)d[2] << 8)  |
		 (uint32_t)(uint8_t)d[3];
	if (id >= highestID) {
		const uint32_t newCapacity = id + 1;
		int8_t** temp = (int8_t**)realloc(allChars, newCapacity * sizeof(int8_t*));
		if (!temp) return;
		allChars = temp;
		for (uint32_t i = highestID; i < newCapacity; i++) allChars[i] = 0;
		highestID = newCapacity;
	}
	if (allChars[id]) free(allChars[id]);
	const uint8_t goNext = (uint8_t)d[5];
	uint16_t size = 4, actualID = 6;
	for (uint8_t i = 0; i < goNext; i++) {
		const uint8_t len = 1 + (uint8_t)d[actualID];
		actualID += len;
		size += len;
	}
	allChars[id] = (int8_t*)malloc(size);
	mc(allChars[id], d + 2, size);
	allChars[id][0] = charData.sizeX;
	allChars[id][1] = charData.sizeY;
	printf(
	"[QFont] Adding char:\n"
	"|    ID: 0x%X\n"
	"\\_ Size: %u\n",
	id, size);
	// for (int i = 0; i < size - 1; i++) printf("%i, ", allChars[id][i]);
	// printf("%i\n", allChars[id][size - 1]);
}
#define qfl qfFont,(int8_t[])
void qfInit() {
	qfCharData qfFont = qfCreate(8, 12);
	qfAddChar(qfl{0,0,0,0x21, 2,8, // !
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		0,
		2, 3,-2,
		2, 3,-2
	});
	qfAddChar(qfl{0,0,0,0x22, 2,2, // "
		4, 1,-2,2,-2,
		4, 1,-2,2,-2
	});
	qfAddChar(qfl{0,0,0,0x23, 2,8, // #
		4, 2,-1,2,-1,
		4, 2,-1,2,-1,
		2, 1,-6,
		4, 2,-1,2,-1,
		4, 2,-1,2,-1,
		2, 1,-6,
		4, 2,-1,2,-1,
		4, 2,-1,2,-1
	});
	qfAddChar(qfl{0,0,0,0x24, 2,8, // $
		2, 3,-2,
		2, 2,-5,
		2, 1,-2,
		2, 2,-4,
		2, 5,-2,
		2, 1,-5,
		2, 3,-2,
		0
	});
	/*
	qfAddChar(qfl{0,0,0,0x25, 2,8, // %
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x26, 2,8, // &
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x27, 2,8, // '
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x28, 2,8, // (
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x29, 2,8, // )
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x2a, 2,8, // *
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x2b, 2,8, // +
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x2c, 2,8, // ,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x2d, 2,8, // -
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x2e, 2,8, // .
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	qfAddChar(qfl{0,0,0,0x2f, 2,8, // /
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	});
	*/
}
