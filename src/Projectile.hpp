#pragma once

#include <cstring>
#include <cstdlib>
#include "Vector2d.hpp"
#include "logger/logger.h"

struct Image
{
	unsigned int * img_matrix = NULL;
	unsigned int width = 0;
	unsigned int height = 0;

	char filename[128] = {'\0'};
	Image(const char * _filename) {
		int str_size = strlen(_filename) + 1;
		if (str_size > 128) PSP_LOGGER::psp_log(PSP_LOGGER::CRITICAL, "Filename"
		" %s too long", _filename);
		strncpy(filename, _filename, str_size);
	}

	Image(unsigned int _height, unsigned int _width, unsigned int * _img_matrix, const char * _filename) {
		height = _height;
		width = _width;
		img_matrix = _img_matrix;

		int str_size = strlen(_filename) + 1;
		if (str_size > 128) PSP_LOGGER::psp_log(PSP_LOGGER::CRITICAL, "Filename"
		" %s too long", _filename);
		strncpy(filename, _filename, str_size-1);
	}

	Image() {}

	~Image() {
	}
};

struct Animation
{
	private:
		unsigned int next_frame = 0;
	public:
	// Program updated variables
	unsigned int * * img_matrices = NULL;
	unsigned int last_updated = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	bool animate = false;
	bool looping =  false;

	// User provided variables 
	unsigned int frame_time = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	char filename[128] = {'\0'};

	Image get_frame(int index) {
		PSP_LOGGER::assert_or_log(index < rows*cols, "get_frame(index) "
		"requested an in bounds value of %d for size %d", index, rows * cols);

		return(Image(height, width, img_matrices[index], "."));
	}

	Image get_next_frame(int time, int repetitions){
		int curr_frame = next_frame;

		if (time > (last_updated + frame_time)){
			next_frame++;
			next_frame%=(rows * cols);
			last_updated = time;

			if (curr_frame > rows*cols -2) repetitions--;
			if (!repetitions) animate = false;			
		}
		return(Image(height, width, img_matrices[curr_frame], ".."));
	}

	void reset() {
		next_frame = 0;
	}

	Animation(unsigned int _rows, unsigned int _cols, unsigned int _frame_time, const char * _filename) {
		frame_time = _frame_time;
		
		int str_size = strlen(_filename) + 1;
		if (str_size > 128) PSP_LOGGER::psp_log(PSP_LOGGER::CRITICAL, "Filename"
		" %s too long", _filename);
		strncpy(filename, _filename, str_size-1);

		rows = _rows;
		cols = _cols;
	}

	~Animation() {
	}
};


class Object
{
	public:
	float grav = 60.0f;
	Image image;
	int draw_pos_x; // postion to draw at

	Vector2d vector;
	Object() {}
	Object(Image _img) {
		image = _img;
	}

	bool off_screen() {
		return draw_pos_x > 480 + 50 || draw_pos_x < -50
		|| vector.y > 512 + 50 || vector.y < -50;
	}
};

class Person: public Object 
{
	public:
	Object weapon = Object(Image("assets/player_rocket.bmp"));
	
	bool jumping = false;
	unsigned int jump_time;
	int jump_height = 0;
	int starting_jump_height;

	Person(Image _img) {
		image = _img;
	}

	unsigned short jump_height_at(float time) {
		time *=2;
		return starting_jump_height + vector.vel_y*(time) 
		       + .5 * (grav * (time) * (time) );		
	}

} ;



