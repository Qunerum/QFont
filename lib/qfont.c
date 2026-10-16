#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "qfont.h"

static float
	pivX = 0.0f, pivY = 0.0f,
	qfSize = 32.0f,
	qfR = 1.0f, qfG = 1.0f, qfB = 1.0f, qfA = 1.0f;
static uint8_t qfStyle = QFONT_REGULAR;
static void (*drawFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a);
void qfSetDrawRect(const int8_t px, const int8_t py, void (*drawRectFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a)) {
	pivX = px <= -1 ? 0.0f : px >= 1 ? 1.0f : 0.5f;
	pivY = py <= -1 ? 0.0f : py >= 1 ? 1.0f : 0.5f;
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
qfCharData qfCreate(const uint8_t sizeX, const uint8_t sizeY) { return (qfCharData) { .sizeX = sizeX, .sizeY = sizeY }; }
static int8_t** allChars = 0;
static uint32_t charCount = 0, highestID = 0;
#define u8d (uint8_t)d
void qfAddChar(const qfCharData charData, const int8_t* d) {
	if (!d) return;
	const uint32_t id =
		((uint32_t)u8d[0] << 24) |
		((uint32_t)u8d[1] << 16) |
		((uint32_t)u8d[2] << 8)  |
		 (uint32_t)u8d[3];
	if (id >= highestID) {
		const uint32_t newCapacity = id + 1;
		int8_t** temp = (int8_t**)realloc(allChars, newCapacity * sizeof(int8_t*));
		if (!temp) return;
		allChars = temp;
		for (uint32_t i = highestID; i < newCapacity; i++) allChars[i] = 0;
		highestID = newCapacity;
	}
	if (allChars[id]) free(allChars[id]);
	const uint8_t goNext = u8d[5];
	uint16_t size = 4, actualID = 6;
	for (uint8_t i = 0; i < goNext; i++) {
		const uint8_t len = 1 + u8d[actualID];
		actualID += len;
		size += len;
	}
	allChars[id] = (int8_t*)malloc(size);
	mc(allChars[id], d + 2, size);
	allChars[id][0] = charData.sizeX;
	allChars[id][1] = charData.sizeY;
	charCount++;
	printf("[QFont][%u] Added char [%c]: WH:[%ux%u] ID:[0x%X] LS:[%uB]\n", charCount, id > 31 && id < 127 ? id : '?', charData.sizeX, charData.sizeY, id, size);
}
float qfDrawChar(const float posX, const float posY, const uint32_t c) {
	if (c >= highestID || !allChars[c]) return .0f;
	const int8_t* d = allChars[c];
	const uint8_t
		w = u8d[0], h = u8d[1],
		s = u8d[2], rn = u8d[3],
		n = s + rn > h ? h - s : rn;
	if (s >= h) return .0f;
	uint8_t x = 0, y = s, i = 4;
	const float sc = qfSize / h;
	for (uint8_t r = 0; r < n; r++) {
		const uint8_t len = u8d[i] + 1;
		for (uint8_t l = 1; l < len; l++) {
			const int8_t dil = d[i + l];
			const uint8_t
				draw = dil < 0,
				rv = draw ? -dil : dil,
				v = rv > w ? w : rv;
			if (draw) drawFunc(posX + (x + v * pivX) * sc, posY - (y + pivY) * sc, v * sc, sc, qfR,qfG,qfB,qfA);
			x += v;
		}
		i += len;
		x = 0;
		y++;
	}
	return w * sc;
}
void qfDrawText(const float posX, const float posY, const char* t) {
	float x = posX, y = posY;
	while (*t) {
		if (*t == '\n') {
			x = posX;
			y -= qfSize;
			t++;
			continue;
		}
		x += qfDrawChar(x, y, *t++);
	}
}
#define qfl qfFont,(int8_t[]){0,0,0
void qfInit() {
	qfCharData qfFont = qfCreate(8, 12);
	qfAddChar(qfl,0x1f, 0,12, // full
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8,
		1, -8
	});
	qfAddChar(qfl,0x20, 0,0}); // space
	qfAddChar(qfl,0x21, 2,8, // !
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		0,
		2, 3,-2,
		2, 3,-2
	});
	qfAddChar(qfl,0x22, 2,2, // "
		4, 2,-1,1,-1,
		4, 2,-1,1,-1
	});
	qfAddChar(qfl,0x23, 2,8, // #
		4, 2,-1,2,-1,
		4, 2,-1,2,-1,
		2, 1,-6,
		4, 2,-1,2,-1,
		4, 2,-1,2,-1,
		2, 1,-6,
		4, 2,-1,2,-1,
		4, 2,-1,2,-1
	});
	qfAddChar(qfl,0x24, 1,10, // $
		2, 3,-2,
		2, 2,-4,
		4, 1,-1,4,-1,
		2, 1,-1,
		2, 2,-2,
		2, 4,-2,
		2, 6,-1,
		4, 1,-1,4,-1,
		2, 2,-4,
		2, 3,-2
	});
	qfAddChar(qfl,0x25, 2,8, // %
		4, 1,-2,2,-1,
		4, 1,-2,2,-1,
		2, 4,-1,
		2, 4,-1,
		2, 3,-1,
		2, 3,-1,
		4, 2,-1,2,-2,
		4, 2,-1,2,-2
	});
	qfAddChar(qfl,0x26, 2,8, // &
		2, 2,-3,
		4, 1,-1,3,-1,
		2, 1,-1,
		2, 2,-1,
		4, 1,-1,1,-1,
		6, 1,-1,2,-1,1,-1,
		4, 1,-1,3,-1,
		4, 2,-3,1,-1
	});
	qfAddChar(qfl,0x27, 2,2, // '
		2, 4,-1,
		2, 4,-1
	});
	qfAddChar(qfl,0x28, 2,8, // (
		2, 5,-1,
		2, 4,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 4,-1,
		2, 5,-1
	});
	qfAddChar(qfl,0x29, 2,8, // )
		2, 2,-1,
		2, 3,-1,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 3,-1,
		2, 2,-1
	});
	qfAddChar(qfl,0x2a, 2,3, // *
		4, 2,-1,2,-1,
		2, 3,-2,
		4, 2,-1,2,-1
	});
	qfAddChar(qfl,0x2b, 3,5, // +
		2, 3,-1,
		2, 3,-1,
		2, 1,-5,
		2, 3,-1,
		2, 3,-1,
	});
	qfAddChar(qfl,0x2c, 8,4, // ,
		2, 3,-2,
		2, 3,-2,
		2, 4,-1,
		2, 3,-1
	});
	qfAddChar(qfl,0x2d, 5,1,  2, 1,-6 }); // -
	qfAddChar(qfl,0x2e, 8,2, // .
		2, 3,-2,
		2, 3,-2
	});
	qfAddChar(qfl,0x2f, 2,8, // /
		2, 5,-1,
		2, 5,-1,
		2, 4,-1,
		2, 4,-1,
		2, 3,-1,
		2, 3,-1,
		2, 2,-1,
		2, 2,-1
	});
	qfAddChar(qfl,0x30, 2,8, // 0
		2, 2,-4,
		4, 1,-1,4,-1,
		4, 1,-2,3,-1,
		6, 1,-1,1,-1,2,-1,
		6, 1,-1,2,-1,1,-1,
		4, 1,-1,3,-2,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x31, 2,8, // 1
		2, 4,-1,
		2, 3,-2,
		4, 2,-1,1,-1,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 2,-5
	});
	qfAddChar(qfl,0x32, 2,8, // 2
		2, 2,-4,
		4, 1,-1,4,-1,
		2, 6,-1,
		2, 5,-1,
		2, 4,-1,
		2, 3,-1,
		2, 2,-1,
		2, 1,-6
	});
	qfAddChar(qfl,0x33, 2,8, // 3
		2, 2,-4,
		4, 1,-1,4,-1,
		2, 6,-1,
		2, 3,-3,
		2, 6,-1,
		2, 6,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x34, 2,8, // 4
		2, 4,-2,
		4, 3,-1,1,-1,
		4, 2,-1,2,-1,
		4, 1,-1,3,-1,
		2, 1,-6,
		2, 5,-1,
		2, 5,-1,
		2, 5,-1
	});
	qfAddChar(qfl,0x35, 2,8, // 5
		2, 1,-6,
		2, 1,-1,
		2, 1,-1,
		2, 1,-5,
		2, 6,-1,
		2, 6,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x36, 2,8, // 6
		2, 2,-4,
		4, 1,-1,4,-1,
		2, 1,-1,
		2, 1,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x37, 2,8, // 7
		2, 1,-6,
		2, 6,-1,
		2, 5,-1,
		2, 4,-1,
		2, 4,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1
	});
	qfAddChar(qfl,0x38, 2,8, // 8
		2, 2,-4,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-4,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x39, 2,8, // 9
		2, 2,-4,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-5,
		2, 6,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x3a, 3,6, // :
		2, 3,-2,
		2, 3,-2,
		0,
		0,
		2, 3,-2,
		2, 3,-2
	});
	qfAddChar(qfl,0x3b, 3,8, // ;
		2, 3,-2,
		2, 3,-2,
		0,
		0,
		2, 3,-2,
		2, 3,-2,
		2, 4,-1,
		2, 3,-1
	});
	qfAddChar(qfl,0x3c, 3,6, // <
		2, 5,-2,
		2, 3,-2,
		2, 1,-2,
		2, 1,-2,
		2, 3,-2,
		2, 5,-2
	});
	qfAddChar(qfl,0x3d, 4,4, // =
		2, 1,-6,
		0,
		0,
		2, 1,-6
	});
	qfAddChar(qfl,0x3e, 3,6, // >
		2, 1,-2,
		2, 3,-2,
		2, 5,-2,
		2, 5,-2,
		2, 3,-2,
		2, 1,-2
	});
	qfAddChar(qfl,0x3f, 2,8, // ?
		2, 3,-3,
		4, 2,-1,3,-1,
		2, 6,-1,
		2, 5,-1,
		2, 4,-1,
		2, 4,-1,
		0,
		2, 4,-1
	});
	qfAddChar(qfl,0x40, 2,8, // @
		2, 2,-4,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		6, 1,-1,1,-2,1,-1,
		4, 1,-1,1,-3,
		2, 1,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x41, 2,8, // A
		2, 3,-2,
		4, 2,-1,2,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-6,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1
	});
	qfAddChar(qfl,0x42, 2,8, // B
		2, 1,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-5
	});
	qfAddChar(qfl,0x43, 2,8, // C
		2, 2,-4,
		4, 1,-1,4,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x44, 2,8, // D
		2, 1,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-5
	});
	qfAddChar(qfl,0x45, 2,8, // E
		2, 1,-6,
		2, 1,-1,
		2, 1,-1,
		2, 1,-4,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-6
	});
	qfAddChar(qfl,0x46, 2,8, // F
		2, 1,-6,
		2, 1,-1,
		2, 1,-1,
		2, 1,-4,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1
	});
	qfAddChar(qfl,0x47, 2,8, // G
		2, 2,-4,
		4, 1,-1,4,-1,
		2, 1,-1,
		2, 1,-1,
		4, 1,-1,2,-3,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x48, 2,8, // H
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-6,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1
	});
	qfAddChar(qfl,0x49, 2,8, // I
		2, 1,-5,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 1,-5
	});
	qfAddChar(qfl,0x4a, 2,8, // J
		2, 1,-5,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 1,-2
	});
	qfAddChar(qfl,0x4b, 2,8, // K
		4, 1,-1,4,-1,
		4, 1,-1,3,-1,
		4, 1,-1,2,-1,
		2, 1,-3,
		4, 1,-1,2,-1,
		4, 1,-1,3,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
	});
	qfAddChar(qfl,0x4c, 2,8, // L
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-6
	});
	qfAddChar(qfl,0x4d, 2,8, // M
		4, 1,-1,4,-1,
		4, 1,-2,2,-2,
		6, 1,-1,1,-2,1,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1
	});
	qfAddChar(qfl,0x4e, 2,8, // N
		4, 1,-1,4,-1,
		4, 1,-2,3,-1,
		6, 1,-1,1,-1,2,-1,
		6, 1,-1,1,-1,2,-1,
		6, 1,-1,2,-1,1,-1,
		6, 1,-1,2,-1,1,-1,
		4, 1,-1,3,-2,
		4, 1,-1,4,-1
	});
	qfAddChar(qfl,0x4f, 2,8, // O
		2, 2,-4,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x50, 2,8, // P
		2, 1,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-5,
		2, 1,-1,
		2, 1,-1,
		2, 1,-1
	});
	qfAddChar(qfl,0x51, 2,9, // Q
		2, 2,-4,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-5,
		2, 6,-2
	});
	qfAddChar(qfl,0x52, 2,8, // R
		2, 1,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-5,
		4, 1,-1,3,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
	});
	qfAddChar(qfl,0x53, 2,8, // S
		2, 2,-4,
		4, 1,-1,4,-1,
		2, 1,-1,
		2, 2,-2,
		2, 4,-2,
		2, 6,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x54, 2,8, // T
		2, 1,-5,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
	});
	qfAddChar(qfl,0x55, 2,8, // U
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-4
	});
	qfAddChar(qfl,0x56, 2,8, // V
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 2,-1,2,-1,
		4, 2,-1,2,-1,
		4, 2,-1,2,-1,
		2, 3,-2,
		2, 3,-2
	});
	qfAddChar(qfl,0x57, 2,8, // W
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		6, 1,-1,1,-2,1,-1,
		4, 1,-2,2,-2,
		4, 1,-1,4,-1
	});
	qfAddChar(qfl,0x58, 2,8, // X
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 2,-1,2,-1,
		2, 3,-2,
		2, 3,-2,
		4, 2,-1,2,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1
	});
	qfAddChar(qfl,0x59, 2,8, // Y
		4, 1,-1,3,-1,
		4, 1,-1,3,-1,
		4, 2,-1,1,-1,
		4, 2,-1,1,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1
	});
	qfAddChar(qfl,0x5a, 2,8, // Z
		2, 1,-6,
		2, 6,-1,
		2, 5,-1,
		2, 4,-1,
		2, 3,-1,
		2, 2,-1,
		2, 1,-1,
		2, 1,-6
	});
	qfAddChar(qfl,0x5b, 2,8, // [
		2, 3,-3,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-1,
		2, 3,-3
	});
	qfAddChar(qfl,0x5c, 2,8, // \.
		2, 2,-1,
		2, 2,-1,
		2, 3,-1,
		2, 3,-1,
		2, 4,-1,
		2, 4,-1,
		2, 5,-1,
		2, 5,-1
	});
	qfAddChar(qfl,0x5d, 2,8, // ]
		2, 2,-3,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 4,-1,
		2, 2,-3
	});
	qfAddChar(qfl,0x5e, 2,2, // ^
		2, 3,-2,
		4, 2,-1,2,-1
	});
	qfAddChar(qfl,0x5f, 9,1,   2, 1,-6 }); // _
	qfAddChar(qfl,0x60, 2,2, // `
		2, 3,-1,
		2, 4,-1
	});
	qfAddChar(qfl,0x61, 4,6, // a
		2, 2,-4,
		2, 6,-1,
		2, 2,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 2,-5
	});
	qfAddChar(qfl,0x62, 2,8, // b
		2, 1,-1,
		2, 1,-1,
		2, 1,-1,
		2, 1,-5,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		4, 1,-1,4,-1,
		2, 1,-5,
	});
	/*
	qfAddChar(qfl,0x63, 2,8, // c
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x64, 2,8, // d
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x65, 2,8, // e
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x66, 2,8, // f
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x67, 2,8, // g
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x68, 2,8, // h
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x69, 2,8, // i
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x6a, 2,8, // j
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x6b, 2,8, // k
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x6c, 2,8, // l
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x6d, 2,8, // m
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x6e, 2,8, // n
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x6f, 2,8, // o
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x70, 2,8, // p
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x71, 2,8, // q
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x72, 2,8, // r
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x73, 2,8, // s
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x74, 2,8, // t
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x75, 2,8, // u
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x76, 2,8, // v
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x77, 2,8, // w
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x78, 2,8, // x
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x79, 2,8, // y
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x7a, 2,8, // z
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x7b, 2,8, // {
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x7c, 2,8, // |
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x7d, 2,8, // }
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	qfAddChar(qfl,0x7e, 2,8, // ~
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,
	});
	*/
}
