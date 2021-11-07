#pragma once
#include <cstdint>
#include <functional>
#include "Color.h"
#include "VirtImage.h"

typedef bool(__stdcall* ImgI_canProcessF)(const char*);
typedef uint8_t* (__stdcall* ImgI_readDataF)(const char*, int*, int*);
typedef void(__stdcall* ImgI_writeDataF)(const char*, int, int, uint8_t*);

class ImageData: public VirtImage {
private:
	inline int _idx(int x, int y);

public:
	static ImgI_canProcessF canProcessF;
	static ImgI_readDataF readDataF;
	static ImgI_writeDataF writeDataF;

	int width;
	int height;
	Color* rawData;

	ImageData(ImageData* other);
	ImageData(int width, int height);
	ImageData(int width, int height, Color fillColor);
	ImageData(const char* fPath);
	~ImageData();

	ImageData* clone();

	/* 
	 * Functions to verify that data is loaded in the expected orientation
	*/
	ImageData* test_onlyChannel(int cIdx);
	ImageData* test_leftToRightGray();
	ImageData* test_topToBotGray();

	void setPixel(int x, int y, Color c);

	// VirtImage functions
	Vec2<int> getSize();
	Color getPixel(int x, int y);
	bool doesPixelExist(int x, int y);

	ImageData* crop(int x, int y, int width, int height);

	ImageData* saveTo(const char* fPath);

	static void init();
};