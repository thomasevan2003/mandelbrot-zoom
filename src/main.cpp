#include "stb_image_write.h"
#include <iostream>
#include <cmath>
#include <omp.h>
#include "Stopwatch.h"
#include <cstring>
#include <filesystem>

constexpr int IMAGE_WIDTH = 1920/4;
constexpr int IMAGE_HEIGHT = 1080/4;
constexpr double CX_CENTER = -0.74453986035590838012;
constexpr double CY_CENTER = 0.12172377389442482241;
constexpr double HEIGHT = 2.0;
constexpr double WIDTH = HEIGHT*static_cast<double>(IMAGE_WIDTH)/static_cast<double>(IMAGE_HEIGHT);
constexpr int MAX_ITERATIONS = 100000;
constexpr double DIVERGED_MAGNITUDE = 2.0;
constexpr int MAX_ZOOM_EXPONENT = 64;

int main() {
	std::cout << "start of main" << std::endl;
	unsigned char* image = new unsigned char[IMAGE_WIDTH*IMAGE_HEIGHT*3];
	if (!std::filesystem::exists("base-output")) {
		if (!std::filesystem::create_directory("base-output")) {
			std::cout << "Failed to create output directory" << std::endl;
			return -1;
		}
	}
	Stopwatch stopwatch; 
	for (int k = 0; k < MAX_ZOOM_EXPONENT; ++k) {
		stopwatch.start();
		#pragma omp parallel for
		for (int i = 0; i < IMAGE_HEIGHT; ++i) {
			for (int j = 0; j < IMAGE_WIDTH; ++j) {
				double cx = CX_CENTER + pow(2.0,-static_cast<double>(k))*(static_cast<double>(j)/static_cast<double>(IMAGE_WIDTH)*WIDTH - WIDTH/2.0);
				double cy = CY_CENTER + pow(2.0,-static_cast<double>(k))*(static_cast<double>(i)/static_cast<double>(IMAGE_HEIGHT)*HEIGHT - HEIGHT/2.0);
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
		char filename[256];
		sprintf(filename, "%d.jpg", k);
		std::filesystem::path outpath = std::filesystem::path("base-output") / std::filesystem::path(filename); 
		stbi_write_jpg(outpath.string().c_str(), IMAGE_WIDTH, IMAGE_HEIGHT, 3, image, 100);
		memset(static_cast<void*>(image), 0, IMAGE_WIDTH*IMAGE_HEIGHT*3);
		std::cout << "(" << k+1 << " / " << MAX_ZOOM_EXPONENT << ")\t" << stopwatch.time() << " seconds" << std::endl;
	}
	delete[] image;
	std::cout << "end of main" << std::endl;
}