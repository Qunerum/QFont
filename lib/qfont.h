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

typedef struct { uint8_t sizeX, sizeY; } qfCharData;

void qfSetDrawRect(const int8_t pivotX, const int8_t pivotY, void (*drawRectFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a));
void qfConfigure(const float fontSize, const uint8_t style, const float r, const float g, const float b, const float a);
qfCharData qfCreate(const uint8_t sizeX, const uint8_t sizeY);
void qfAddChar(const qfCharData charData, const int8_t* data);
void qfWriteFile(uint8_t add, const char* path, const qfCharData charData, const int8_t* d);
void qfReadFile(const char* path);
float qfDrawChar(const float posX, const float posY, const uint32_t c);
void qfDrawText(const float posX, const float posY, const char* text);
void qfInit();

#endif
