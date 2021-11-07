#ifndef REGION_H
#define REGION_H

#include "Color.h"

class Region
{
	public:
		int size;
		float weight;
		Color color;
		Color color2;
		Region* mergedInto;
		int idx;
		
		// Bounding box
		/*
		int minX;
		int minY;
		int maxX;
		int maxY;
		*/
		
		Region();
		Region(Color color, Color color2, float weight, int idx);
		
		Region* mergeWith(Region* other);
		Region* getMasterRegion();
		static float getColorDiff(Region* r1, Region* r2);
};

#endif