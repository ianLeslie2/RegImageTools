#include "LateLoadImg.h"
#include <UtilF.h>

LateLoadImg::LateLoadImg(const char* imgPath) {
	_imgPath = UtilF::copyStr(imgPath);
	_data = NULL;
	_size = Vec2<int>();
	_knowSize = false;
	_realImg = NULL;
}

LateLoadImg::LateLoadImg(VirtImage* realImg) {
	_imgPath = NULL;
	_data = NULL;
	_size = realImg->getSize();
	_knowSize = true;
	_realImg = realImg;
}

bool LateLoadImg::isLoaded() {
	if (_realImg != NULL) return true;
	return _data != NULL;
}

void LateLoadImg::load() {
	if (_realImg != NULL) return;
	if (_data == NULL) {
		_data = new ImageData(_imgPath);
		_size = _data->getSize();
		_knowSize = true;
	}
}

void LateLoadImg::unload() {
	if (_realImg != NULL) return;
	if (_data != NULL) {
		delete _data;
		_data = NULL;
	}
}

// ===================
// VirtImage functions
// ===================
Vec2<int> LateLoadImg::getSize() {
	// Don't load whole image for size query if image was loaded in the past
	if (!_knowSize) {
		load();
	}
	return _size;
}

Color LateLoadImg::getPixel(int x, int y) {
	if (_realImg != NULL) return _realImg->getPixel(x, y);
	load();
	return _data->getPixel(x, y);
}

bool LateLoadImg::doesPixelExist(int x, int y) {
	if (_realImg != NULL) return _realImg->doesPixelExist(x, y);
	load();
	return _data->doesPixelExist(x, y);
}