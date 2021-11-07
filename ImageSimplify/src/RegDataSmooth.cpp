#include "RegDataSmooth.h"
#include <cassert>
#include <cmath>

int RegDataSmooth::_getIdxForHighest(float* regWeights,int cnt){
	int highIdx = 0;
	for(int i=1; i<cnt; i++){
		if(regWeights[i] > regWeights[highIdx]){
			highIdx = i;
		}
	}
	return highIdx;
}

int RegDataSmooth::_calcIdx(int x, int y, int width) {
	return y * width + x;
}

void RegDataSmooth::smoothPass(RegData* rData, int smoothStr){
	int compRadius = smoothStr;
	int j;
	
	float *vals = (float*)malloc(sizeof(float)*(((size_t)compRadius*2 + 1)*(compRadius*2 + 1)));
	j = 0;
	for(int yOff=-compRadius; yOff<=compRadius; yOff++){
		for(int xOff=-compRadius; xOff<=compRadius; xOff++){
			if(xOff == 0 && yOff == 0){
				vals[j] = 0.01f; // Tiebreaker essentially
			}
			else{
				vals[j] = 1.0f/(float)sqrt(xOff*xOff + yOff*yOff);
			}
			j++;
		}
	}

	int width = rData->width;
	int height = rData->height;
	int* idxListBuffer = (int*)malloc((size_t)width * height * sizeof(int));
	float* regSums = (float*)malloc(rData->rCount*sizeof(float));

	int i = 0;
	for(int y=0; y<height; y++){
		for(int x=0; x<width; x++){
			// Reset weight tracker
			for(j=0; j<rData->rCount; j++){
				regSums[j] = 0;
			}
			
			j = 0;
			for(int yOff=-compRadius; yOff<=compRadius; yOff++){
				for(int xOff=-compRadius; xOff<=compRadius; xOff++){
					if(x + xOff >= 0 && x + xOff <width && y + yOff >= 0 && y + yOff <height){
						regSums[ rData->idxList[_calcIdx(x+xOff,y+yOff,width)] ] += vals[j];
					}
					j++;
				}
			}
			
			idxListBuffer[i] = _getIdxForHighest(regSums,rData->rCount);
			
			i++;
		}
	}
	
	// Replace buffer
	free(rData->idxList);
	rData->idxList = idxListBuffer;
	
	free(vals);
	free(regSums);
}


