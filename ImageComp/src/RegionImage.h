#pragma once
#include "global.h"
#include "Vec2.h"
#include "Dist.h"
#include "VirtImage.h"
#include "VirtualTileSet.h"
#include "ImageData.h"

class RegData;

class RegionImage: public VirtImage{
private:
	RegData* regData;
	ImageData* sourceImage;

public:
	RegionImage(int regionId, RegData* regData, ImageData* sourceImage);

	Vec2<int> pos;
	Vec2<int> size;
	int regionId;
		
	// VirtImage functions
	Vec2<int> getSize();
	Color getPixel(int x, int y);
	bool doesPixelExist(int x, int y);
};