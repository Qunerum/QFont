// #define QGPU_COLORS
#define QGPU_SHAPES
#include "../lib/qgpu.h"

#include "../lib/qfont.h"

void draw(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a) { qgAddRect((Vector2){posX,posY},(Vector2){sizeX,sizeY},(ColorRGBA){r,g,b,a}); }
void init() {
	qgSetBackground(CLR_RGB(0.05f, 0.05f, 0.08f));
	qfSetDrawRect(0,0, draw);
	qfInit();
	qfWriteFile(0,"test.qf", (qfCharData){8,12},(int8_t[]){0,0,0,0x1e, 2,8, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 0, 2, 3,-2, 2, 3,-2});
	qfReadFile("test.qf");
	qfReadFile("font.qf");
	qfConfigure(32, QFONT_REGULAR, 1, 1, 1, 1);
}
void update() {
	uint w = qgGetWidth(), h = qgGetHeight();
	int hw = w / 2, hh = h / 2;
	qfDrawText(-hw + 10, hh - 10,
		"\x1F!\"#$%&'()*+,-./\n"
		"0123456789:;<=>?@\n"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
		"[\\]^_`\n"
		"abcdefghijklmnopqrstuvwxyz\n"
		"{|}~\n\x1e\x7F\x80");
}

int main() { return qgpuCreate(680, 720, "QFont tests", init, update); }


