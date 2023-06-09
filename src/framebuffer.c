#include <stdio.h>
#include <stdint.h>

#include "easing.h"
#include "tigr.h"


#define WIDTH 512
#define HEIGHT 512

#define SIZE 20U
#define SEP (SIZE * 2U)
#define SPEED 0.3


typedef struct Pos {
    float x;
    float y;
} Pos;

Tigr *screen;
uint32_t numPositions;
Pos positions[1024];
Pos targetPositions[1024];
float travelTime = 0;
TPixel black = { 0, 0, 0, 255 };

void update(float dt);
float clamp(float start, float end, float value);
float maxf(float first, float second);
float minf(float first, float second);

int main(int argc, char *argv[]) {
    screen = tigrWindow(WIDTH, HEIGHT, "Frame Buffer", TIGR_FIXED);
    tigrClear(screen, tigrRGB(255, 151, 5));

    numPositions = 10 * 10;
	float xOffset = 5 * SEP;
	float yOffset = 5 * SEP;
    for (uint32_t index = 0; index < numPositions; index++) {
        uint32_t x = index % 10;
        uint32_t y = index / 10;
        targetPositions[index].x = x * SEP + (WIDTH / 2) - xOffset;
        targetPositions[index].y = y * SEP + (HEIGHT / 2) - yOffset;
        positions[index].x = WIDTH / 2;
        positions[index].y = HEIGHT / 2;
    }

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
        float dt = tigrTime();
        update(dt);
        tigrUpdate(screen);
		tigrClear(screen, tigrRGB(255, 151, 5));
    }

    tigrFree(screen);
}

void update(float dt) {
    travelTime += dt;

	float rate = clamp(0.0, 1.0, travelTime * SPEED);
    for (uint32_t index = 0; index < numPositions; index++) {
		float xDist = targetPositions[index].x - positions[index].x;
        float x = positions[index].x + (CubicEaseInOut(rate) * xDist);

		float yDist = targetPositions[index].y - positions[index].y;
        float y = positions[index].y + (CubicEaseInOut(rate) * yDist);

		tigrFillRect(screen, x, y, SIZE, SIZE, black);
    }
}

float clamp(float start, float end, float value) {
	return minf(maxf(value, start), end);
}

float maxf(float first, float second) {
	return first > second ? first : second;
}

float minf(float first, float second) {
	return first < second ? first : second;
}
