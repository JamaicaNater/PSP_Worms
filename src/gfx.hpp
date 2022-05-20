#pragma once

#include <cstdint>

#define SCREEN_HEIGHT 272
#define SCREEN_WIDTH_RES 480
#define SCREEN_WIDTH 512
#define PI 3.14159265359

enum flips{FORWARD, BACKWARD};

namespace GFX 
{
	void init();
	void clear(uint32_t color);
	void swapBuffers();
	void drawTerrain(unsigned char noise[], int cam_pos_x);
	void drawBMP(int x, int y, short rot, char direction, const char* filename, uint32_t filter, unsigned int * &image);
	void populate_trig_tables();
	void do_homescreen();
}