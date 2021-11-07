#ifndef REGIONMANAGER_H
#define REGIONMANAGER_H

#include "Region.h"
#include "BorderNode.h"
#include <unordered_set>
#include <map>
#include <forward_list>
#include <cstddef>

class RegionManager
{
	private:
		BorderNode** _nodeList;
		int _size;
		int _capacity;
		std::unordered_set<unsigned long long int> _nodeHashSet;
		std::multimap<int, BorderNode*> _bNodeLeftLookup;
		std::multimap<int, BorderNode*> _bNodeRightLookup;
		
		void _swap(int i, int j);
		int _bubbleDown(int idx);
		int _bubbleUp(int idx);
		void _removeAt(int idx, bool removeFromMaps);
		
		void _eraseFromMultiMap(std::multimap<int, BorderNode*>* mapRef, int key, BorderNode* val);
	public:
		RegionManager(int nodeCapacity);
	
		BorderNode* pop();
		void add(BorderNode* n);
		bool contains(BorderNode* n);
		int getSize();
		void updateForMerge(int grownRegionIdx, int absorbedRegionIdx, Region* regionArrayRef);
		
		void debugPrint();
};

#endif