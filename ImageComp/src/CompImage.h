#pragma once
#include "global.h"
#include "RegionImage.h"
#include "Vec2.h"
#include "VirtImage.h"
#include "RegData.h"
#include "ImageData.h"

class CompImage {
	private:
		RegData* _regDataRef;
		ImageData* _sourceRef;
		ImageData* _workingImage;

		inline bool _inBounds(int x, int y);
		void _borderBlurPixel(int x, int y, ImageData* outCanvas, int sqRadius, float* weights);
	
	public:
		RegionImage** rImageList;
		int rImageCount;
		int width;
		int height;
		
		CompImage(RegData* regDataRef, ImageData* sourceRef);

		// Offset is print-image.pos - rImage.pos (in pixel units)
		// Since the print-image needs to cover the region, the offset components must be <= 0
		void printToCanvas(RegionImage* rImage, VirtImage* vImg);
		void applyBorderBlur();
		ImageData* getWorkingRef();
};