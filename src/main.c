// #define QGPU_COLORS
#define QGPU_SHAPES
#include "../lib/qgpu.h"

#include "../lib/qfont.h"

void draw(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a) { qgAddRect((Vector2){posX,posY},(Vector2){sizeX,sizeY},(ColorRGBA){r,g,b,a}); }
void init() {
	qgSetBackground(CLR_RGB(0.05f, 0.05f, 0.08f));
	qfSetDrawRect(0,0, draw);
	qfInit();
	qfConfigure(48, QFONT_REGULAR, 1, 1, 1, 1);
}
static uint btn = 0;
void drawBtn(const int hw, const int hh) {
	qgAddRect((Vector2){-hw + 75, hh - 20}, (Vector2){140, 30}, CLR_RGBA(.26f,.26f,.26f,1.0f));
	qgAddText((Vector2){-hw + 10, hh - 10}, "Button");
}
void update() {
	uint w = qgGetWidth(), h = qgGetHeight();
	int hw = w / 2, hh = h / 2;
	qgSetLayerUI(0);
	qgAddRect((Vector2){0, hh - 40}, (Vector2){w, 80}, CLR_RGBA(.2f,.2f,.2f,1.0f));
	btn = 0;
	qgSetLayerUI(1);
	drawBtn(hw,hh);

	qfDrawText(-hw + 10, 0,
		"\x1F!\"#$%&'()*+,-./0123456789:;<=>?@\n"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
		"[\\]^_` abcdefghijklmnopqrstuvwxyz {|}~\n");
}

int main() { return qgpuCreate(680, 720, "QFont Maker", init, update); }
