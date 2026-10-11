#include <stdint.h>
#include "qfont.h"

static const qFont _qFont = {
	.sizeX = 8,
	.sizeY = 11,
	.chars = 0
};

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

qFont qfCreate(const uint8_t sizeX, const uint8_t sizeY, const uint32_t capacity) { return (qFont){
	.sizeX = sizeX,
	.sizeY = sizeY,
	.charCount = capacity,
	.chars = 0
};}
