#include "stb_image_write.h"

constexpr int IMAGE_WIDTH = 1920;
constexpr int IMAGE_HEIGHT = 1080;

int main() {
	unsigned char* image = new unsigned char[IMAGE_WIDTH*IMAGE_HEIGHT*4];
	for (int i = 0; i < IMAGE_HEIGHT; ++i) {
		for (int j = 0; j < IMAGE_WIDTH; ++j) {
			image[(j+IMAGE_WIDTH*i)*4] = static_cast<unsigned char>(static_cast<float>(j)/static_cast<float>(IMAGE_WIDTH)*255.0f);
			image[(j+IMAGE_WIDTH*i)*4+1] = static_cast<unsigned char>(static_cast<float>(i)/static_cast<float>(IMAGE_HEIGHT)*255.0f);
			image[(j+IMAGE_WIDTH*i)*4+3] = 255;
		}
	}
	stbi_write_bmp("test.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 4, image);
	delete[] image;
}