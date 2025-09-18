#include "stb_image_write.h"
#include <iostream>
#include <cmath>
#include <omp.h>
#include "Stopwatch.h"

constexpr int IMAGE_WIDTH = 5000;
constexpr int IMAGE_HEIGHT = 3000;
constexpr double CX_CENTER = -1.0;
constexpr double CY_CENTER = 0.0;
constexpr double HEIGHT = 3.0;
constexpr double WIDTH = HEIGHT*static_cast<double>(IMAGE_WIDTH)/static_cast<double>(IMAGE_HEIGHT);
constexpr int MAX_ITERATIONS = 1000;
constexpr double DIVERGED_MAGNITUDE = 1000.0;

int main() {
	std::cout << "start of main" << std::endl;
	unsigned char* image = new unsigned char[IMAGE_WIDTH*IMAGE_HEIGHT*3];
	Stopwatch stopwatch; stopwatch.start();
	#pragma omp parallel for
	for (int i = 0; i < IMAGE_HEIGHT; ++i) {
		for (int j = 0; j < IMAGE_WIDTH; ++j) {
			double cx = CX_CENTER + static_cast<double>(j)/static_cast<double>(IMAGE_WIDTH)*WIDTH - WIDTH/2.0;
			double cy = CY_CENTER + static_cast<double>(i)/static_cast<double>(IMAGE_HEIGHT)*HEIGHT - HEIGHT/2.0;
			double zx = 0.0;
			double zy = 0.0;
			int iter = 0;
			double zmag_old;
			double zmag;
			while (iter < MAX_ITERATIONS) {
				++iter;
				double zx_old = zx;
				double zy_old = zy;
				zx = zx_old*zx_old - zy_old*zy_old + cx;
				zy = 2.0*zx_old*zy_old + cy;
				zmag_old = zmag;
				double zmag = sqrt(zx*zx + zy*zy);
				if (zmag > DIVERGED_MAGNITUDE) {
					break;
				}
			}
			if (iter < MAX_ITERATIONS) {
				unsigned char colormag = static_cast<unsigned char>(log(static_cast<double>(iter) + zmag_old/DIVERGED_MAGNITUDE)
				                                                    /
																	log(static_cast<double>(MAX_ITERATIONS))*255.0);
				image[(j+i*IMAGE_WIDTH)*3+0] = colormag;
				image[(j+i*IMAGE_WIDTH)*3+1] = colormag;
				image[(j+i*IMAGE_WIDTH)*3+2] = colormag;
			}
		}
	}
	std::cout << stopwatch.time() << " seconds" << std::endl;
	stbi_write_bmp("test.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 3, image);
	delete[] image;
	std::cout << "end of main" << std::endl;
}