#include "VirtImageExt.h"

float VirtImageExt::calcTrueDifference(VirtImage* vImg1, VirtImage* vImg2){
	Vec2<int> compSize = Vec2<int>::min(vImg1->getSize(),vImg2->getSize());
	assert(compSize.equals(vImg1->getSize()));
	
	float sum = 0;
	int pixelCount = 0;
	for(int x=0; x<compSize.x; x++){
		for(int y=0; y<compSize.y; y++){
			if(!vImg1->doesPixelExist(x,y) || !vImg2->doesPixelExist(x,y)){
				// Pixel does not exist for one of the images. No comparison to be made
				continue;
			}
			pixelCount++;
			sum += Color::getManhattanDiff(vImg1->getPixel(x, y), vImg2->getPixel(x, y)) / 3.0f;
		}
	}
	
	return sum/(pixelCount);
}