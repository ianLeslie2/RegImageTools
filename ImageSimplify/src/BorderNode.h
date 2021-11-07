#ifndef BORDERNODE_H
#define BORDERNODE_H

#include <cstddef>

// All properties are public but shouldn't be modified after
// initialization
class BorderNode
{
	public:
		int r1Idx;
		int r2Idx;
		float diff;
		int heapIdx;
		
		/* The unique value generate has a size of O(r1Idx^2) essentially
		   and r1Idx is basically O(pixelCount)
		   Unsigned long long int has a max of ~10^19, allowing
		   r1Idx a max of ~10^9.5, and giving image dimensions a max of 10^4.75
		   
		   So, as long as image dimensions don't exceed 4 digits this method is fine
		   Still,
		   TODO find a more generic alternative
		*/
		unsigned long long int uniqueVal;
		
		// After initialization r1Idx <= r2Idx
		// Order given in constructor may not match order of properties
		BorderNode(int unorderedIdxA, int unorderedIdxB);
};

#endif