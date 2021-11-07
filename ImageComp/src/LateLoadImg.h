#pragma once
#include <VirtImage.h>
#include <ImageData.h>

// Object for storing an image that can be load & unloaded from memory
// Image is automatically loaded when interacted with
class LateLoadImg : public VirtImage {
private:
	char* _imgPath;
	ImageData* _data;
	Vec2<int> _size;
	bool _knowSize;
	VirtImage* _realImg;

public:
	LateLoadImg(const char* imgPath);
	/* 
	   For cases where you sometimes want to pass a generic image to something with
		special cases for loadable imagees, I've added an option to disguise a generic
		image as a late-load one.
	   Ugly solution but I can't think of a simple alternative at the moment
	   */
	LateLoadImg(VirtImage* realImg);

	bool isLoaded();
	void load();
	void unload();

	// VirtImage functions
	Vec2<int> getSize();
	Color getPixel(int x, int y);
	bool doesPixelExist(int x, int y);
};