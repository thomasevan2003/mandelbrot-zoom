#include <iostream>
#include <cmath>
#include <omp.h>
#include "Stopwatch.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include "set_map_format.hpp"
#include "fixedpoint1024.hpp"

constexpr int IMAGE_WIDTH = 192/4;
constexpr int IMAGE_HEIGHT = 108/4;
constexpr double HEIGHT = 2.0;
constexpr double WIDTH = HEIGHT*static_cast<double>(IMAGE_WIDTH)/static_cast<double>(IMAGE_HEIGHT);
constexpr double DIVERGED_MAGNITUDE = 2.0;
constexpr int MAX_ZOOM_EXPONENT = 128;
constexpr int INITIAL_MAX_ITERATIONS = 1000;
constexpr int MAX_ITERATIONS_SLOPE = 1000;
constexpr std::string_view cx_guess = "-0.74453986035590838011";
constexpr std::string_view cy_guess = "0.12172377389442482241";

int main() {
	std::cout << "start of main" << std::endl;
	Set_Map_Element* setmap = new Set_Map_Element[IMAGE_WIDTH*IMAGE_HEIGHT];
	if (!std::filesystem::exists("base-output")) {
		if (!std::filesystem::create_directory("base-output")) {
			std::cout << "Failed to create output directory" << std::endl;
			return -1;
		}
	}
	fixedpoint1024 cx_center(cx_guess);
	fixedpoint1024 cy_center(cy_guess);
	Stopwatch total_stopwatch; total_stopwatch.start();
	Stopwatch iter_stopwatch; 
	for (int k = 0; k < MAX_ZOOM_EXPONENT; ++k) {
		iter_stopwatch.start();
		int max_iterations = INITIAL_MAX_ITERATIONS + MAX_ITERATIONS_SLOPE*k;
		char filename[256];
		sprintf(filename, "%d.setmap", k);
		std::filesystem::path outpath = std::filesystem::path("base-output") / std::filesystem::path(filename); 
		std::ofstream outfile(outpath.string(), std::ios::binary);
		Set_Map_Header header = { IMAGE_WIDTH, IMAGE_HEIGHT, max_iterations, DIVERGED_MAGNITUDE };
		outfile.write(reinterpret_cast<char*>(&header), sizeof(header));
		#pragma omp parallel for
		for (int i = 0; i < IMAGE_HEIGHT; ++i) {
			for (int j = 0; j < IMAGE_WIDTH; ++j) {
				fixedpoint1024 dx(pow(2.0,-static_cast<double>(k))*(static_cast<double>(j)/static_cast<double>(IMAGE_WIDTH)*WIDTH - WIDTH/2.0));
				fixedpoint1024 dy(pow(2.0,-static_cast<double>(k))*(static_cast<double>(i)/static_cast<double>(IMAGE_HEIGHT)*HEIGHT - HEIGHT/2.0));
				fixedpoint1024 cx = cx_center + dx;
				fixedpoint1024 cy = cy_center + dy;
				fixedpoint1024 zx;
				fixedpoint1024 zy;
				int iter = 0;
				while (iter < max_iterations) {
					++iter;
					fixedpoint1024 zx_old = zx;
					fixedpoint1024 zy_old = zy;
					zx = zx_old.squared() - zy_old.squared() + cx;
					zy = (zx_old*zy_old).times_2() + cy;
					double zx_truncated = zx.truncated_double();
					double zy_truncated = zy.truncated_double();
					if (zx_truncated*zx_truncated + zy_truncated*zy_truncated > DIVERGED_MAGNITUDE*DIVERGED_MAGNITUDE) {
						break;
					}
				}
				setmap[j+i*IMAGE_WIDTH] = { iter };
			}
		}
		outfile.write(reinterpret_cast<char*>(setmap), IMAGE_WIDTH*IMAGE_HEIGHT*sizeof(Set_Map_Element));
		std::cout << "(" << k+1 << " / " << MAX_ZOOM_EXPONENT << ")\t" << iter_stopwatch.time() << " seconds" << std::endl;
	}
	std::cout << "total time " << total_stopwatch.time() << " seconds" << std::endl;
	std::cout << "end of main" << std::endl;
}