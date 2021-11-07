#include "TileData.h"
#include <assert.h>
#include "UtilF.h"
#include <math.h>
#include <cstring>
#include "VirtualTileSet.h"
#include "DebugPrint.h"

TileData::TileData(){
	version = TDATA_VERSION;
	tileCount = Vec2<int>();
	compCount = 0;
	vImg = NULL;
	imgSize = Vec2<int>();
	data = NULL;
	minVal = 0;
	maxVal = 255;

}

TileData::TileData(LateLoadImg* vImg, Vec2<int> tileSize){
	this->version = TDATA_VERSION;
	this->tileCount = Vec2<int>();
	this->compCount = 0;
	this->vImg = vImg;
	this->imgSize = vImg->getSize();
	
	_loadFromImage(tileSize, 3, 0, 255);
	vImg->unload(); //Image probably wont be used for a while
}

int TileData::_calcIdx(int tileX, int tileY, int compIdx){
	return 3*(tileY*tileCount.x + tileX) + compIdx;
}

TCell TileData::getCell(int x, int y, int compIdx) {
	return data[_calcIdx(x, y, compIdx)];
}

void TileData::_freeMemory(){
	int dataLen = tileCount.x*tileCount.y*compCount;
	if(dataLen != 0){
		free(data);
		tileCount = Vec2<int>();
	}
}

float TileData::_getVImgPixel(int x, int y, int compIdx) {
	Color color = vImg->getPixel(x, y);
	switch (compIdx) {
	case 0:
		return (float)color.x;
	case 1:
		return (float)color.y;
	default:
		return (float)color.z;
	}
}

void TileData::_loadTile(Vec2<int> tilePos, Vec2<int> adjTileSize, int compIdx){
	assert(adjTileSize.x*adjTileSize.y != 0);
	
	long sum;
	float mean, var;
	int pixelCount;
	
	// Calculate mean value
	sum = 0;
	pixelCount = 0;
	for(int x=tilePos.x*tileSize.x; x<tilePos.x*tileSize.x + adjTileSize.x; x++){
		for(int y=tilePos.y*tileSize.y; y<tilePos.y*tileSize.y + adjTileSize.y; y++){
			if(!vImg->doesPixelExist(x,y)){
				// Pixel is considered invisible
				continue;
			}
			sum += (long)_getVImgPixel(x,y,compIdx);
			pixelCount++;
		}
	}
	
	if(pixelCount == 0){
		// Fully invisible tile
		data[_calcIdx(tilePos.x,tilePos.y,compIdx)].exists = false;
		//printf("Fully invisible tile @ (%d,%d)\n",tilePos.x,tilePos.y);
		return;
	}
	
	mean = sum/(float)(pixelCount);
	
	// Calculate variance
	var = 0;
	float tmp;
	for(int x=tilePos.x*tileSize.x; x<tilePos.x*tileSize.x + adjTileSize.x; x++){
		for(int y=tilePos.y*tileSize.y; y<tilePos.y*tileSize.y + adjTileSize.y; y++){
			if(!vImg->doesPixelExist(x,y)){
				// Pixel is considered invisible
				continue;
			}
			tmp = _getVImgPixel(x,y,compIdx);
			var += (mean-tmp)*(mean-tmp);
		}
	}
	var /= (float)(pixelCount);
	
	
	Dist dObj = Dist(mean,var);
	dObj.applyTruncatedAdjustment(minVal,maxVal);
	//printf("(%d,%d,%d) -> mean: %f, var: %f\n", tilePos.x,tilePos.y,compIdx,dObj.mean,dObj.var);
	data[_calcIdx(tilePos.x,tilePos.y,compIdx)].dist = dObj;
	data[_calcIdx(tilePos.x,tilePos.y,compIdx)].exists = true;
}

// Currently pixels with any degree of transparency are considered invisible
void TileData::_loadFromImage(Vec2<int> tileSize,int compCount,float minVal,float maxVal){
	assert(vImg != NULL);
	// Free memory if necessary
	_freeMemory();
	
	this->tileSize = tileSize;
	Vec2<int> iSize = vImg->getSize();
	this->tileCount = Vec2<int>(	(int)ceil(iSize.x/(float)tileSize.x),
									(int)ceil(iSize.y/(float)tileSize.y));
	this->compCount = compCount;
	this->minVal = minVal;
	this->maxVal = maxVal;
	
	printf("Loading TileData from image (imgSize: %dx%d, tSize: %dx%d)\n",iSize.x,iSize.y,tileSize.x,tileSize.y);
	//printf("  Comps: %d\n",compCount);
	//printf("  Calculated Tile Count: %dx%d\n",tileCount.x,tileCount.y);
	
	data = (TCell*)malloc(sizeof(TCell)*((size_t)tileCount.x*tileCount.y*compCount));
	
	int tWidth, tHeight;
	for(int tileX=0; tileX<tileCount.x; tileX++){
		tWidth = UtilF::min(tileSize.x,iSize.x-(tileX*tileSize.x));
		for(int tileY=0; tileY<tileCount.y; tileY++){
			tHeight = UtilF::min(tileSize.y,iSize.y-(tileY*tileSize.y));
			for(int compIdx=0; compIdx<compCount; compIdx++){
				_loadTile(Vec2<int>(tileX,tileY),Vec2<int>(tWidth,tHeight),compIdx);
			}
		}
	}
	
	//printf("%d tiles loaded\n", tileCount.x*tileCount.y*compCount);
}


/*
	maxOffset*tSize + pixelSize = containerPixelSize
	maxOffset = (containerPixelSize-pixelSize) / tSize
*/
Vec2<int> TileData::_calcMaxOffsetFor(Vec2<int> pixelSize){
	Vec2<int> tmp = Vec2<int>::sub(imgSize,pixelSize);
	return Vec2<int>(	(int)(tmp.x/(float)tileSize.x),
						(int)(tmp.y/(float)tileSize.y) );
}

int TileData::getSubsetCount(Vec2<int> pixelSize, int tileIncrement){
	if(!Vec2<int>::min(pixelSize,imgSize).equals(pixelSize)){
		// Sub-zone doesn't fit
		return 0;
	}
	
	Vec2<int> tmp = _calcMaxOffsetFor(pixelSize);
	tmp.x = (int)(tmp.x/(float)tileIncrement);
	tmp.y = (int)(tmp.y/(float)tileIncrement);
	
	return (tmp.x+1)*(tmp.y+1);
}

int TileData::createSubsetsFor(Vec2<int> pixelSize, int tileIncrement, VirtualTileSet* outArray){
	if(!Vec2<int>::min(pixelSize,imgSize).equals(pixelSize)){
		// Sub-zone doesn't fit
		return 0;
	}
	
	Vec2<int> maxOffset = _calcMaxOffsetFor(pixelSize);
	Vec2<int> subsetSize = Vec2<int>(	(int)ceil(pixelSize.x/(float)tileSize.x),
										(int)ceil(pixelSize.y/(float)tileSize.y) );
	Vec2<int> offset;
	int i = 0;
	for(offset.x=0; offset.x <= maxOffset.x; offset.x += tileIncrement){
		for(offset.y=0; offset.y <= maxOffset.y; offset.y += tileIncrement){
			outArray[i] = VirtualTileSet(this,offset,subsetSize);
			i++;
		}
	}
	
	return i;
}

/*
 NOTE:
	The virtual-image object is not serialized when the data is written
	to a stream. This is why when the object is de-serialized from
	a stream the v-image reference needs to be supplied manually.
	
	The TileData object doesn't actually access the virtual image
	except when the object is constructed from it.
	It's just usefull to be able to trace back to from where the
	TileData object got made.
*/
	
void TileData::writeToStream(std::ofstream* outStream){
	printf("Serializing TileData\n");
	outStream->write(reinterpret_cast<char*>(&version),	sizeof(int));
	
	tileSize.writeToStream(outStream);
	tileCount.writeToStream(outStream);
	imgSize.writeToStream(outStream);
	outStream->write(reinterpret_cast<char*>(&compCount),sizeof(int));
	outStream->write(reinterpret_cast<char*>(&minVal),	sizeof(float));
	outStream->write(reinterpret_cast<char*>(&maxVal),	sizeof(float));
	outStream->write(reinterpret_cast<char*>(data),		sizeof(TCell)*((size_t)tileCount.x*tileCount.y*compCount));
}

void TileData::readFromStream(std::ifstream* inStream, LateLoadImg* vImg){
	printf("Deserializing TileData\n");
	
	inStream->read(reinterpret_cast<char*>(&version),	sizeof(int));
	assert(version == TDATA_VERSION);
	
	// Free memory if necessary
	_freeMemory();
	
	this->vImg = vImg;
	
	tileSize.readFromStream(inStream);
	tileCount.readFromStream(inStream);
	imgSize.readFromStream(inStream);
	inStream->read(reinterpret_cast<char*>(&compCount),sizeof(int));
	inStream->read(reinterpret_cast<char*>(&minVal),	sizeof(float));
	inStream->read(reinterpret_cast<char*>(&maxVal),	sizeof(float));
	
	data = (TCell*)malloc(sizeof(TCell)*((size_t)tileCount.x*tileCount.y*compCount));
	inStream->read(reinterpret_cast<char*>(data),		sizeof(TCell)*((size_t)tileCount.x*tileCount.y*compCount));
}

// Calculate average expected color difference for two TileData regions
// where the "color difference" of two pixel is the manhattan difference divided by 3
Dist TileData::getDifferenceDist(TileData *tData1, Vec2<int> offset1, TileData *tData2, Vec2<int> offset2, Vec2<int> compareSize){
	assert(tData1->tileSize.equals(tData2->tileSize));
	assert(tData1->minVal == tData2->minVal && tData1->maxVal == tData2->maxVal);
	assert(tData1->compCount == tData2->compCount);
	
	Vec2<int> workingSize1 = Vec2<int>::sub(tData1->tileCount,offset1);
	Vec2<int> workingSize2 = Vec2<int>::sub(tData2->tileCount,offset2);
	assert(workingSize1.x > 0 && workingSize1.y > 0 && workingSize2.x > 0 && workingSize2.y > 0);
	
	Vec2<int> maxTestSize = Vec2<int>::min(workingSize1,workingSize2);
	assert(compareSize.x <= maxTestSize.x && compareSize.y <= maxTestSize.y);
	
	// For the purposes of this calculation it makes more sense to think of
	// workingDist as a random variable
	Dist workingDist = Dist(0, 0);
	int distCount = 0;
	TCell *cellArr1 = tData1->data;
	TCell *cellArr2 = tData2->data;
	TCell temp1, temp2;
	for(int x=0; x<compareSize.x; x++){
		for(int y=0; y<compareSize.y; y++){
			for(int z=0; z<3; z++){
				// i = 3*(y*width + x) + z;
				temp1 = cellArr1[3*((y+offset1.y)*tData1->tileCount.x + (x+offset1.x)) + z];
				temp2 = cellArr2[3*((y+offset2.y)*tData2->tileCount.x + (x+offset2.x)) + z];
				
				if(!temp1.exists || !temp2.exists){
					// One of the cells has no data so there's no comparison to be made
					continue;
				}
				workingDist.add(Dist::abs(Dist::sub(temp1.dist,temp2.dist))); // Add manhattan color difference
				distCount++;
			}
		}
	}
	
	assert(distCount > 0);
	
	// Return distribution of average difference
	workingDist.div((float)distCount);
	return workingDist;
}