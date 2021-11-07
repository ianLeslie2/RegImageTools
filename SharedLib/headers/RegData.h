#pragma once
#include <fstream>
#include "Color.h"
#include "ImageData.h"

class RegData {
private:
	int _uncompressedRegionDataSize();
	void _loadFrom(std::ifstream* fileStream);
	inline int _idx(int x, int y);

public:
	int rCount;
	int width;
	int height;
	Color* rgbColors;
	int* idxList;

	RegData(const char* inPath);
	RegData(std::ifstream* fileStream);
	RegData(int w, int h, int regionCount);
	~RegData();

	int getVersion();

	int getRegionIdx(int x, int y);

	void saveTo(std::ofstream* fileStream);
	void saveTo(const char* outPath);
	ImageData* genImage();
	void snapshot(const char* outPath); // Generates images, saves image, and deletes image object
};