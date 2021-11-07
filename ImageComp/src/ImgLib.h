#pragma once
#include "global.h"
#include "TileData.h"
#include "Vec2.h"
#include <set>
#include <utility>
#include <string>

class ImgLibEntry{
	public:
		char* imgPath;
		char* cacheFolder;
		bool useCache;
		int id;
		
		ImgLibEntry();
		ImgLibEntry(char* imgPath, char* cacheFolder, bool useCache, int id);
		TileData* getTileDataFor(Vec2<int> tileSize);
};

class ImgLib{
	private:
		char* _fullFolderPath;
		int _imgCount;
		ImgLibEntry* _entryList;
		
		std::set<std::string> _getImagesForDir(const char* folderPath);
	
	public:
		ImgLib(const char* folderPath, const char* cacheFolder, bool useCache, bool rebuildImageList);
		
		int getSize();
		TileData* getTileDataFor(int imgIdx, Vec2<int> tileSize);
};