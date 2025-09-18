#include "stb_image_write.h"
#include <iostream>
#include <cmath>
#include <omp.h>
#include "Stopwatch.h"
#include <cstring>
#include <filesystem>

constexpr int IMAGE_WIDTH = 1920*2;
constexpr int IMAGE_HEIGHT = 1080*2;
constexpr double CX_CENTER = -0.74453986035590838012;
constexpr double CY_CENTER = 0.12172377389442482241;
constexpr double HEIGHT = 2.0;
constexpr double WIDTH = HEIGHT*static_cast<double>(IMAGE_WIDTH)/static_cast<double>(IMAGE_HEIGHT);
constexpr int MAX_ITERATIONS = 10000;
constexpr double DIVERGED_MAGNITUDE = 2.0;
constexpr int MAX_ZOOM_EXPONENT = 64;
constexpr double COLORMAP_SCALE = 0.4;
constexpr int INITIAL_MAX_ITERATIONS = 1000;
constexpr int MAX_ITERATIONS_SLOPE = 1000;
constexpr int ITERATION_DRAW_OFFSET = 50;

struct RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

RGB colormap(double x) {
	x = fmod(x, 1.0);
	if (x < 0.0) {
		x = x + 1.0;
	}
	x = x * 4.0;
	double r = 0.0;
	double g = 0.0;
	double b = 1.0;
	if (x < 1.0) {
		
	} else if (x < 2.0) {
		r = x - 1.0;
	} else if (x < 3.0) {
		r = 1.0;
		g = x - 2.0;
	} else {
		r = 3.0 - x;
		g = 3.0 - x;
	}
	return {static_cast<unsigned char>(r*255.0), static_cast<unsigned char>(g*255.0), static_cast<unsigned char>(b*255.0)};
}

int main() {
	std::cout << "start of main" << std::endl;
	RGB* image = new RGB[IMAGE_WIDTH*IMAGE_HEIGHT];
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
		#pragma omp parallel for
		for (int i = 0; i < IMAGE_HEIGHT; ++i) {
			for (int j = 0; j < IMAGE_WIDTH; ++j) {
				double cx = CX_CENTER + pow(2.0,-static_cast<double>(k))*(static_cast<double>(j)/static_cast<double>(IMAGE_WIDTH)*WIDTH - WIDTH/2.0);
				double cy = CY_CENTER + pow(2.0,-static_cast<double>(k))*(static_cast<double>(i)/static_cast<double>(IMAGE_HEIGHT)*HEIGHT - HEIGHT/2.0);
				double zx = 0.0;
				double zy = 0.0;
				int iter = 0;
				double zmag = sqrt(zx*zx + zy*zy);
				double zmag_old;
				while (iter < max_iterations) {
					++iter;
					double zx_old = zx;
					double zy_old = zy;
					zx = zx_old*zx_old - zy_old*zy_old + cx;
					zy = 2.0*zx_old*zy_old + cy;
					zmag_old = zmag;
					zmag = sqrt(zx*zx + zy*zy);
					if (zmag > DIVERGED_MAGNITUDE) {
						break;
					}
				}
				if (iter < max_iterations) {
					double colormag = (log(static_cast<double>(iter+ITERATION_DRAW_OFFSET))-log(static_cast<double>(ITERATION_DRAW_OFFSET)))*COLORMAP_SCALE;
					image[j+i*IMAGE_WIDTH] = colormap(colormag);
				}
			}
		}
		char filename[256];
		sprintf(filename, "%d.jpg", k);
		std::filesystem::path outpath = std::filesystem::path("base-output") / std::filesystem::path(filename); 
		stbi_write_jpg(outpath.string().c_str(), IMAGE_WIDTH, IMAGE_HEIGHT, 3, reinterpret_cast<unsigned char*>(image), 100);
		memset(static_cast<void*>(image), 0, IMAGE_WIDTH*IMAGE_HEIGHT*3);
		std::cout << "(" << k+1 << " / " << MAX_ZOOM_EXPONENT << ")\t" << iter_stopwatch.time() << " seconds" << std::endl;
	}
	delete[] image;
	std::cout << "total time " << total_stopwatch.time() << " seconds" << std::endl;
	std::cout << "end of main" << std::endl;
}