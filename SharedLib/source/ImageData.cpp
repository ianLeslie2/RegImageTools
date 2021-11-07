#include "ImageData.h"
#include <cstdlib>
#include <Windows.h>
#include <cstdio>
#include <assert.h>

#define CONCAT(a,b,c) a ## b ## c
#define LOAD_FUN(name) CONCAT(name,F,) = (CONCAT(ImgI_,name,F))GetProcAddress(handle, #name); if(!CONCAT(name,F,)){ printf("Could not load function: " #name "\n"); exit(1); }

ImgI_canProcessF ImageData::canProcessF = NULL;
ImgI_readDataF ImageData::readDataF = NULL;
ImgI_writeDataF ImageData::writeDataF = NULL;

void ImageData::init() {
	const char* libName = "ImageInterface.dll";
	HINSTANCE handle = LoadLibraryA(libName);
	if (!handle) {
		printf("Error: Could not load %s\n", libName);
		exit(1);
	}

	FARPROC funAddr = GetProcAddress(handle, "canProcess");
	if (!funAddr) {
		printf("Error: Could not load function %s\n", "canProcess");
		exit(1);
	}

	LOAD_FUN(canProcess);
	LOAD_FUN(readData);
	LOAD_FUN(writeData);

	printf("Successfully loaded %s\n", libName);
}

ImageData::ImageData(int width, int height) {
	this->width = width;
	this->height = height;
	rawData = (Color*)malloc((size_t)width * height * sizeof(Color));
}

ImageData::ImageData(int width, int height, Color fillColor) {
	this->width = width;
	this->height = height;
	rawData = (Color*)malloc((size_t)width * height * sizeof(Color));
	for (int i = 0; i < width * height; i++) {
		rawData[i] = fillColor;
	}
}

ImageData::ImageData(const char* fPath) {
	if (canProcessF == NULL) {
		printf("Can't save, ImageInterface.dll has not been configured.\n");
		exit(1);
	}
	if(!canProcessF(fPath)){
		printf("Loaded ImageInterface.dll does not support file type:\n%s\n", fPath);
		exit(1);
	}

	uint8_t* data = readDataF(fPath, &width, &height);
	rawData = (Color*)malloc((size_t)width * height * sizeof(Color));
	for (int i = 0; i < width * height; i++) {
		rawData[i] = Color(
						data[4 * i + 0], 
						data[4 * i + 1], 
						data[4 * i + 2]);
		// Alpha channel is ignored
	}
	free(data);
}

ImageData::ImageData(ImageData* other) {
	width = other->width;
	height = other->height;
	rawData = (Color*)malloc((size_t)width * height * sizeof(Color));
	/*for (int i = 0; i < width * height; i++) {
		rawData[i] = other->rawData[i];
	}*/
	memcpy(rawData, other->rawData, (size_t)width * height * sizeof(Color));
}

ImageData::~ImageData() {
	free(rawData);
}

ImageData* ImageData::clone() {
	return new ImageData(this);
}

ImageData* ImageData::test_onlyChannel(int cIdx) {
	assert(0 <= cIdx && cIdx <= 2);
	for (int i = 0; i < width * height; i++) {
		switch (cIdx) {
		case 0:
			rawData[i].y = 0;
			rawData[i].z = 0;
			break;
		case 1:
			rawData[i].x = 0;
			rawData[i].z = 0;
			break;
		case 2:
			rawData[i].x = 0;
			rawData[i].y = 0;
			break;
		}
	}

	return this;
}

ImageData* ImageData::test_leftToRightGray() {
	double avgVal;
	Color tmpC;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			tmpC = rawData[_idx(x, y)];
			avgVal = (tmpC.x + tmpC.y + tmpC.z)/3;
			tmpC.x = tmpC.x + (avgVal - tmpC.x) * ((float)x / width);
			tmpC.y = tmpC.y + (avgVal - tmpC.y) * ((float)x / width);
			tmpC.z = tmpC.z + (avgVal - tmpC.z) * ((float)x / width);
			rawData[_idx(x, y)] = tmpC;
		}
	}

	return this;
}

ImageData* ImageData::test_topToBotGray() {
	double avgVal;
	Color tmpC;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			tmpC = rawData[_idx(x, y)];
			avgVal = (tmpC.x + tmpC.y + tmpC.z) / 3;
			tmpC.x = tmpC.x + (avgVal - tmpC.x) * ((float)(height - y) / height);
			tmpC.y = tmpC.y + (avgVal - tmpC.y) * ((float)(height - y) / height);
			tmpC.z = tmpC.z + (avgVal - tmpC.z) * ((float)(height - y) / height);
			rawData[_idx(x, y)] = tmpC;
		}
	}

	return this;
}

int ImageData::_idx(int x, int y) {
	return width * y + x;
}

void ImageData::setPixel(int x, int y, Color c) {
	rawData[_idx(x, y)] = c;
}

Color ImageData::getPixel(int x, int y) {
	return rawData[_idx(x, y)];
}

Vec2<int> ImageData::getSize() {
	return Vec2(width, height);
}

bool ImageData::doesPixelExist(int x, int y) {
	return x >= 0 && x < width && y >= 0 && y < height;
}

ImageData* ImageData::crop(int x, int y, int width, int height) {
	if (width + x > this->width || height + y > this->height) {
		printf("Err in ImageData::crop(%d,%d,%d,%d), crop-zone outsize of image region.\n", x, y, width, height);
		exit(1);
	}

	Color* newData = (Color*)malloc((size_t)width * height * sizeof(Color));
	int oldIdx, newIdx;
	for (int x2 = x; x2 < x + width; x2++) {
		for (int y2 = y; y2 < y + height; y2++) {
			oldIdx = _idx(x2, y2);
			newIdx = width * (y2 - y) + (x2 - x);
			newData[newIdx] = rawData[oldIdx];
		}
	}
	free(rawData);
	rawData = newData;
	this->width = width;
	this->height = height;

	return this;
}

ImageData* ImageData::saveTo(const char* fPath) {
	if (canProcessF == NULL) {
		printf("Can't save, ImageInterface.dll has not been configured.\n");
		exit(1);
	}
	if (!canProcessF(fPath)) {
		printf("Loaded ImageInterface.dll does not support file type:\n%s\n", fPath);
		exit(1);
	}

	uint8_t* data = (uint8_t*)malloc((size_t)width * height * 4 * sizeof(uint8_t));
	Color tmp;
	for (int i = 0; i < width * height; i++) {
		tmp = rawData[i];
		data[4 * i + 0] = (uint8_t)tmp.x;
		data[4 * i + 1] = (uint8_t)tmp.y;
		data[4 * i + 2] = (uint8_t)tmp.z;
		data[4 * i + 3] = (uint8_t)255; //Alpha is treated as fully opaque
	}
	writeDataF(fPath, width, height, data);
	free(data);
	return this;
}