#pragma once
#include "global.h"
#include "VirtImage.h"
#include "Vec2.h"

// Class for creating a virtual image from the sub-region of another virtual image
class VirtImageTrans: public VirtImage{
	private:
		VirtImage* _vImg;
		Vec2<int> _offset;
		Vec2<int> _size;
	
	public:
		VirtImageTrans();
		VirtImageTrans(VirtImage* vImg, Vec2<int> offset, Vec2<int> size);
		
		// VirtImage functions
		Vec2<int> getSize();
		Color getPixel(int x, int y);
		bool doesPixelExist(int x, int y);
};