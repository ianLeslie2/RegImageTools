#pragma once
#include "global.h"
#include "Vec2.h"
#include "Color.h"

class VirtImage{
	public:
		virtual Vec2<int> getSize() = 0;
		virtual Color getPixel(int x, int y) = 0;
		virtual bool doesPixelExist(int x, int y) = 0;
};