#include "global.h"
#include <iostream>
#include <fstream>
#include "Vec2.h"
#include "TileData.h"
#include "VirtualTileSet.h"
#include "RegData.h"
#include "ImgLib.h"
#include "CmdArgManager.h"
#include "UtilF.h"
#include "RegionImage.h"
#include "CompImage.h"
#include "VirtImageExt.h"

using namespace std;

void processRegImage(CompImage* compImg, RegionImage* rImage, TileData** tDataLib, int libSize, Vec2<int> tileSize){
	int virtualTileIncrement = 4;
	float percentileThreshold = 0.8f;
	
	TileData* searchData = new TileData(new LateLoadImg(rImage), tileSize);
	Vec2<int> searchImgSize = rImage->getSize();
	Vec2<int> searchTCount = searchData->tileCount;
	VirtualTileSet searchVTileSet = VirtualTileSet(searchData);
	
	printf("%dx%d tile search image (img-size: %dx%d)\n", searchTCount.x, searchTCount.y, rImage->getSize().x, rImage->getSize().y);
	
	
	int vLibSize = 0;
	for(int i=0; i<libSize; i++){
		vLibSize += tDataLib[i]->getSubsetCount(searchImgSize,virtualTileIncrement);
	}
	
	VirtualTileSet* virtualTileLib = (VirtualTileSet*)malloc(sizeof(VirtualTileSet)*vLibSize);
	int j=0;
	for(int i=0; i<libSize; i++){
		j += tDataLib[i]->createSubsetsFor(searchImgSize,virtualTileIncrement,&(virtualTileLib[j]));
	}
	assert(j == vLibSize);
	
	printf("%d virtual images\n", vLibSize);
	
	if(vLibSize == 0){
		throw runtime_error("No library image large enough to cover region.");
	}
	
	Vec2<float>* diffRanges = (Vec2<float>*)malloc(sizeof(Vec2<float>)*vLibSize);
	float lowestMax = 0;
	float lowestMin = 0; //Just tracked for sake debugging
	Dist tempDist;
	for(int i=0; i<vLibSize; i++){
		tempDist = searchVTileSet.getDifferenceDist(&(virtualTileLib[i]));
		diffRanges[i] = tempDist.getPercentileRange(percentileThreshold);
		
		//printf("(%f, %f)\n", diffRanges[i].x, diffRanges[i].y);
		if(i == 0 || lowestMax > diffRanges[i].y){
			lowestMax = diffRanges[i].y;
		}
		if(i == 0 || lowestMin > diffRanges[i].x){
			lowestMin = diffRanges[i].x;
		}
	}
	
	printf("Aggregated Diff Range: %f -> %f\n",lowestMin,lowestMax);
	
	// For the sake of debugging, do one pass to count the size of filtered pool
	int filteredCount = 0;
	for(int i=0; i<vLibSize; i++){
		if(diffRanges[i].x <= lowestMax){
			filteredCount++;
		}
	}
	printf("%d virtual images in filtered pool\n", filteredCount);
	
	float minDiff = 0;
	float diff;
	int curMinIdx = -1;
	VirtImageTrans tmpTrans;
	for(int i=0; i<vLibSize; i++){
		if(diffRanges[i].x <= lowestMax){
			tmpTrans = virtualTileLib[i].getVirtImageTrans();
			diff = VirtImageExt::calcTrueDifference(rImage, &tmpTrans);
			if(curMinIdx == -1 || diff < minDiff){
				minDiff = diff;
				curMinIdx = i;
			}
		}
	}
	
	printf("Min diff: %f\n",minDiff);
	//virtualTileLib[curMinIdx].debugPrint();
	
	printf("Printing... ");
	tmpTrans = virtualTileLib[curMinIdx].getVirtImageTrans();
	compImg->printToCanvas(rImage, &tmpTrans);
	printf("Done\n");
	
	delete searchData;
	free(virtualTileLib);
	free(diffRanges);
}

// Variant of assert meant for reporting problems in all versions (whereas assert is more meant for internal debugging)
#define affirm(cond) if(!(cond)){ printf("Err: Check failed: " #cond); exit(1); }

int main(int argc, char **argv)
{	
	char* sourceImagePath;
	char* regDataPath;
	char* libPath;
	char* outputPath;

	char* cachePath = UtilF::copyStr("./cache");
	bool ignoreCache = false;
	int tSize = 8;
	
	CmdArgManager argManager = CmdArgManager();
	
	argManager.defStringArg("imagePath",&sourceImagePath,true);
	argManager.defStringArg("regDataPath",&regDataPath,true);
	argManager.defStringArg("libPath", &libPath, true);
	argManager.defStringArg("outputPath", &outputPath, true);

	argManager.defStringArg("cache", &cachePath);
	argManager.defBoolArg("ignoreCache", &ignoreCache);
	argManager.defIntArg("tileSize", &tSize);

	if (!argManager.processArgs(argc, argv)) {
		argManager.showHelp(argv);
		return 0;
	}
	
	affirm(UtilF::fileExists(sourceImagePath));
	affirm(UtilF::fileExists(regDataPath));
	affirm(UtilF::folderExists(libPath));
	affirm(tSize > 0);
	UtilF::mkDirsForFile(outputPath);
	if (!ignoreCache) {
		UtilF::mkDirs(cachePath);
	}

	Vec2<int> tileSize = Vec2<int>(tSize, tSize);
	printf("Starting...\n");
	ImageData::init();
	affirm(ImageData::canProcessF(outputPath));
	
	RegData* regData = new RegData(regDataPath);
	ImageData* sourceImage = new ImageData(sourceImagePath);

	CompImage compImg = CompImage(regData, sourceImage);
	ImgLib imgLib = ImgLib(libPath, cachePath, !ignoreCache, true);
	
	int libSize = imgLib.getSize();
	TileData** tDataLib = (TileData**)malloc(sizeof(TileData*)*libSize);
	for(int i=0; i<libSize; i++){
		tDataLib[i] = imgLib.getTileDataFor(i,tileSize);
	}
	
	printf("%d images in library\n", libSize);
	
	for(int i=0; i< compImg.rImageCount; i++){
		printf("\n=================================\n");
		printf("(%d/%d) Processing region-image...",i+1, compImg.rImageCount);
		printf("\n=================================\n");
		
		processRegImage(&compImg,compImg.rImageList[i],tDataLib,libSize,tileSize);

		// Unload any images that got loaded into memory
		for (int j = 0; j < libSize; j++) {
			tDataLib[j]->vImg->unload();
		}
	}
	
	printf("----------------\n");
	printf("Applying region-border blur\n");
	compImg.applyBorderBlur();
	printf("Saving...\n");
	compImg.getWorkingRef()->saveTo(outputPath);
	printf("Done.\n");
	
}
