#include "RegData.h"
#include "zlib.h"

//#pragma comment(lib,"libz-static.lib")

#define CUR_REG_VERSION 5

typedef uint32_t intType;

#define WRITE_S(varName) fileStream->write(reinterpret_cast<char*>(&(varName)), sizeof(varName));
#define READ_S(varName) fileStream->read(reinterpret_cast<char*>(&(varName)), sizeof(varName));

#define WRITE_BUF(varName) memcpy(outBuffer + bufOffset,&(varName),sizeof(varName)); bufOffset += sizeof(varName);
#define READ_BUF(varName) memcpy(&(varName),inBuffer + bufOffset,sizeof(varName)); bufOffset += sizeof(varName);

// Note: I'm deliberately leaving __FILE__ out of the macro since I'd like to use this macro for general error reporting
//			and I prefer not leaking full paths from my PC
#define affirm(cond) if(!(cond)) { printf("Err processing RegData format (line %d). Failed check: " #cond, __LINE__); exit(1); }

int RegData::getVersion() {
	return CUR_REG_VERSION;
}

// This constructor requires an external source to configure the stored data
RegData::RegData(int w, int h, int regionCount) {
	width = w;
	height = h;
	rCount = regionCount;
	rgbColors = (Color*)malloc(rCount * sizeof(Color));
	idxList = (int*)malloc((size_t)width * height * sizeof(int));
}

RegData::RegData(const char* inPath) {
	std::ifstream fileStream;
	fileStream.open(inPath, std::ios::out | std::ios::binary);
	try {
		_loadFrom(&fileStream);
	}
	catch (...) {
		fileStream.close();
		throw;
	}
	fileStream.close();
}

RegData::RegData(std::ifstream* fileStream) {
	_loadFrom(fileStream);
}

void RegData::_loadFrom(std::ifstream* fileStream) {
	intType version;
	READ_S(version);
	affirm(version == CUR_REG_VERSION);

	intType rawSize;
	intType compressedSize;
	READ_S(rawSize);
	READ_S(compressedSize);

	char* inBuffer = (char*)malloc(rawSize);
	int bufOffset = 0;
	char* compressedData = (char*)malloc(compressedSize);
	fileStream->read(compressedData, compressedSize);

	uLongf actualRawSize = rawSize;
	int res = uncompress((Bytef*)inBuffer, &actualRawSize, (Bytef*)compressedData, compressedSize);
	affirm(res == Z_OK);
	affirm(actualRawSize == rawSize);

	intType rCount, width, height;
	READ_BUF(rCount);
	READ_BUF(width);
	READ_BUF(height);
	this->rCount = (int)rCount;
	this->width = (int)width;
	this->height = (int)height;

	// Load region colors
	rgbColors = (Color*)malloc(rCount * sizeof(Color));
	intType x, y, z;
	for (intType i = 0; i < rCount; i++) {
		READ_BUF(x);
		READ_BUF(y);
		READ_BUF(z);
		rgbColors[i] = Color(x, y, z);
	}

	// Load index data
	idxList = (int*)malloc((size_t)width * height * sizeof(int));
	intType tmp;
	for (intType i = 0; i < width*height; i++) {
		READ_BUF(tmp);
		idxList[i] = (int)tmp;
	}

	affirm(bufOffset == rawSize);

	free(compressedData);
	free(inBuffer);
}

RegData::~RegData() {
	free(rgbColors);
	free(idxList);
}

int RegData::_uncompressedRegionDataSize() {
	const int headerSize = sizeof(int) * 3; //rCount, width, height
	const int colorSize = sizeof(int) * 3;
	const int regEntrySize = sizeof(int);
	return headerSize + colorSize * rCount + regEntrySize * width * height;
}


void RegData::saveTo(const char* outPath) {
	std::ofstream fileStream;
	fileStream.open(outPath, std::ios::out | std::ios::binary);
	try {
		saveTo(&fileStream);
	}
	catch (...) {
		fileStream.close();
		throw;
	}
	fileStream.close();
}

intType _colorV(double v) {
	int tmp = (int)std::round(v);
	if (tmp < 0) return 0;
	if (tmp > 255) return 255;
	return (intType)tmp;
}

void RegData::saveTo(std::ofstream* fileStream) {
	intType version = (intType)getVersion();
	WRITE_S(version);

	intType rawSize = (intType)_uncompressedRegionDataSize();
	char* outBuffer = (char*)malloc(rawSize);
	int bufOffset = 0;

	intType rCount, width, height;
	rCount = (intType)this->rCount;
	width = (intType)this->width;
	height = (intType)this->height;
	WRITE_BUF(rCount);
	WRITE_BUF(width);
	WRITE_BUF(height);

	// Write colors
	intType x,y,z;
	for (intType i = 0; i < rCount; i++) {
		x = _colorV(rgbColors[i].x);
		y = _colorV(rgbColors[i].y);
		z = _colorV(rgbColors[i].z);
		WRITE_BUF(x);
		WRITE_BUF(y);
		WRITE_BUF(z);
	}

	// Write idx data
	intType tmp;
	for (intType i = 0; i < width * height; i++) {
		tmp = (intType)idxList[i];
		WRITE_BUF(tmp);
	}
	affirm(bufOffset == rawSize);

	char* compressedBuffer = (char*)malloc(rawSize);
	uLongf compressedSize = rawSize;
	int res = compress((Bytef*)compressedBuffer, &compressedSize, (Bytef*)outBuffer, rawSize);
	affirm(res == Z_OK);
	intType compressedSizeInt = compressedSize;

	WRITE_S(rawSize);
	WRITE_S(compressedSizeInt);
	fileStream->write(compressedBuffer, compressedSizeInt);

	free(outBuffer);
	free(compressedBuffer);
}

int RegData::_idx(int x, int y) {
	return width * y + x;
}

int RegData::getRegionIdx(int x, int y) {
	return idxList[_idx(x, y)];
}

ImageData* RegData::genImage() {
	ImageData* obj = new ImageData(width, height);
	int i = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			obj->setPixel(x, y, rgbColors[idxList[i++]]);
		}
	}
	return obj;
}

void RegData::snapshot(const char* outPath) {
	ImageData* obj = genImage();
	obj->saveTo(outPath);
	delete obj;
}