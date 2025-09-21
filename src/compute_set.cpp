#include <iostream>
#include <cmath>
#include <omp.h>
#include "Stopwatch.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include "set_map_format.hpp"

constexpr int IMAGE_WIDTH = 1920;
constexpr int IMAGE_HEIGHT = 1080;
constexpr double CX_CENTER = -0.74453986035590838012;
constexpr double CY_CENTER = 0.12172377389442482241;
constexpr double HEIGHT = 2.0;
constexpr double WIDTH = HEIGHT*static_cast<double>(IMAGE_WIDTH)/static_cast<double>(IMAGE_HEIGHT);
constexpr double DIVERGED_MAGNITUDE = 2.0;
constexpr int MAX_ZOOM_EXPONENT = 64;
constexpr int INITIAL_MAX_ITERATIONS = 1000;
constexpr int MAX_ITERATIONS_SLOPE = 1000;

int main() {
	std::cout << "start of main" << std::endl;
	Set_Map_Element* setmap = new Set_Map_Element[IMAGE_WIDTH*IMAGE_HEIGHT];
	if (!std::filesystem::exists("base-output")) {
		if (!std::filesystem::create_directory("base-output")) {
			std::cout << "Failed to create output directory" << std::endl;
			return -1;
		}
	}
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
				double cx = CX_CENTER + pow(2.0,-static_cast<double>(k))*(static_cast<double>(j)/static_cast<double>(IMAGE_WIDTH)*WIDTH - WIDTH/2.0);
				double cy = CY_CENTER + pow(2.0,-static_cast<double>(k))*(static_cast<double>(i)/static_cast<double>(IMAGE_HEIGHT)*HEIGHT - HEIGHT/2.0);
				double zx = 0.0;
				double zy = 0.0;
				int iter = 0;
				double zmag = sqrt(zx*zx + zy*zy);
				while (iter < max_iterations) {
					++iter;
					double zx_old = zx;
					double zy_old = zy;
					zx = zx_old*zx_old - zy_old*zy_old + cx;
					zy = 2.0*zx_old*zy_old + cy;
					zmag = sqrt(zx*zx + zy*zy);
					if (zmag > DIVERGED_MAGNITUDE) {
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