#pragma once
#include "global.h"
#include "Vec2.h"
#include "Dist.h"
#include "TileData.h"
#include "VirtImage.h"
#include "VirtImageTrans.h"

class VirtualTileSet{
	private:
		TileData* _sourceRef;
		Vec2<int> _offset;
		Vec2<int> _size;
	
	public:
		VirtualTileSet(TileData* sourceData);
		VirtualTileSet(TileData* sourceData, Vec2<int> offset, Vec2<int> size);
		
		Dist getDifferenceDist(VirtualTileSet* other);
		VirtImageTrans getVirtImageTrans();
};