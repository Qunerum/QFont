#include <stdint.h>
#include <stdlib.h>
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

qFont* qfCreate(const uint8_t sizeX, const uint8_t sizeY, const uint32_t capacity) {
	qFont* font = malloc(sizeof(qFont));
	font->sizeX = sizeX;
	font->sizeY = sizeY;
	font->capacity = capacity;
	font->charCount = 0;
	font->chars = 0;
	return font;
}
void qfAddChar(qFont* font, int8_t* data) {
	if (!font || !data || font->charCount == 0 || font->capacity == 0 || font->charCount >= font->capacity) return;

}
static qFont* qfMain = 0;
#define qfac_ qfAddChar(qfMain,(int8_t[]){
void qfInit() {
	qfMain = qfCreate(8, 12, 128);
	// a b c d e f g h i j k l m n o p q r s t u v w x y z - 26
	// A B C D E F G H I J K L M N O P Q R S T U V W X Y Z - 26
	// ` 1 2 3 4 5 6 7 8 9 0 - =                           - 13
	// ~ ! @ # $ % ^ & * ( ) _ +                           - 13
	// [ ] \   { } |                                       - 6
	// ; '     : "                                         - 4
	// , . /   < > ?                                       - 6
	qfac_'!',2,8,
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		2, 3,-2,
		0,
		2, 3,-2,
		2, 3,-2
	});
}
