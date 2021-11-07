#include "ImageSimplifier.h"
#include <cassert>
#include "UtilF.h"


ImageSimplifier::ImageSimplifier(const char* imagePath){
	_init(imagePath,Config());
}

ImageSimplifier::ImageSimplifier(const char* imagePath, Config configObj){
	_init(imagePath,configObj);
}

void ImageSimplifier::_init(const char* imagePath, Config configObj){
	_config = configObj;
	
	if (UtilF::strEndsWith(imagePath, ".regdata")) {
		// Load RegData object and generate image to use
		RegData* tmpObj = new RegData(imagePath);
		_workingImage = tmpObj->genImage();
		delete tmpObj;
	}
	else{
		_workingImage = new ImageData(imagePath);
	}
	_workingImageOutdated = false;
	
	_width = _workingImage->width/_config.scaleFactor;
	_height = _workingImage->height/_config.scaleFactor;
	_size = _width*_height;
	_activeRegionCount = _size;
	_adjIdxCache = (int*)malloc(_size * sizeof(int));
	
	// In case _config.scaleFactor does not evenly divide the image dimensions, we
	// need to scale down the image slightly to prevent thin borders of
	// unchanged pixels
	_workingImage->crop(0, 0, _width * _config.scaleFactor, _height * _config.scaleFactor);
	
	printf("Processing %dx%d image. Virtual %dx%d image. %d initial regions.\n",_workingImage->width,_workingImage->height,_width,_height,_size);
	
	_regionManager = new RegionManager(_size*2 - _width - _height);
	_regionList = (Region*)malloc(sizeof(Region)*_size);
	_masterRegionList = (Region**)malloc(sizeof(Region*)*_size);
	_masterRegionSizeList = (int*)malloc(sizeof(int)*_size);
	
	_addInitialRegions();
	printf("%d/%d regions after initial merge sweep.\n",_activeRegionCount,_size);
	_addInitialBorders();
	printf("%d intial borders setup.\n",_regionManager->getSize());
}

// Set up core region objects and merge any adjacent pixels
// that have identical colors
void ImageSimplifier::_addInitialRegions(){
	int i = 0;
	Color curColor, lastColor, contrastColor, lastContrastColor, weightColor;
	float weightVal;
	Region *curRegRef, *testRegRef;
	
	ImageData* contrastMap = NULL;
	ImageData* weightMap = NULL;
	if(_config.contrastMapPath != NULL){
		contrastMap = new ImageData(_config.contrastMapPath);
	}
	if(_config.weightMapPath != NULL){
		weightMap = new ImageData(_config.weightMapPath);
	}
	
	for(int y=0; y<_height; y++){
		for(int x=0; x<_width; x++){
			curColor = _getVirtualPixelColor(x,y);

			if(_config.contrastMapPath != NULL){ 
				// Contrast color is in LAB space
				contrastColor = _getVirtualPixelColor(contrastMap, true, x,y);
				
				/* Note: If contrastMult > 1 this could result in a "color" with compoment
					values outside of the logical bounds, but in this case I don't think this should cause any problems. */
				contrastColor.multMatrix(_config.contrastMult,0,0,
										0,_config.contrastMult,0,
										0,0,_config.contrastMult);
			}
			else{
				contrastColor = Color(0,0,0);
			}
			if(_config.weightMapPath != NULL){
				weightColor = _getVirtualPixelColor(weightMap, false, x,y);
				weightVal = 1.0f + _config.weightMult*((float)(weightColor.x + weightColor.y + weightColor.z))/((float)(3*255));
			}
			else{
				weightVal = 1.0f;
			}
			_regionList[i] = Region(curColor,contrastColor,weightVal,i);
			
			curRegRef = &_regionList[i];
			if(x != 0 && lastColor.equals(curColor) && lastContrastColor.equals(contrastColor)){
				// Merge with region to left
				curRegRef = _regionList[i-1].getMasterRegion()->mergeWith(curRegRef);
				_activeRegionCount--;
			}
			if(y != 0 && _regionList[i-_width].color.equals(curColor) && _regionList[i-_width].color2.equals(contrastColor)){
				// Merge with region above if not already merged
				testRegRef = _regionList[i-_width].getMasterRegion();
				if(testRegRef != curRegRef){
					curRegRef = testRegRef->mergeWith(curRegRef);
					_activeRegionCount--;
				}
			}
			
			lastColor = curColor;
			lastContrastColor = contrastColor;
			i++;
		}
	}

	i = 0;
	for(int y=0; y<_height; y++){
		for(int x=0; x<_width; x++){
			_masterRegionList[i] = _regionList[i].getMasterRegion();
			_masterRegionSizeList[i] = _masterRegionList[i]->size;
			i++;
		}
	}
}

// Setup initial borders
// This is done after all initial merges are complete, otherwise
// a merge could invalidate a border
void ImageSimplifier::_addInitialBorders(){
	int i=0;
	BorderNode *bNode;
	Region *curRegRef, *testRegRef;
	for(int y=0; y<_height; y++){
		for(int x=0; x<_width; x++){
			curRegRef = _regionList[i].getMasterRegion();
			if(x != 0){
				testRegRef = _regionList[i-1].getMasterRegion();
				if(curRegRef != testRegRef){
					bNode = new BorderNode(curRegRef->idx,testRegRef->idx);
					bNode->diff = Region::getColorDiff(curRegRef,testRegRef);
					_regionManager->add(bNode);
				}
			}
			if(y != 0){
				testRegRef = _regionList[i-_width].getMasterRegion();
				if(curRegRef != testRegRef){
					bNode = new BorderNode(curRegRef->idx,testRegRef->idx);
					bNode->diff = Region::getColorDiff(curRegRef,testRegRef);
					_regionManager->add(bNode);
				}
			}
			i++;
		}
	}
}

Color ImageSimplifier::_getVirtualPixelColor(int virtualX, int virtualY){
	return _getVirtualPixelColor(_workingImage, true, virtualX, virtualY);
}

void ImageSimplifier::_setVirtualPixelColor(int virtualX, int virtualY, Color c){
	_setVirtualPixelColor(_workingImage, true, virtualX, virtualY, c);
}
/* Virtual get/set functions perform the dimension and color
	scaling operations so that the rest of the code can treat
	the underlying image as having a 1-1 correspondance with
	the tracked regions.
	
	With a scale factor of 'S', getting a color value takes the
	average value of a SxS square. Likewise, setting a color
	value sets the color values of a SxS square.
*/

Color ImageSimplifier::_getVirtualPixelColor(ImageData* imgRef, bool useLAB, int virtualX, int virtualY){
	int c_x = 0;
	int c_y = 0;
	int c_z = 0;
	int w = imgRef->width;
	int h = imgRef->height;
	Color tmpC;
	for (int x=virtualX*_config.scaleFactor; x<(virtualX+1)*_config.scaleFactor; x++){
		for (int y=virtualY*_config.scaleFactor; y<(virtualY+1)*_config.scaleFactor; y++){
			tmpC = imgRef->getPixel(x, y);
			c_x += (int)tmpC.x;
			c_y += (int)tmpC.y;
			c_z += (int)tmpC.z;
		}
	}
	c_x = (int)(c_x/(_config.scaleFactor*_config.scaleFactor));
	c_y = (int)(c_y/(_config.scaleFactor*_config.scaleFactor));
	c_z = (int)(c_z/(_config.scaleFactor*_config.scaleFactor));
	
	Color c = Color(c_x,c_y,c_z);
	if(useLAB){
		// Convert to LAB for better color comparisions
		c.convert_sRGB_XYZ();
		c.convert_XYZ_LAB();
	}
	
	return c;
}

void ImageSimplifier::_setVirtualPixelColor(ImageData* imgRef,  bool useLAB, int virtualX, int virtualY, Color c){
	if(useLAB){
		// Convert back to sRGB for storage
		c.convert_LAB_XYZ();
		c.convert_XYZ_sRGB();
	}

	for (int x=virtualX*_config.scaleFactor; x<(virtualX+1)*_config.scaleFactor; x++){
		for (int y=virtualY*_config.scaleFactor; y<(virtualY+1)*_config.scaleFactor; y++){
			imgRef->setPixel(x, y, c);
		}
	}
}

RegData* ImageSimplifier::getRegionData(){
	RegData* obj = new RegData(_width, _height, _activeRegionCount);

	// Remap indices of used regions so there are no gaps
	// and create a list of region colors as sRGB int triplets
	// --------------------------------------------------
	int* adjIdxList = _adjIdxCache;
	for(int i=0; i<_size; i++){
		adjIdxList[i] = -1;
	}
	
	Region* tmpRegion;
	Color tmpColor;
	int idxTracker = 0;
	for(int i=0; i<_size; i++){
		tmpRegion = _masterRegionList[i]->getMasterRegion();
		if(adjIdxList[tmpRegion->idx] == -1){
			adjIdxList[tmpRegion->idx] = idxTracker;
			
			tmpColor = tmpRegion->color;
			tmpColor.convert_LAB_XYZ();
			tmpColor.convert_XYZ_sRGB();
			obj->rgbColors[idxTracker++] = tmpColor;
		}
		obj->idxList[i] = adjIdxList[tmpRegion->idx];
	}
	assert(idxTracker == _activeRegionCount);

	return obj;
}

int ImageSimplifier::getMaxRegionCount(){
	return _size;
}

int ImageSimplifier::getRegionCount(){
	return _activeRegionCount;
}

void ImageSimplifier::_updateWorkingImage(){
	//printf("Updating working image...");
	
	int i=0;
	for(int y=0; y<_height; y++){
		for(int x=0; x<_width; x++){
			if(_masterRegionList[i]->mergedInto != NULL || _masterRegionList[i]->size != _masterRegionSizeList[i]){
				// Pixel got changed
				_masterRegionList[i] = _masterRegionList[i]->getMasterRegion();
				_masterRegionSizeList[i] = _masterRegionList[i]->size;
				
				_setVirtualPixelColor(x,y,_masterRegionList[i]->color);
			}
			i++;
		}
	}
	
	_workingImageOutdated = false;
	//printf(" Done\n");
}

void ImageSimplifier::renderCurrentStateTo(const char* imagePath){
	if(_workingImageOutdated){
		_updateWorkingImage();
	}
	_workingImage->saveTo(imagePath);
}

void ImageSimplifier::peformSimplificationStep(){
	//printf("Performing simplification step...");
	
	BorderNode* minNode = _regionManager->pop();
	//printf("minNode diff: %f\n",minNode->diff);
	//_regionManager->debugPrint();
	_regionList[minNode->r1Idx].mergeWith(&(_regionList[minNode->r2Idx]));
	_regionManager->updateForMerge(minNode->r1Idx,minNode->r2Idx,_regionList);
	_activeRegionCount--;
	
	delete minNode;
	
	_workingImageOutdated = true;
	//printf(" Done\n");
}