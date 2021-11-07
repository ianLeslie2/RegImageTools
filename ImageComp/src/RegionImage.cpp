#include "RegionImage.h"
#include "RegData.h" // RegionImage.h only defines a placeholder

RegionImage::RegionImage(int regionId, RegData* regData, ImageData* sourceImage){
	this->regionId = regionId;
	this->regData = regData;
	this->sourceImage = sourceImage;
	pos = Vec2<int>();
	size = Vec2<int>();
}

// ===================
// VirtImage functions
// ===================
Vec2<int> RegionImage::getSize(){
	return size;
}

Color RegionImage::getPixel(int x, int y){
	return sourceImage->getPixel(pos.x + x, pos.y + y);
}

bool RegionImage::doesPixelExist(int x, int y){
	return regData->getRegionIdx(pos.x + x, pos.y + y) == regionId;
}
// ===================