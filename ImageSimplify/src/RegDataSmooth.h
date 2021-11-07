#pragma once
#include "RegData.h"

class RegDataSmooth{
	private:
		static int _getIdxForHighest(float* regWeights,int cnt);
		static inline int _calcIdx(int x, int y, int width);
	
	public:
		static void smoothPass(RegData* rData, int smoothStr);
};