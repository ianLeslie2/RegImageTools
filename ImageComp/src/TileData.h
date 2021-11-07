#pragma once
#include "global.h"
#include <fstream>
#include "Dist.h"
#include "Vec2.h"
#include "VirtImage.h"
#include "LateLoadImg.h"

#define TDATA_VERSION 4

class VirtualTileSet;

struct TCell{
	Dist dist;
	bool exists;
};

class TileData{
	private:
		inline int _calcIdx(int tileX, int tileY, int compIdx);
		void _freeMemory();
		void _loadTile(Vec2<int> tilePos, Vec2<int> adjTileSize, int compIdx);
		void _loadFromImage(Vec2<int> tileSize,int compCount,float minVal,float maxVal);
		Vec2<int> _calcMaxOffsetFor(Vec2<int> pixelSize);
		float _getVImgPixel(int x, int y, int compIdx);

	public:
		int version;
		Vec2<int> tileSize;
		Vec2<int> tileCount;
		int compCount;
		Vec2<int> imgSize;
		
		float minVal;
		float maxVal;
		
		TCell* data;
		LateLoadImg* vImg;
		
		TileData();
		TileData(LateLoadImg* vImg, Vec2<int> tileSize);
		
		TCell getCell(int x, int y, int compIdx);

		int getSubsetCount(Vec2<int> pixelSize, int tileIncrement);
		int createSubsetsFor(Vec2<int> pixelSize, int tileIncrement, VirtualTileSet* outArray);
		
		void writeToStream(std::ofstream* outStream);
		void readFromStream(std::ifstream* inStream, LateLoadImg* vImgRef);
		
		static Dist getDifferenceDist(TileData *tData1, Vec2<int> offset1, TileData *tData2, Vec2<int> offset2, Vec2<int> compareSize);
};