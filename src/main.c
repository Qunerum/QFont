// #define QGPU_COLORS
#define QGPU_SHAPES
#include "../lib/qgpu.h"

void init() {
	qgSetBackground(CLR_RGB(0.05f, 0.05f, 0.08f));
}

void update() {
	uint w = qgGetWidth(), h = qgGetHeight(),
		hw = w / 2, hh = h / 2;
	qgAddRect((Vector2){0, hh - 40}, (Vector2){w, 80}, CLR_RGBA(.2f, .2f, .2f, 1.0f));
}

int main() { return qgpuCreate(680, 720, "QFont Maker", init, update); }
