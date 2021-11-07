#include "CompImage.h"
#include <unordered_map>
#include <assert.h>

CompImage::CompImage(RegData* regDataRef, ImageData* sourceRef){
	_regDataRef = regDataRef;
	_sourceRef = sourceRef;

	rImageCount = _regDataRef->rCount;
	width = _regDataRef->width;
	height = _regDataRef->height;

	if (width != _sourceRef->width || height != _sourceRef->height) {
		printf("Err: CompImage dimension mismatch between regData(%d,%d) & source(%d,%d)\n", width,height, _sourceRef->width, _sourceRef->height);
		exit(1);
	}
	_workingImage = new ImageData(width, height, Color(0, 0, 0));
	
	printf("  rImageCount: %d\n  width: %d\n  height: %d\n",rImageCount,width,height);

	// Build list of regions and calculate bounding boxes for each region
	// For this step the "size" property is used as the max-val part of the bounding box
	rImageList = (RegionImage**)malloc(sizeof(RegionImage*)*rImageCount);
	int foundRegCount = 0;
	int regId;
	RegionImage* rImg;
	std::unordered_map<int,int> regIdxMap = std::unordered_map<int,int>();
	for(int y=0; y<height; y++){
		for(int x=0; x<width; x++){
			regId = _regDataRef->getRegionIdx(x, y);
			if(regIdxMap.find(regId) == regIdxMap.end()){
				// New region
				regIdxMap[regId] = foundRegCount;
				rImageList[foundRegCount] = new RegionImage(regId, _regDataRef, _sourceRef);
				rImageList[foundRegCount]->pos = Vec2<int>(x,y);
				rImageList[foundRegCount]->size = Vec2<int>(x,y);
				foundRegCount++;
			}
			else{
				rImg = rImageList[regIdxMap[regId]];
				rImg->pos = Vec2<int>::min(rImg->pos,Vec2<int>(x,y));
				rImg->size = Vec2<int>::max(rImg->size,Vec2<int>(x,y));
			}
		}
	}
	
	// Fix size propery of r-images
	for(int i=0; i<rImageCount; i++){
		rImageList[i]->size = Vec2<int>::add(Vec2<int>::sub(rImageList[i]->size,rImageList[i]->pos),Vec2<int>(1,1));
	}
	
	printf("RegionImage list generated.\n");
	printf("RegData object successfully initialized.\n");
}

void CompImage::printToCanvas(RegionImage* rImage, VirtImage* vImg){
	assert(Vec2<int>::min(rImage->getSize(), vImg->getSize()).equals(rImage->getSize()));
	
	Vec2<int> pos = rImage->pos;
	for(int x=rImage->pos.x; x<rImage->pos.x+rImage->size.x; x++){
		for(int y=rImage->pos.y; y<rImage->pos.y+rImage->size.y; y++){
			if(_regDataRef->getRegionIdx(x,y) != rImage->regionId){
				// Pixel not part of this region
				continue;
			}
			_workingImage->setPixel(x, y, vImg->getPixel(x - pos.x, y - pos.y));
		}
	}
}

bool CompImage::_inBounds(int x, int y) {
	return x >= 0 && x < width && y >= 0 && y < height;
}

#define wIdx(i,j) (j)*weightZoneWidth + (i)
#define adjPos(i,j) x + (i) - sqRadius, y + (j) - sqRadius

void CompImage::_borderBlurPixel(int x, int y, ImageData* outCanvas, int sqRadius, float* weights) {
	float sums[3];
	float wSum;
	int regId;
	bool isBorder;
	Color tmpC;

	int weightZoneWidth = sqRadius * 2 + 1;

	regId = _regDataRef->getRegionIdx(x,y);
	sums[0] = sums[1] = sums[2] = 0;
	isBorder = false;
	wSum = 0;
	for (int i = 0; i < weightZoneWidth; i++) {
		for (int j = 0; j < weightZoneWidth; j++) {
			if (!_inBounds(adjPos(i,j))) continue;
			if (_regDataRef->getRegionIdx(adjPos(i, j)) != regId) {
				isBorder = true;
			}
			tmpC = _workingImage->getPixel(adjPos(i, j));
			sums[0] += weights[wIdx(i,j)] * (float)tmpC.x;
			sums[1] += weights[wIdx(i, j)] * (float)tmpC.y;
			sums[2] += weights[wIdx(i, j)] * (float)tmpC.z;
			wSum += weights[wIdx(i, j)];
		}
	}
	if (isBorder) {
		tmpC = Color(sums[0] / wSum, sums[1] / wSum, sums[2] / wSum);
		outCanvas->setPixel(x, y, tmpC);
	}
	else {
		outCanvas->setPixel(x, y, _workingImage->getPixel(x, y));
	}
}

// Applies a blur effect to pixels that are near a region border.
void CompImage::applyBorderBlur(){
	/* 
	 * Set size of region used for blur calculation and calculate pixel weights
	 * The term "radius" is used informally here
	 * The region used for blurring is always a square with an odd width and height
	 * The "radius" is the number of additional pixels to the left & right of the central pixel.
	 * ie. a "radius" of 1 means a 3x3 square is used.
	 * The "diameter" is just the width/height of the whole square. (so radius*2 + 1)
	 */
	const int radius = 1;
	const int diameter = radius * 2 + 1;
	float weights[diameter * diameter];
	for (int i = 0; i < diameter; i++) {
		for (int j = 0; j < diameter; j++) {
			int distSq = (i - radius) * (i - radius) + (j - radius) * (j - radius);
			if (distSq == 0) {
				weights[j * diameter + i] = 1.5f; // Weight of central pixel
			}
			else {
				weights[j * diameter + i] = 1.0f / sqrt((float)distSq);
			}
		}
	}
	
	ImageData* newCanvas = new ImageData(width, height);
	for(int x=0; x<width; x++){
		for(int y=0; y<height; y++){
			_borderBlurPixel(x, y, newCanvas, radius, weights);
		}
	}
	
	delete _workingImage;
	_workingImage = newCanvas;
}

ImageData* CompImage::getWorkingRef() {
	return _workingImage;
}