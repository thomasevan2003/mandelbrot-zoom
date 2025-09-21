#include "stb_image_write.h"
#include <cmath>
#include <iostream>
#include <filesystem>
#include <omp.h>
#include <cstring>
#include <fstream>
#include <vector>
#include "set_map_format.hpp"

constexpr int FRAMES_PER_SETMAP = 30;
constexpr int IMAGE_WIDTH = 1920;
constexpr int IMAGE_HEIGHT = 1080;
constexpr double COLORMAP_SCALE = 0.4;
constexpr int ITERATION_DRAW_OFFSET = 50;

struct RGBf {
	double r;
	double g;
	double b;
};

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
	return { static_cast<unsigned char>(r*255.0), static_cast<unsigned char>(g*255.0), static_cast<unsigned char>(b*255.0) };
}

int main() {
	std::cout << "start of main" << std::endl;
	std::filesystem::path indir = std::filesystem::path("base-output");
	if (!std::filesystem::exists(indir)) {
		std::cout << "No input directory found" << std::endl;
	}
	std::filesystem::path outdir = std::filesystem::path("frames");
	if (!std::filesystem::exists(outdir)) {
		if (!std::filesystem::create_directory(outdir)) {
			std::cout << "Failed to create output directory" << std::endl;
			return -1;
		}
	}
	int frame_count = 0;
	std::vector<Set_Map_Element> setmap;
	std::vector<RGB> image;
	image.resize(IMAGE_WIDTH*IMAGE_HEIGHT);
	int infile_count = 0;
	for (std::filesystem::directory_entry dir_entry : std::filesystem::directory_iterator{indir}) {
		if (dir_entry.path().extension().string().length() == 7 && dir_entry.path().extension().string() == ".setmap") {
			infile_count++;
		}
	}
	for (int o = 0; o < infile_count; ++o) {
		char infilename[256];
		sprintf(infilename, "%d.setmap", o);
		std::ifstream infile(indir / std::filesystem::path(infilename), std::ios::binary);
		Set_Map_Header header;
		infile.read(reinterpret_cast<char*>(&header), sizeof(Set_Map_Header));
		setmap.resize(header.width*header.height);
		infile.read(reinterpret_cast<char*>(setmap.data()), header.width*header.height*sizeof(Set_Map_Element));
		char outfilename[256];
		for (int k = 0; k < FRAMES_PER_SETMAP; ++k) {
			sprintf(outfilename, "%d.jpg", frame_count);
			std::filesystem::path outpath = outdir / std::filesystem::path(outfilename);
			std::cout << outpath.string() << std::endl;
			double i_center = static_cast<double>(IMAGE_HEIGHT)/2.0;
			double j_center = static_cast<double>(IMAGE_WIDTH)/2.0;
			double m_center = static_cast<double>(header.height)/2.0;
			double n_center = static_cast<double>(header.width)/2.0;
			double zoom_factor = pow(2.0, -static_cast<double>(k)/static_cast<double>(FRAMES_PER_SETMAP));
			#pragma omp parallel for
			for (int i = 0; i < IMAGE_HEIGHT; ++i) {
				for (int j = 0; j < IMAGE_WIDTH; ++j) {
					int m = (static_cast<double>(i)-i_center)/static_cast<double>(IMAGE_HEIGHT)*static_cast<double>(header.height)*zoom_factor + m_center;
					int n = (static_cast<double>(j)-j_center)/static_cast<double>(IMAGE_WIDTH)*static_cast<double>(header.width)*zoom_factor + n_center;
					if (setmap[n+m*header.width].iterations < header.max_iterations) {
						int iter = setmap[n+m*header.width].iterations;
						if (m < header.height - 1) {
							iter = std::min(iter, setmap[n+(m+1)*header.width].iterations);
						}
						if (n < header.width - 1) {
							iter = std::min(iter, setmap[n+1+m*header.width].iterations);
						}
						if (n < header.width - 1 && m < header.height - 1) {
							iter = std::min(iter, setmap[n+1*(m+1)*header.width].iterations);
						}
						double colormag = (log(static_cast<double>(iter+ITERATION_DRAW_OFFSET))-log(static_cast<double>(ITERATION_DRAW_OFFSET)))*COLORMAP_SCALE;
						image[j+i*IMAGE_WIDTH] = colormap(colormag);
					} else {
						image[j+i*IMAGE_WIDTH] = {0,0,0};
					}
				}
			}
			stbi_write_jpg(outpath.string().c_str(), IMAGE_WIDTH, IMAGE_HEIGHT, 3, reinterpret_cast<unsigned char*>(image.data()), 100);
			++frame_count;
		}
	}
	std::cout << "end of main" << std::endl;
}