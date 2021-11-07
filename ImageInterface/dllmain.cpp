// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"
#include "dllInterface.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <assert.h>
#include "UtilF.h"

#pragma pack(push, 1)
struct bmFileHeader {
    BYTE id[2];
    uint32_t fSize;
    uint16_t res1;
    uint16_t res2;
    uint32_t imageDataOffset;
};

struct bmInfoHeader {
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;
    uint16_t planes; //1
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize; //width*height*3
    uint32_t resX;
    uint32_t resY;
    uint32_t colorsUsed; //0
    uint32_t importantColors; //0
};

struct bmV5HeaderExtra {
    uint32_t redMask; //   00 00 FF 00
    uint32_t greenMask; // 00 FF 00 00
    uint32_t blueMask; //  FF 00 00 00
    uint32_t alphaMask; // 00 00 00 FF

    DWORD        bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    DWORD        bV5GammaRed;
    DWORD        bV5GammaGreen;
    DWORD        bV5GammaBlue;
    DWORD        bV5Intent;
    DWORD        bV5ProfileData;
    DWORD        bV5ProfileSize;
    DWORD        bV5Reserved;
};

struct pixelData {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};
#pragma pack(pop)

#define WRITE(outVar,varType) fileStream.write(reinterpret_cast<char*>(&outVar), sizeof(varType));
#define READ(outVar,varType) fileStream.read(reinterpret_cast<char*>(&outVar), sizeof(varType));

#define affirm(cond) if(!(cond)) { printf("Err loadBitmap(%s)\n Test failed: " #cond, fPath); exit(1); }

uint8_t* loadBitmap(const char* fPath, int* width, int* height) {
    std::ifstream fileStream;

    fileStream.open(fPath, std::ios::in | std::ios::binary);
    if (fileStream.fail()) {
        printf("Could not open fstream for: %s\n", fPath);
        exit(1);
    }

    bmFileHeader topHeader;
    READ(topHeader, bmFileHeader);
    affirm(topHeader.id[0] == 'B' && topHeader.id[1] == 'M');

    bmInfoHeader infoHeader;
    READ(infoHeader,bmInfoHeader);
    affirm(infoHeader.bitsPerPixel == 24);
    affirm(infoHeader.compression == 0);
    affirm(infoHeader.headerSize == 40 || infoHeader.headerSize == 124);
    if (infoHeader.headerSize == 124) {
        bmV5HeaderExtra extraHeader;
        READ(extraHeader, bmV5HeaderExtra);

        affirm(extraHeader.redMask   == 0x00FF0000);
        affirm(extraHeader.greenMask == 0x0000FF00);
        affirm(extraHeader.blueMask  == 0x000000FF);
        if (extraHeader.bV5CSType != LCS_sRGB) {
            switch (extraHeader.bV5CSType) {
            case LCS_CALIBRATED_RGB:
                printf("Invalid Space: LCS_CALIBRATED_RGB\n");
                printf("Path:%s\n", fPath);
                exit(1);
            case LCS_WINDOWS_COLOR_SPACE:
                printf("Invalid Space: LCS_WINDOWS_COLOR_SPACE\n");
                printf("Path:%s\n", fPath);
                exit(1);
            case PROFILE_LINKED:
                printf("Invalid Space: PROFILE_LINKED\n");
                printf("Path:%s\n", fPath);
                exit(1);
            case PROFILE_EMBEDDED:
                printf("Invalid Space: PROFILE_EMBEDDED\n");
                printf("Path:%s\n", fPath);
                exit(1);
            default:
                printf("Invalid Space: <unknown>\n");
                printf("Path:%s\n", fPath);
                exit(1);
            }
        }
        affirm(extraHeader.bV5CSType == LCS_sRGB);
    }
    
    // Move to image data section
    fileStream.seekg(topHeader.imageDataOffset, SEEK_SET);

    uint8_t* outData = (uint8_t*)malloc((size_t)infoHeader.width * infoHeader.height * 4);
    pixelData tmp;
    for (uint32_t i = 0; i < infoHeader.width * infoHeader.height; i++) {
        READ(tmp,pixelData);
        outData[4 * i + 0] = tmp.red;
        outData[4 * i + 1] = tmp.green;
        outData[4 * i + 2] = tmp.blue;
        outData[4 * i + 3] = 255; //alpha
    }
    fileStream.close();

    *width = infoHeader.width;
    *height = infoHeader.height;
    return outData;
}

void saveBitmap(const char* fPath, uint8_t* data, int width, int height) {
    std::ofstream fileStream;

    fileStream.open(fPath, std::ios::out | std::ios::binary);
    if (fileStream.fail()) {
        printf("Could not open fstream for: %s\n", fPath);
        exit(1);
    }

    bmFileHeader topHeader;
    topHeader.id[0] = 'B';
    topHeader.id[1] = 'M';
    topHeader.fSize = sizeof(bmFileHeader) + sizeof(bmInfoHeader) + ((size_t)width * height * 3);
    topHeader.res1 = 0;
    topHeader.res2 = 0;
    topHeader.imageDataOffset = sizeof(bmFileHeader) + sizeof(bmInfoHeader);
    WRITE(topHeader, bmFileHeader);

    bmInfoHeader infoHeader;
    infoHeader.headerSize = sizeof(bmInfoHeader);
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = 24;
    infoHeader.compression = 0;
    infoHeader.imageSize = width*height*3;
    infoHeader.resX = 0;
    infoHeader.resY = 0;
    infoHeader.colorsUsed = 0;
    infoHeader.importantColors = 0;
    WRITE(infoHeader, bmInfoHeader);

    pixelData tmp;
    for (int i = 0; i < width * height; i++) {
        tmp.red   = data[4 * i + 0];
        tmp.green = data[4 * i + 1];
        tmp.blue  = data[4 * i + 2];
        // Alpha is ignored
        WRITE(tmp, pixelData);
    }
    fileStream.close();
}

char* magickPath = NULL;

char* getMagickExePath() {
    const DWORD maxBufferSize = 32767 + 1;
    char* envBuffer = (char*)malloc(maxBufferSize);
    envBuffer[0] = ';';
    DWORD loadedSize = GetEnvironmentVariableA("PATH", envBuffer + 1, maxBufferSize);
    if (loadedSize == 0) {
        printf("Could not read PATH variable to find Magick\n");
        free(envBuffer);
        return NULL;
    }
    loadedSize++;

    const char* testPat = "ImageMagick-";
    const char* toolName = "magick.exe";
    int patLen = (int)strlen(testPat);
    int lastPathEnd = loadedSize - 1;
    int lastSlash = -1;
    // Go through list in reverse order so that later installation
    // are matched first
    for (int i = loadedSize - 2; i >=0; i--) {
        if (envBuffer[i] == '\\') envBuffer[i] = '/';
        if (envBuffer[i] == '/' && i != lastPathEnd-1) {
            lastSlash = i;
        }

        if (envBuffer[i] == ';') {
            if (lastSlash < lastPathEnd && lastPathEnd - lastSlash > patLen && strncmp(envBuffer + lastSlash + 1, testPat, patLen) == 0) {
                // Found path to image-magick!
                envBuffer[lastPathEnd] = '\0';
                int lastPathStart = i+1;
                int toolPathLen = lastPathEnd - lastPathStart + (envBuffer[lastPathEnd - 1] == '/' ? 0 : 1) + (int)strlen(toolName) + 1; //+1 for terminal char
                char* retV = (char*)malloc(toolPathLen);
                if (envBuffer[lastPathEnd - 1] == '/') {
                    sprintf(retV, "%s%s", envBuffer + lastPathStart, toolName);
                }
                else {
                    sprintf(retV, "%s/%s", envBuffer + lastPathStart, toolName);
                }
                free(envBuffer);
                return retV;
            }
            envBuffer[i] = '\0';
            lastPathEnd = i;
        }
    }

    free(envBuffer);
    return NULL;
}

char* getTempFilePath() {
    char dirBuf[MAX_PATH];
    char pathBuf[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, dirBuf)) {
        printf("Err getting tmp file path\n");
        exit(1);
    }

    char prefix = '\0';
    if (!GetTempFileNameA(dirBuf, &prefix, 0, pathBuf)) {
        printf("Err getting tmp file path\n");
        exit(1);
    }
    char* retV = UtilF::copyStr(pathBuf);
    UtilF::mkDirs(dirBuf);
    return retV;
}

void runCmd(char* cmd) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(
        magickPath,
        cmd,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi)
        ) {
        printf("Couldn't create process to run: %s %s\n", magickPath, cmd);
        exit(1);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void init() {
    magickPath = getMagickExePath();
    if (magickPath == NULL) {
        printf("Err: Could not find Image-Magick installation via PATH variable.\n");
        exit(1);
    }
    //printf("Tool Path: >%s<\n", magickPath);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        init();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#define MAX_EXT_LEN 6

bool canProcess(const char* fPath) {
    int sLen = (int)strlen(fPath);
    int lastPeriod = UtilF::getLastIdx(fPath, '.', sLen);
    if (lastPeriod == -1 || sLen - lastPeriod > MAX_EXT_LEN) {
        return false;
    }

    char ext[MAX_EXT_LEN + 1];
    strncpy(ext, fPath + lastPeriod + 1, MAX_EXT_LEN + 1);

    char* tmpPath = getTempFilePath();
    char cmdBuf[MAX_PATH * 2];
    sprintf(cmdBuf, " -size 1x1 canvas:none %s:\"%s\"", ext, tmpPath);
    runCmd(cmdBuf);

    if (UtilF::fileExists(tmpPath)) {
        remove(tmpPath);
        free(tmpPath);
        return true;
    }
    free(tmpPath);
    return false;
}

uint8_t* readData(const char* fPath, int* width, int* height) {
    char* tmpPath = getTempFilePath();
    char cmdBuf[MAX_PATH * 3];
    sprintf(cmdBuf, " \"%s\" -depth 8 -colorspace sRGB -type TrueColor bmp3:\"%s\"", fPath, tmpPath);
    runCmd(cmdBuf);

    if (!UtilF::fileExists(tmpPath)) {
        printf("Err reading file, tmp file does not exist: %s\n",fPath);
        exit(1);
    }

    uint8_t* data = loadBitmap(tmpPath, width, height);
    remove(tmpPath);
    free(tmpPath);
    return data;
}

void writeData(const char* fPath, int width, int height, uint8_t* data) {
    UtilF::mkDirsForFile(fPath);

    char* tmpPath = getTempFilePath();
    saveBitmap(tmpPath, data, width, height);

    char cmdBuf[MAX_PATH * 3];
    sprintf(cmdBuf, " \"%s\" \"%s\"", tmpPath, fPath);
    runCmd(cmdBuf);

    if (!UtilF::fileExists(fPath)) {
        printf("Err writing file, target file does not exist: %s\n", fPath);
        exit(1);
    }
    remove(tmpPath);
    free(tmpPath);
}

