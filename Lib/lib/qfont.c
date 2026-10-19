#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "qfont.h"

#define NKO "\033[1;38;5;240m[\033[1;38;5;"
#define NKC "\033[1;38;5;240m]\033[1;38;5;251m"
#define QFONT_PREFIX NKO "211mQFont" NKC

static float
	pivX = 0.0f, pivY = 0.0f,
	qfSize = 32.0f,
	qfR = 1.0f, qfG = 1.0f, qfB = 1.0f, qfA = 1.0f;
static uint8_t qfStyle = QFONT_REGULAR, qfAxis = QFONT_Y_UP;
static void (*drawFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a);
void qfSetDrawRect(const int8_t px, const int8_t py, const uint8_t ya, void (*drawRectFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a)) {
	pivX = px <= -1 ? 0.0f : px >= 1 ? 1.0f : 0.5f;
	pivY = py <= -1 ? 0.0f : py >= 1 ? 1.0f : 0.5f;
	qfAxis = ya;
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
static int8_t** allChars = 0, isFile = 0;
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
	const uint8_t c = isFile ? 12 : 245;
	printf(QFONT_PREFIX "%c" NKO "245m%4u" NKC
		" %s " NKO "%um%c" NKC
		": WH:" NKO "%um%ux%u" NKC
		" ID:" NKO "%um0x%08X" NKC
		" LS:" NKO "%um%4uB" NKC "\n",
		isFile ? 0 : ' ', charCount, isFile ? "Readed char" : "Added char", c, (char)(id > 31 && id < 127 ? id : '?'), c, charData.sizeX, charData.sizeY, c, id, c, size);
}
void qfWriteFile(const uint8_t add, const char* path, const qfCharData charData, const int8_t* d) {
	if (!d) return;
	FILE* f = fopen(path, add ? "ab" : "wb");
	if (!f) return;
	const uint32_t id =
	(((uint32_t)u8d[0] << 24)) |
	(((uint32_t)u8d[1] << 16)) |
	(((uint32_t)u8d[2] << 8))  |
	((uint32_t)u8d[3]);
	const uint8_t startAt = u8d[4], goNext = u8d[5];
	uint16_t l = 0, actualID = 6;
	for (uint8_t i = 0; i < goNext; i++) {
		const uint8_t len = 1 + u8d[actualID];
		actualID += len;
		l += len;
	}
	fwrite(&d[0], 1, 1, f);
	fwrite(&d[1], 1, 1, f);
	fwrite(&d[2], 1, 1, f);
	fwrite(&d[3], 1, 1, f);
	fwrite(&charData.sizeX, 1, 1, f);
	fwrite(&charData.sizeY, 1, 1, f);
	fwrite(&startAt, 1, 1, f);
	fwrite(&goNext, 1, 1, f);
	fwrite(&d[6], 1, l, f);
	fclose(f);
	const uint16_t totalRecordSize = 6 + l;
	printf(QFONT_PREFIX
	"   Saved to file   " NKO "115m%c" NKC
	": WH:" NKO "115m%ux%u" NKC
	" ID:" NKO "115m0x%08X" NKC
	" LS:" NKO "115m%4uB" NKC "\n",
	(char)(id > 31 && id < 127 ? id : '?'), charData.sizeX, charData.sizeY, id, totalRecordSize);
}
void qfReadFile(const char* path) {
	FILE* f = fopen(path, "rb");
	if (!f) return;
	isFile = 1;
	uint32_t id = 0;
	while (fread(&id, sizeof(uint32_t), 1, f) == 1) {
		uint8_t sizeX = 0, sizeY = 0, startAt = 0, goNext = 0;
		if (fread(&sizeX, 1, 1, f) != 1 ||
			fread(&sizeY, 1, 1, f) != 1 ||
			fread(&startAt, 1, 1, f) != 1 ||
			fread(&goNext, 1, 1, f) != 1) break;
		const qfCharData cd = (qfCharData){ .sizeX = sizeX, .sizeY = sizeY };
		uint16_t size = 6;
		for (uint8_t i = 0; i < goNext; i++) {
			uint8_t v = 0;
			if (fread(&v, 1, 1, f) != 1) break;
			fseek(f, (long)v, SEEK_CUR);
			size += (1 + v);
		}
		int8_t* d = (int8_t*)malloc(size);
		if (!d) {
			fclose(f);
			return;
		}
		fseek(f, -(long)(size + 2), SEEK_CUR);
		d[3] = (int8_t)(id >> 24);
		d[2] = (int8_t)(id >> 16);
		d[1] = (int8_t)(id >> 8);
		d[0] = (int8_t)id;
		fseek(f, 6, SEEK_CUR);
		fread(&d[4], 1, size - 4, f);
		qfAddChar(cd, d);
	}
	isFile = 0;
	fclose(f);
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
			const float pyt = (y + pivY) * sc,
						py = qfAxis ? posY + pyt : posY - pyt;
			if (draw) drawFunc(posX + (x + v * pivX) * sc, py, v * sc, sc, qfR,qfG,qfB,qfA);
			x += v;
		}
		i += len;
		x = 0;
		y++;
	}
	return w * sc;
}
static uint8_t tmpi = 0;
static char tmp[11];
void qfDrawText(const float posX, const float posY, const char* t) {
	float x = posX, y = posY;
	while (*t) {
		if (*t == '\n') {
			x = posX;
			y -= qfSize;
			t++;
			continue;
		}
		if (*t == '\33') {
			t++;
			const uint8_t type = *t == 'd' ? 1 : *t == 'x' ? 2 : 0;
			if (!type || !*t) continue;
			tmpi = 0;
			tmp[0] = 0;
			for (uint8_t i = 0; i < sizeof(tmp) - 1; i++) {
				t++;
				if (!*t) return;
				if (*t != 'q') tmp[tmpi++] = *t; else break;
			}
			uint32_t res = 0;
			uint8_t i = 0;
			if (type == 1) {
				while (tmp[i] >= '0' && tmp[i] <= '9') {
					res = res * 10 + tmp[i] - '0';
					i++;
				}
			} else {
				for (i = 0; i < tmpi; i++) if (tmp[i] >= 'A' && tmp[i] <= 'F') tmp[i] += 32;
				i = 0;
				while (i < tmpi) {
					const char c = tmp[i];
					uint32_t val = 0;
					if (c >= '0' && c <= '9') val = c - '0';
					else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
					else break;
					res = (res << 4) | val;
					i++;
				}
			}
			x += qfDrawChar(x, y, res);
			t++;
			continue;
		}
		x += qfDrawChar(x, y, (unsigned char)*t++);
	}
}
#define qfl qfFont,(int8_t[]){0,0,0
void qfInit() {
	qfCharData qfFont = qfCreate(8, 12);
	qfAddChar(qfl,0x7e, 0,0}); // alloc
	charCount--;
	qfAddChar(qfl,0x1f, 0,12, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8});
	qfAddChar(qfl,0x20, 0,0});
	qfAddChar(qfl,0x21, 2,8, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 0, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x22, 2,2, 4, 2,-1,1,-1, 4, 2,-1,1,-1});
	qfAddChar(qfl,0x23, 2,8, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 1,-6, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 1,-6, 4, 2,-1,2,-1, 4, 2,-1,2,-1});
	qfAddChar(qfl,0x24, 1,10, 2, 3,-2, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 2,-2, 2, 4,-2, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4, 2, 3,-2});
	qfAddChar(qfl,0x25, 2,8, 4, 1,-2,2,-1, 4, 1,-2,2,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 4, 2,-1,2,-2, 4, 2,-1,2,-2});
	qfAddChar(qfl,0x26, 2,8, 2, 2,-3, 4, 1,-1,3,-1, 2, 1,-1, 2, 2,-1, 4, 1,-1,1,-1, 6, 1,-1,2,-1,1,-1, 4, 1,-1,3,-1, 4, 2,-3,1,-1});
	qfAddChar(qfl,0x27, 2,2, 2, 4,-1, 2, 4,-1});
	qfAddChar(qfl,0x28, 2,8, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 4,-1, 2, 5,-1});
	qfAddChar(qfl,0x29, 2,8, 2, 2,-1, 2, 3,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1});
	qfAddChar(qfl,0x2a, 2,3, 4, 2,-1,2,-1, 2, 3,-2, 4, 2,-1,2,-1});
	qfAddChar(qfl,0x2b, 3,5, 2, 3,-1, 2, 3,-1, 2, 1,-5, 2, 3,-1, 2, 3,-1,});
	qfAddChar(qfl,0x2c, 8,4, 2, 3,-2, 2, 3,-2, 2, 4,-1, 2, 3,-1});
	qfAddChar(qfl,0x2d, 5,1,  2, 1,-6 });
	qfAddChar(qfl,0x2e, 8,2, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x2f, 2,8, 2, 5,-1, 2, 5,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 2, 2,-1, 2, 2,-1});
	qfAddChar(qfl,0x30, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-2,3,-1, 6, 1,-1,1,-1,2,-1, 6, 1,-1,2,-1,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x31, 2,8, 2, 4,-1, 2, 3,-2, 4, 2,-1,1,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 2,-5});
	qfAddChar(qfl,0x32, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1, 2, 1,-6});
	qfAddChar(qfl,0x33, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 6,-1, 2, 3,-3, 2, 6,-1, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x34, 2,8, 2, 4,-2, 4, 3,-1,1,-1, 4, 2,-1,2,-1, 4, 1,-1,3,-1, 2, 1,-6, 2, 5,-1, 2, 5,-1, 2, 5,-1});
	qfAddChar(qfl,0x35, 2,8, 2, 1,-6, 2, 1,-1, 2, 1,-1, 2, 1,-5, 2, 6,-1, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x36, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x37, 2,8, 2, 1,-6, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1});
	qfAddChar(qfl,0x38, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x39, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x3a, 3,6, 2, 3,-2, 2, 3,-2, 0, 0, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x3b, 3,8, 2, 3,-2, 2, 3,-2, 0, 0, 2, 3,-2, 2, 3,-2, 2, 4,-1, 2, 3,-1});
	qfAddChar(qfl,0x3c, 3,6, 2, 5,-2, 2, 3,-2, 2, 1,-2, 2, 1,-2, 2, 3,-2, 2, 5,-2});
	qfAddChar(qfl,0x3d, 4,4, 2, 1,-6, 0, 0, 2, 1,-6});
	qfAddChar(qfl,0x3e, 3,6, 2, 1,-2, 2, 3,-2, 2, 5,-2, 2, 5,-2, 2, 3,-2, 2, 1,-2});
	qfAddChar(qfl,0x3f, 2,8, 2, 3,-3, 4, 2,-1,3,-1, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 4,-1, 0, 2, 4,-1});
	qfAddChar(qfl,0x40, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 6, 1,-1,1,-2,1,-1, 4, 1,-1,1,-3, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x41, 2,8, 2, 3,-2, 4, 2,-1,2,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x42, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5});
	qfAddChar(qfl,0x43, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x44, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5});
	qfAddChar(qfl,0x45, 2,8, 2, 1,-6, 2, 1,-1, 2, 1,-1, 2, 1,-4, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-6});
	qfAddChar(qfl,0x46, 2,8, 2, 1,-6, 2, 1,-1, 2, 1,-1, 2, 1,-4, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x47, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-1, 4, 1,-1,2,-3, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x48, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x49, 2,8, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-5});
	qfAddChar(qfl,0x4a, 2,8, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-2});
	qfAddChar(qfl,0x4b, 2,8, 4, 1,-1,4,-1, 4, 1,-1,3,-1, 4, 1,-1,2,-1, 2, 1,-3, 4, 1,-1,2,-1, 4, 1,-1,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1,});
	qfAddChar(qfl,0x4c, 2,8, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-6});
	qfAddChar(qfl,0x4d, 2,8, 4, 1,-1,4,-1, 4, 1,-2,2,-2, 6, 1,-1,1,-2,1,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x4e, 2,8, 4, 1,-1,4,-1, 4, 1,-2,3,-1, 6, 1,-1,1,-1,2,-1, 6, 1,-1,1,-1,2,-1, 6, 1,-1,2,-1,1,-1, 6, 1,-1,2,-1,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x4f, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x50, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 2, 1,-1, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x51, 2,9, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-2});
	qfAddChar(qfl,0x52, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 4, 1,-1,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1,});
	qfAddChar(qfl,0x53, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 2,-2, 2, 4,-2, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x54, 2,8, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1,});
	qfAddChar(qfl,0x55, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x56, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x57, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 6, 1,-1,1,-2,1,-1, 4, 1,-2,2,-2, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x58, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2, 4, 2,-1,2,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x59, 2,8, 4, 1,-1,3,-1, 4, 1,-1,3,-1, 4, 2,-1,1,-1, 4, 2,-1,1,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1});
	qfAddChar(qfl,0x5a, 2,8, 2, 1,-6, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1, 2, 1,-1, 2, 1,-6});
	qfAddChar(qfl,0x5b, 2,8, 2, 3,-3, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-3});
	qfAddChar(qfl,0x5c, 2,8, 2, 2,-1, 2, 2,-1, 2, 3,-1, 2, 3,-1, 2, 4,-1, 2, 4,-1, 2, 5,-1, 2, 5,-1});
	qfAddChar(qfl,0x5d, 2,8, 2, 2,-3, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 2,-3});
	qfAddChar(qfl,0x5e, 2,2, 2, 3,-2, 4, 2,-1,2,-1});
	qfAddChar(qfl,0x5f, 9,1,   2, 1,-6 });
	qfAddChar(qfl,0x60, 2,2, 2, 3,-1, 2, 4,-1});
	qfAddChar(qfl,0x61, 4,6, 2, 2,-4, 2, 6,-1, 2, 2,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5});
	qfAddChar(qfl,0x62, 2,8, 2, 1,-1, 2, 1,-1, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5,});
	qfAddChar(qfl,0x63, 4,6, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x64, 2,8, 2, 6,-1, 2, 6,-1, 4, 2,-3,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5});
	qfAddChar(qfl,0x65, 4,6, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-6, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x66, 2,8, 2, 4,-2, 2, 3,-1, 2, 3,-1, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1});
	qfAddChar(qfl,0x67, 4,8, 4, 2,-3,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x68, 2,8, 2, 1,-1, 2, 1,-1, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x69, 2,8, 2, 3,-1, 2, 3,-1, 0, 2, 2,-2, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-5});
	qfAddChar(qfl,0x6a, 2,10, 2, 4,-1, 2, 4,-1, 0, 2, 3,-2, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 4, 2,-1,1,-1, 2, 3,-1});
	qfAddChar(qfl,0x6b, 2,8, 2, 1,-1, 2, 1,-1, 4, 1,-1,4,-1, 4, 1,-1,3,-1, 2, 1,-4, 4, 1,-1,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x6c, 2,8, 2, 2,-2, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-5});
	qfAddChar(qfl,0x6d, 4,6, 3, -3,1,-2, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1});
	qfAddChar(qfl,0x6e, 4,6, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x6f, 4,6, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x70, 4,8, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x71, 4,8, 4, 2,-3,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-1, 2, 6,-1});
	qfAddChar(qfl,0x72, 4,6, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x73, 4,6, 2, 2,-5, 2, 1,-1, 2, 2,-4, 2, 6,-1, 2, 6,-1, 2, 1,-5});
	qfAddChar(qfl,0x74, 2,8, 2, 3,-1, 2, 3,-1, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 4,-2});
	qfAddChar(qfl,0x75, 4,6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,3,-2, 4, 2,-3,1,-1});
	qfAddChar(qfl,0x76, 4,6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x77, 4,6, 3, -1,5,-1, 3, -1,5,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 4, 1,-2,1,-2});
	qfAddChar(qfl,0x78, 4,6, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2, 4, 2,-1,2,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x79, 4,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,3,-2, 4, 2,-3,1,-1, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x7a, 4,6, 2, 1,-6, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1, 2, 1,-6});
	qfAddChar(qfl,0x7b, 2,8, 2, 5,-1, 2, 4,-1, 2, 4,-1, 2, 3,-2, 2, 3,-2, 2, 4,-1, 2, 4,-1, 2, 5,-1});
	qfAddChar(qfl,0x7c, 0,12, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x7d, 2,8, 2, 2,-1, 2, 3,-1, 2, 3,-1, 2, 3,-2, 2, 3,-2, 2, 3,-1, 2, 3,-1, 2, 2,-1});
	qfAddChar(qfl,0x7e, 5,2, 4, 2,-2,2,-1, 4, 1,-1,2,-2});
}
