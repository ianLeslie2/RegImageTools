#include "VirtualTileSet.h"
#include "UtilF.h"

VirtualTileSet::VirtualTileSet(TileData* sourceData){
	_sourceRef = sourceData;
	_offset = Vec2<int>();
	_size = sourceData->tileCount;
}

VirtualTileSet::VirtualTileSet(TileData* sourceData, Vec2<int> offset, Vec2<int> size){
	_sourceRef = sourceData;
	_offset = offset;
	_size = size;
}
		
Dist VirtualTileSet::getDifferenceDist(VirtualTileSet* other){
	assert(_size.equals(other->_size));
	return TileData::getDifferenceDist(_sourceRef,_offset,other->_sourceRef,other->_offset,_size);
}

VirtImageTrans VirtualTileSet::getVirtImageTrans(){
	Vec2<int> tSize = _sourceRef->tileSize;
	Vec2<int> pos = Vec2<int>::compMult(_offset,tSize);
	Vec2<int> size = Vec2<int>::min(
							Vec2<int>::compMult(_size,tSize),
							Vec2<int>::sub(_sourceRef->vImg->getSize(),pos));
	return VirtImageTrans(_sourceRef->vImg, pos, size);
}
