#include "ImgLib.h"
#include <string>
#include <iostream>
#include <fstream>
#include <direct.h>
#include <stdlib.h>
#include <map>
#include <filesystem>

#include "ImageData.h"
#include "UtilF.h"
#include "TileData.h"

using namespace std;

//----------------
// ImgLibEntry
//----------------

ImgLibEntry::ImgLibEntry(){
	imgPath = NULL;
	cacheFolder = NULL;
	useCache = true;
	id = -1;
}

ImgLibEntry::ImgLibEntry(char* imgPath, char* cacheFolder, bool useCache, int id){
	this->imgPath = imgPath;
	this->cacheFolder = cacheFolder;
	this->useCache = useCache;
	this->id = id;
}

TileData* ImgLibEntry::getTileDataFor(Vec2<int> tileSize){
	if (!useCache) {
		LateLoadImg* imgData = new LateLoadImg(imgPath);
		return new TileData(imgData, tileSize);
	}

	char tFilePath[STR_BUFFER_SIZE];
	sprintf(tFilePath,"%s/rData/%d_%dx%d.rdata",cacheFolder,id,tileSize.x,tileSize.y);
	
	LateLoadImg* imgData = new LateLoadImg(imgPath);
	TileData* tData;
	
	if(!UtilF::fileExists(tFilePath)){
		// Get data from source image
		tData = new TileData(imgData,tileSize);
		
		// Save data to cache
		ofstream fileStream;
		fileStream.open(tFilePath, ios::out | ios::binary);
		tData->writeToStream(&fileStream);
		fileStream.close();
	}
	else{
		tData = new TileData();
		
		// Load data from cache
		ifstream fileStream;
		fileStream.open(tFilePath, ios::in | ios::binary);
		tData->readFromStream(&fileStream, imgData);
		fileStream.close();
	}
	return tData;
}

//----------------
// ImgLib
//----------------
ImgLib::ImgLib(const char* folderPath, const char* cacheFolder, bool useCache, bool rebuildImageList){
	if (!useCache) {
		_fullFolderPath = UtilF::getFullPath(folderPath);
		set<string> imagePaths = _getImagesForDir(_fullFolderPath);
		_imgCount = (int)imagePaths.size();
		_entryList = (ImgLibEntry*)malloc(sizeof(ImgLibEntry) * (size_t)_imgCount);
		int i = 0;
		for (auto it = imagePaths.begin(); it != imagePaths.end(); it++) {
			_entryList[i] = ImgLibEntry(UtilF::copyStr((*it).c_str()), NULL, useCache, i);
			i++;
		}
		printf("%d images found\n------\n", _imgCount);
		return;
	}

	printf("------\nSetting up library for: %s\n",folderPath);
	
	// Save original working directory
	char oldWorkingDir[STR_BUFFER_SIZE];
	_getcwd(oldWorkingDir,STR_BUFFER_SIZE);
	
	ifstream inS;
	ofstream outS;
	char tmpStr[STR_BUFFER_SIZE];
	int tmpInt;
	
	_fullFolderPath = UtilF::getFullPath(folderPath);
	char* fullCacheFolderPath = UtilF::getFullPath(cacheFolder);
	_mkdir(fullCacheFolderPath);
	
	// Change working-dir to lib cache folder
	_chdir(fullCacheFolderPath);
	
	const char* libIndexFName = "libIndex.txt";
	
	int libId = -1;
	int highestLibId = -1;
	if(!UtilF::fileExists(libIndexFName)){
		// Create file
		outS.open(libIndexFName, ios::out);
		outS.close();
	}
	inS.open(libIndexFName, ios::in);
	while(inS.peek() != EOF){
		// Read lib-id
		inS.getline(tmpStr,STR_BUFFER_SIZE,' ');
		tmpInt = atoi(tmpStr);
		if(tmpInt > highestLibId) highestLibId = tmpInt;
		
		// Read lib-sourceDirPath
		inS.getline(tmpStr,STR_BUFFER_SIZE,'\n');
		if(strncmp(_fullFolderPath,tmpStr,STR_BUFFER_SIZE) == 0){
			// Index containts entry for this library
			libId = tmpInt;
			break;
		}
	}
	inS.close();
	
	if(libId == -1){
		// Create new library entry
		libId = highestLibId + 1;
		
		// Add entry to index file
		outS.open(libIndexFName, ios::in | ios::app);
		sprintf(tmpStr,"%d %s\n", libId, _fullFolderPath);
		outS.write((const char*)tmpStr, strlen(tmpStr));
		outS.close();
	}
	
	// Adjust cache path to point to lib-specific cache
	sprintf(fullCacheFolderPath,"%s/lib_%d",fullCacheFolderPath,libId);
	
	// Create lib-specific folder if necessary
	_mkdir(fullCacheFolderPath);
	
	// Change working-dir to lib cache folder
	_chdir(fullCacheFolderPath);
	
	_mkdir("rData");
	
	const char* imgIndexFName = "imgIndex.txt";
	if(!UtilF::fileExists(imgIndexFName)){
		// Create basic file
		outS.open(imgIndexFName, ios::out);
		sprintf(tmpStr,"0\n");
		outS.write((const char*)tmpStr, strlen(tmpStr));
		outS.close();
	}
	
	// Get image-paths and id's
	map<string,int> imgMap = map<string,int>();
	set<string> curImgSet = set<string>();
	inS.open(imgIndexFName, ios::in);
	inS.getline(tmpStr,STR_BUFFER_SIZE,'\n');
	int nextImgId = atoi(tmpStr);
	while(inS.peek() != EOF){
		inS.getline(tmpStr,STR_BUFFER_SIZE,' ');
		tmpInt = atoi(tmpStr);
		inS.getline(tmpStr,STR_BUFFER_SIZE,'\n');
		imgMap[string(tmpStr)] = tmpInt;
		curImgSet.insert(string(tmpStr));
	}
	inS.close();
	
	string tmpStrObj;
	if(rebuildImageList){
		printf("Refreshing image-list\n");
		set<string> fullImgSet =  _getImagesForDir(_fullFolderPath);
		set<string> resSet = set<string>();
		
		// Determine images that no longer exist
		for(auto it=curImgSet.begin(); it!=curImgSet.end(); it++){
			tmpStrObj = *it;
			if(fullImgSet.find(tmpStrObj) == fullImgSet.end()){
				resSet.insert(tmpStrObj);
			}
		}

		printf("  %d obsolete images\n", (int)resSet.size());
		// Delete from map
		for(auto it=resSet.begin(); it!=resSet.end(); it++){
			printf(" - %s\n", (*it).c_str());
			imgMap.erase(*it);
		}
		
		// Determine new images
		resSet.clear();
		for(auto it=fullImgSet.begin(); it!=fullImgSet.end(); it++){
			tmpStrObj = *it;
			if(curImgSet.find(tmpStrObj) == curImgSet.end()){
				resSet.insert(tmpStrObj);
			}
		}

		printf("  %d new images\n", (int)resSet.size());
		// Add to map
		for(auto it=resSet.begin(); it!=resSet.end(); it++){
			printf(" + %s\n", (*it).c_str());
			imgMap[*it] = nextImgId;
			nextImgId++;
		}
		
		// Update index file
		outS.open(imgIndexFName, ios::out);
		sprintf(tmpStr,"%d\n",nextImgId);
		outS.write((const char*)tmpStr, strlen(tmpStr));
		for(auto it=imgMap.begin(); it!=imgMap.end(); it++){
			sprintf(tmpStr,"%d %s\n", (*it).second, (*it).first.c_str());
			outS.write((const char*)tmpStr, strlen(tmpStr));
		}
		outS.close();
	}
	
	// At this point imgMap is up-to-date
	_imgCount = (int)imgMap.size();
	_entryList = (ImgLibEntry*)malloc(sizeof(ImgLibEntry)*(size_t)_imgCount);
	int i=0;
	for(auto it=imgMap.begin(); it!=imgMap.end(); it++){
		_entryList[i] = ImgLibEntry(UtilF::copyStr((*it).first.c_str()), fullCacheFolderPath, useCache, (*it).second);
		i++;
	}
	
	// Restore original working directory
	_chdir(oldWorkingDir);
	
	printf("%d images indexed\n------\n",_imgCount);
}

bool isValidImage(filesystem::path fPath) {
	if (!fPath.has_extension() || fPath.extension() == filesystem::path(".txt")) {
		return false;
	}
	return ImageData::canProcessF(fPath.string().c_str());
}

set<string> ImgLib::_getImagesForDir(const char* folderPath){
	set<string> imgPathSet = set<string>();
	
	printf("  Scanning: %s\n", folderPath);

	filesystem::path fPath = filesystem::path(folderPath);
	filesystem::path curPath;
	for (filesystem::recursive_directory_iterator next(fPath), end; next != end; next++) {
		curPath = next->path();
		if (filesystem::is_regular_file(curPath) && isValidImage(curPath)) {
			imgPathSet.insert(filesystem::absolute(curPath).string());
		}
	}
	
	return imgPathSet;
}

int ImgLib::getSize(){
	return _imgCount;
}

TileData* ImgLib::getTileDataFor(int imgIdx, Vec2<int> tileSize){
	return _entryList[imgIdx].getTileDataFor(tileSize);
}
