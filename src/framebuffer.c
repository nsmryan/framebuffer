#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "easing.h"
#include "randq.h"
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
TPixel colors[1024];
float totalTime = 0;
TPixel black = { 0, 0, 0, 255 };
TPixel white = { 255, 255, 255, 255 };
TPixel orange = { 255, 151, 5 };

void update(float dt);
void moveRectangles(void);
void tryNoise(float dt);

float clamp(float start, float end, float value);
float maxf(float first, float second);
float minf(float first, float second);
float fbm(float x, float y, float gain, int octaves, int hgrid);
float noise(int x, int y);
float smoothstep(float edge0, float edge1, float t);
bool within(float left, float top, float width, float height, float x, float y);
bool withinCircle(float midX, float midY, float radius, float x, float y);
int toIndex(int x, int y);
float perlin(float x, float y, float freq, float gain, int depth);
float noise2d(float x, float y);

int main(int argc, char *argv[]) {
	srandqd(0);

	screen = tigrWindow(WIDTH, HEIGHT, "Frame Buffer", TIGR_FIXED);
	tigrClear(screen, black);//tigrRGB(255, 151, 5));

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

		uint32_t color = randqd_uint32();
		//colors[index] = tigrRGB(color & 0xFF, (color & 0xFF00) >> 8, (color & 0xFF0000) >> 16);
		colors[index] = black;
	}

	while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
		float dt = tigrTime();
        //printf("dt = %.3f\n", dt);
		update(dt);
		tigrUpdate(screen);
		//tigrClear(screen, tigrRGB(255, 151, 5));
        tigrClear(screen, black);//tigrRGB(255, 151, 5));
	}

	tigrFree(screen);
}

void update(float dt) {
	totalTime += dt;

	tryNoise(dt);
	//moveRectangles();
}

void tryNoise(float dt) {
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			float offset = totalTime * 10;
			float speed = 0;
			float newX = x;
			float newY = y;

			float gain = 0.05;
			float freq = 0.10;
			int octaves = 1;

			float value = 0;

			float scale = 15;
			value = perlin(x + offset, y + offset, freq, gain, octaves) * 20;
			value = perlin(x + value + speed, y + value + speed, freq, gain, octaves);
			newX += scale * value;

            value = 0;
			value = perlin(x + offset * 2, y + offset * 2, freq, gain, octaves) * 20;
			value = perlin(x + value + speed, y + value + speed, freq, gain, octaves);
			newY += scale * value;

			float dist = 1.0 - sqrt(fabs(((x - newX)) + ((y - newY)))) / scale;
			TPixel color = tigrRGB(clamp(0, 255, orange.r * dist), clamp(0, 255, orange.g * dist), clamp(0, 255, orange.b * dist));

			//if (within(10, 10, WIDTH - 10, HEIGHT - 10, newX, newY)) {
			if (withinCircle(WIDTH/2, HEIGHT/2, WIDTH / 2.4, newX, newY)) {
				int index = toIndex(x, y);
				screen->pix[index] = color;
			}
		}
	}
}

int toIndex(int x, int y) {
	return x + y * WIDTH;
}

void moveRectangles(void) {
	float rate = clamp(0.0, 1.0, totalTime * SPEED);
	for (uint32_t index = 0; index < numPositions; index++) {
		float xDist = targetPositions[index].x - positions[index].x;
		float x = positions[index].x + (CubicEaseInOut(rate) * xDist);

		float yDist = targetPositions[index].y - positions[index].y;
		float y = positions[index].y + (CubicEaseInOut(rate) * yDist);

		float size = QuadraticEaseOut(rate) * SIZE;
		tigrFillRect(screen, x, y, size, size, colors[index]);
		tigrRect(screen, x, y, size, size, white);
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

float noise(int x, int y) {
	int n;

	n = x + y * 57;
	n = (n << 13) ^ n;
	return (1.0 - ( (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff) / 1073741824.0);
}

float fbm(float x, float y, float gain, int octaves, int hgrid) {
	int i;
	float total = 0.0f;
	float frequency = 1.0f/(float)hgrid;
	float amplitude = gain;
	float lacunarity = 2.0;

	for (i = 0; i < octaves; ++i)
	{
		total += noise((float)x * frequency, (float)y * frequency) * amplitude;         
		frequency *= lacunarity;
		amplitude *= gain;
	} 

	return (total);
}

float smoothstep(float edge0, float edge1, float x) {
    float t = clamp(0.0, 1.0, (x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

bool within(float left, float top, float width, float height, float x, float y) {
	return (x >= left && x < (left + width)) && (y >= top && y < (top + height));
}

// FROM https://gist.github.com/nowl/828013
static const int  SEED = 1985;

static const unsigned char  HASH[] = {
    208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
    185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
    9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
    70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
    203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
    164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
    228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
    232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
    193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
    101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
    135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
    114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219
};

int noise2(int x, int y)
{
    int  yindex = (y + SEED) % 256;
    if (yindex < 0)
        yindex += 256;
    int  xindex = (HASH[yindex] + x) % 256;
    if (xindex < 0)
        xindex += 256;
    const int  result = HASH[xindex];
    return result;
}

static float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

static float smooth_inter(float x, float y, float s)
{
    return lin_inter( x, y, s * s * (3-2*s) );
}

float noise2d(float x, float y)
{
    const int  x_int = floor( x );
    const int  y_int = floor( y );
    const float  x_frac = x - x_int;
    const float  y_frac = y - y_int;
    const int  s = noise2( x_int, y_int );
    const int  t = noise2( x_int+1, y_int );
    const int  u = noise2( x_int, y_int+1 );
    const int  v = noise2( x_int+1, y_int+1 );
    const float  low = smooth_inter( s, t, x_frac );
    const float  high = smooth_inter( u, v, x_frac );
    const float  result = smooth_inter( low, high, y_frac );
    return result;
}

float perlin(float x, float y, float freq, float gain, int depth)
{
    float  xa = x*freq;
    float  ya = y*freq;
    float  amp = 1.0;
    float  fin = 0;
    float  div = 0.0;
    for (int i=0; i<depth; i++)
    {
        div += 256 * amp;
        fin += noise2d( xa, ya ) * amp;
        amp *= gain;
        xa *= 2;
        ya *= 2;
    }
    return fin/div;
}
// END from https://gist.github.com/nowl/828013

bool withinCircle(float midX, float midY, float radius, float x, float y) {
    float xDiff = x - midX;
    float yDiff = y - midY;
    return fabs(xDiff * xDiff + yDiff * yDiff) <= (radius * radius);
}
