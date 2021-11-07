#include "UtilF.h"
#include <cstdio>
#include <string>
#include <direct.h>
#include <windows.h>
#include "global.h"

bool UtilF::fileExists (const char* fPath) {
	if (FILE *file = fopen(fPath, "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}   
}

bool UtilF::folderExists(const char* fPath) {
	DWORD attribs = GetFileAttributesA(fPath);
	if (attribs == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

char* UtilF::copyStr(const char* strIn){
	size_t sLen = (int)strlen(strIn);
	char* retStr = (char*)malloc(sizeof(char)*(sLen+1));
	strcpy(retStr,strIn);
	return retStr;
}

char* UtilF::getFullPath(const char *fPath){
	char strBuffer[STR_BUFFER_SIZE];
	_fullpath(strBuffer,fPath,STR_BUFFER_SIZE);
	return copyStr(strBuffer);
}

void UtilF::mkDirsForFile(const char* fPath) {
	char* fullPath = getFullPath(fPath);
	int sLen = (int)strlen(fullPath);
	for (int i = 0; i < sLen; i++) {
		if (fullPath[i] == '\\') fullPath[i] = '/';
	}

	int lastSlash = getLastIdx(fullPath, '/', sLen - 1);
	if (lastSlash == -1) return;
	fullPath[lastSlash] = '\0';
	mkDirs(fullPath);
	free(fullPath);
}

void UtilF::mkDirs(const char* fPath) {
	char strBuffer[STR_BUFFER_SIZE];
	_fullpath(strBuffer, fPath, STR_BUFFER_SIZE);

	char tmp;
	size_t pLen = strlen(strBuffer);
	for (int i = 0; i < pLen; i++) {
		if (strBuffer[i] == '/' || strBuffer[i] == '\\') {
			tmp = strBuffer[i];
			strBuffer[i] = '\0';
			_mkdir(strBuffer);
			strBuffer[i] = tmp;
		}
	}
	_mkdir(strBuffer);
}

bool UtilF::strEndsWith(const char *testStr, const char *testEnd){
	size_t mainLen = strlen(testStr);
	size_t endLen = strlen(testEnd);
	if(endLen > mainLen) return false;
	return strcmp(&(testStr[mainLen-endLen]),testEnd) == 0;
}

int UtilF::getLastIdx(const char* str, char c, int startPos) {
	int i = startPos;
	while (i >= 0 && str[i] != c) i--;
	return i;
}

// Return earliest position of search character or position of null-char
int UtilF::getFirstIdx(const char* str, char c, int startPos) {
	int i = startPos;
	while (str[i] != '\0' && str[i] != c) i++;
	return i;
}

char* UtilF::getFName(const char* fPath, bool noExt) {
	int endIdx = getFirstIdx(fPath, '\0', 0);
	int lastSlashIdx = getLastIdx(fPath, '/', endIdx);
	int temp = getLastIdx(fPath, '\\', endIdx);
	if (temp > lastSlashIdx) {
		lastSlashIdx = temp;
	}

	int lastPeriod;
	if (noExt) {
		lastPeriod = getLastIdx(fPath, '.', endIdx);
		if (lastPeriod < lastSlashIdx) {
			lastPeriod = endIdx;
		}
	}
	else {
		lastPeriod = endIdx;
	}

	// Copy region (lastSlashIdx,lastPeriod)
	int len = lastPeriod - (lastSlashIdx + 1);
	char* retV = (char*)malloc((size_t)len + 1);
	for (int i = lastSlashIdx + 1; i < lastPeriod; i++) {
		retV[i - (lastSlashIdx + 1)] = fPath[i];
	}
	retV[len] = '\0';
	return retV;
}