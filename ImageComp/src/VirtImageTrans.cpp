#include "VirtImageTrans.h"
#include <cassert>

VirtImageTrans::VirtImageTrans(){
	_vImg = NULL;
	_offset = Vec2<int>();
	_size = Vec2<int>();
}

VirtImageTrans::VirtImageTrans(VirtImage* vImg, Vec2<int> offset, Vec2<int> size){
	_vImg = vImg;
	_offset = offset;
	Vec2<int> workingSize = Vec2<int>::sub(vImg->getSize(),offset);
	assert(Vec2<int>::min(size,workingSize).equals(size));
	_size = size;
}

// ===================
// VirtImage functions
// ===================
Vec2<int> VirtImageTrans::getSize(){
	return _size;
}

Color VirtImageTrans::getPixel(int x, int y){
	return _vImg->getPixel(x + _offset.x,y + _offset.y);
}

bool VirtImageTrans::doesPixelExist(int x, int y){
	return _vImg->doesPixelExist(x + _offset.x,y + _offset.y);
}
// ===================