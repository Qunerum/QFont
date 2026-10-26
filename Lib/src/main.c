// #define QGPU_COLORS
#define QGPU_SHAPES
#include "../lib/qgpu.h"

#include "../lib/qfont.h"

void draw(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a) { qgAddRect((Vector2){posX,posY},(Vector2){sizeX,sizeY},(ColorRGBA){r,g,b,a}); }
void init() {
	qgSetBackground(CLR_RGB(0.05f, 0.05f, 0.08f));
	qfSetDrawRect(0,0, QFONT_Y_UP, draw);
	qfInit();
	qfReadFile("font.qf");
}
const float size = 32;
void update() {
	uint w = qgGetWidth(), h = qgGetHeight();
	int hw = w / 2, hh = h / 2;
	qfConfigure(size, QFONT_REGULAR, 1, 1, 1, 1);
	qfDrawText(-hw + 10, hh - 10,
		"\x1F!\"#$%&'()*+,-./\n"
		"0123456789:;<=>?@\n"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
		"[\\]^_`\n"
		"abcdefghijklmnopqrstuvwxyz\n"
		"{|}~"); // use: \33xHEXq or \33dDECIMALq for custom chars
	float y = 36 * 7;
	qfConfigure(size, QFONT_REGULAR, 1, 1, 1, 1);
	qfDrawText(-hw + 10, -hh + y, "Regular:       Hello!");
	qfConfigure(size, QFONT_BOLD, 1, 1, 1, 1);
	qfDrawText(-hw + 10, -hh + (y -= 36), "Bold:          Hello!");
	qfConfigure(size, QFONT_ITALIC, 1, 1, 1, 1);
	qfDrawText(-hw + 10, -hh + (y -= 36), "Italic:        Hello!");
	qfConfigure(size, QFONT_UNDERLINE, 1, 1, 1, 1);
	qfDrawText(-hw + 10, -hh + (y -= 36), "Underline:     Hello!");
	qfConfigure(size, QFONT_STRIKETHROUGH, 1, 1, 1, 1);
	qfDrawText(-hw + 10, -hh + (y -= 36), "Strikethrough: Hello!");
	qfConfigure(size, QFONT_UPPERCASE, 1, 1, 1, 1);
	qfDrawText(-hw + 10, -hh + (y -= 36), "Uppercase:     Hello!");
	qfConfigure(size, QFONT_LOWERCASE, 1, 1, 1, 1);
	qfDrawText(-hw + 10, -hh + (y -= 36), "Lowercase:     Hello!");
}

int main() {
	int qgpu =  qgpuCreate(680, 720, "QFont tests", init, update);
	qfEnd();
	return qgpu;
}

