// #define QGPU_COLORS
#define QGPU_SHAPES
#include "../lib/qgpu.h"

void init() {
	qgSetBackground(CLR_RGB(0.05f, 0.05f, 0.08f));
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
}

int main() { return qgpuCreate(680, 720, "QFont Maker", init, update); }
