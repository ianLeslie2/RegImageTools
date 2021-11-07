#include "BorderNode.h"

BorderNode::BorderNode(int unorderedIdxA, int unorderedIdxB){
	if(unorderedIdxA <= unorderedIdxB){
		r1Idx = unorderedIdxA;
		r2Idx = unorderedIdxB;
	}
	else{
		r1Idx = unorderedIdxB;
		r2Idx = unorderedIdxA;
	}
	diff = 0;
	heapIdx = -1;
	
	// https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
	uniqueVal = (((unsigned long long int)(r1Idx + r2Idx))*((unsigned long long int)(r1Idx + r2Idx + 1)))/2 + r2Idx;
}