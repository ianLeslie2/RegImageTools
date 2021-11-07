#include "Region.h"
#include <algorithm>
#include <cmath>
#include <iostream>

/* Apart from the initial single-pixel regions
  regions only get combined, not created.
  But in case a region needs to be manually
  created at some point there's this bare-bones constructor
*/
Region::Region(){
}

// Single pixel region constructor
// x &  y currently unused
Region::Region(Color color, Color color2, float weight, int idx){
	size = 1;
	this->color = color;
	this->color2 = color2;
	this->weight = weight;
	this->idx = idx;
	
	mergedInto = NULL;
}

Region* Region::mergeWith(Region* other){
	if(mergedInto != NULL){
		throw std::runtime_error("Merge main target was already merged");
	}
	if(other->mergedInto != NULL){
		throw std::runtime_error("Merge secondary target was already merged");
	}
	
	// Blend colors and update weight tracker
	color  = Color::getComb(color,  (float)size, other->color,  (float)other->size);
	color2 = Color::getComb(color2, (float)size, other->color2, (float)other->size);
	weight += other->weight;
	size += other->size;
	
	other->mergedInto = this;
	
	// Update bounding box
	/*
	minX = std::min(minX,other->minX);
	minY = std::min(minY,other->minY);
	maxX = std::max(maxX,other->maxX);
	maxY = std::max(maxY,other->maxY);
	*/
	
	return this;
}

Region* Region::getMasterRegion(){
	if(mergedInto == NULL){
		return this;
	}
	Region* ref = mergedInto;
	while(ref->mergedInto != NULL){
		ref = ref->mergedInto;
	}
	return ref;
}

float Region::getColorDiff(Region* r1, Region* r2){
	// Calculate colors we'd have if regions got merged
	Color avgC  = Color::getComb(r1->color,  (float)r1->size, r2->color,  (float)r2->size);
	Color avgC2 = Color::getComb(r1->color2, (float)r1->size, r2->color2, (float)r2->size);
	
	/* 
		Adding an exponent to the region size encourages the regions to maintain a similar
		size
		(if the exponent was huge the algorithm would just merge the two smallest regions regardless of color)
	 */
	float sizeExp = 1.2f;
	
	// Calculate total "error" of this simplification
	return Color::getDiff(avgC ,r1->color )*pow(r1->weight,sizeExp) + Color::getDiff(avgC ,r2->color )*pow(r2->weight,sizeExp)
		+  Color::getDiff(avgC2,r1->color2)*pow(r1->weight,sizeExp) + Color::getDiff(avgC2,r2->color2)*pow(r2->weight,sizeExp);
}
