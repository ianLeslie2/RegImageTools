#pragma once
#include <cstdint>

extern "C" __declspec(dllexport) bool canProcess(const char* fPath);
extern "C" __declspec(dllexport) uint8_t* readData(const char* fPath, int* width, int* height);
extern "C" __declspec(dllexport) void writeData(const char* fPath, int width, int height, uint8_t* data);