#ifndef SET_MAP_FORMAT_HPP
#define SET_MAP_FORMAT_HPP

struct Set_Map_Header {
	int width;
	int height;
	int max_iterations;
	double diverged_magnitude;
};

struct Set_Map_Element {
	int iterations;
};

#endif