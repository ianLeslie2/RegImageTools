#pragma once
#include "global.h"

#undef min
#undef max

class UtilF{
	public:
		template<class T>
		inline static T min(T a, T b){
			return (a < b) ? a : b;
		}
		
		template<class T>
		inline static T max(T a, T b){
			return (a < b) ? b : a;
		}
		
		template<class T>
		inline static T abs(T a){
			return (a > 0) ? a : -a;
		}
		
		static bool fileExists (const char* fPath);
		static bool folderExists(const char* fPath);
		static char* copyStr(const char* strIn);
		static char* getFullPath(const char *fPath);
		static void mkDirsForFile(const char* fPath);
		static void mkDirs(const char* fPath);
		static bool strEndsWith(const char *testStr, const char *testEnd);
		static int getLastIdx(const char* str, char c, int startPos);
		static int getFirstIdx(const char* str, char c, int startPos);
		static char* getFName(const char* fPath, bool noExt);
};