#ifndef IMAGESIMPLIFIER_H
#define IMAGESIMPLIFIER_H
#include "RegionManager.h"
#include "ImageData.h"
#include <fstream>
#include "Config.h"
#include "RegData.h"

class ImageSimplifier{
	
	private:
		int _width, _height, _size;
		Region* _regionList;
		Region** _masterRegionList;
		int* _masterRegionSizeList;
		int _activeRegionCount;
		int* _adjIdxCache;
		RegionManager* _regionManager;
		
		ImageData* _workingImage;
		bool _workingImageOutdated;
		
		// Optimization Options
		Config _config;
		
		void _updateWorkingImage();
		void _init(const char* imagePath, Config configObj);
		void _addInitialRegions();
		void _addInitialBorders();
		inline Color _getVirtualPixelColor(int virtualX, int virtualY);
		inline void _setVirtualPixelColor(int virtualX, int virtualY, Color c);
		
		Color _getVirtualPixelColor(ImageData* imgRef, bool useLAB, int virtualX, int virtualY);
		void _setVirtualPixelColor(ImageData* imgRef, bool useLAB, int virtualX, int virtualY, Color c);
	
	public:
		ImageSimplifier(const char* imagePath);
		ImageSimplifier(const char* imagePath, Config configObj);
		
		RegData* getRegionData();
		
		void peformSimplificationStep();
		int getMaxRegionCount();
		int getRegionCount();
		void renderCurrentStateTo(const char* imagePath);
};

#endif