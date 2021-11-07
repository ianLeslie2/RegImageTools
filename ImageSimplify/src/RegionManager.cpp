#include "RegionManager.h"
#include "Color.h"
#include <iostream>
#include <assert.h>

RegionManager::RegionManager(int nodeCapacity){
	_nodeList = (BorderNode**)malloc(sizeof(BorderNode*)*nodeCapacity);
	_capacity = nodeCapacity;
	_size = 0;
	
	_nodeHashSet = std::unordered_set<unsigned long long int>();
	// Border nodes stored by r1Idx key
	_bNodeLeftLookup = std::multimap<int, BorderNode*>();
	// Border nodes stored by r2Idx key
	_bNodeRightLookup = std::multimap<int, BorderNode*>();
}


/* Note:
 	_nodeList is an array that represents a binary min-heap (sorted on BorderNode.diff)
	If a node has index x, then:
		parent index = floor((x-1)/2)
		left child idx = 2x + 1
		right child idx = 2x + 2
*/

inline void RegionManager::_swap(int i, int j){
	BorderNode* temp = _nodeList[i];
	_nodeList[i] = _nodeList[j];
	_nodeList[j] = temp;
	_nodeList[i]->heapIdx = i;
	_nodeList[j]->heapIdx = j;
}

// Returns index of bubbled node
int RegionManager::_bubbleDown(int idx){
	int leftIdx, rightIdx;
	float leftVal, rightVal, mainVal;
	
	while(true){
		leftIdx = 2*idx + 1;
		rightIdx = 2*idx + 2;
		if (leftIdx >= _size){
			// This element has no children and is already at the bottom
			return idx;
		}
		else if(rightIdx >= _size){
			if( _nodeList[leftIdx]->diff < _nodeList[idx]->diff ){
				_swap(leftIdx, idx);
				return leftIdx;
			}
			return idx;
		}
		leftVal = _nodeList[leftIdx]->diff;
		rightVal = _nodeList[rightIdx]->diff;
		mainVal = _nodeList[idx]->diff;
		if(leftVal < mainVal){
			if(rightVal < leftVal){
				// Bubble down right
				_swap(rightIdx, idx);
				idx = rightIdx;
			}
			else{
				// Bubble down left
				_swap(leftIdx, idx);
				idx = leftIdx;
			}
		}
		else if(rightVal < mainVal){
			// Since leftVal <= mainVal, and mainVal < rightVal
			//	we know leftVal < rightVal
			// Bubble down right
			_swap(rightIdx, idx);
			idx = rightIdx;
		}
		else{
			// min-heap property restored
			return idx;
		}
	}
}

// Returns index of bubbled node
int RegionManager::_bubbleUp(int idx){
	int parentIdx = (idx-1)/2;
	while(idx != 0 && _nodeList[parentIdx]->diff > _nodeList[idx]->diff){
		// Move up
		_swap(parentIdx, idx);
		idx = parentIdx;
		parentIdx = (idx-1)/2;
	}
	return idx;
}

// https://stackoverflow.com/questions/3952476/how-to-remove-a-specific-pair-from-a-c-multimap
void RegionManager::_eraseFromMultiMap(std::multimap<int, BorderNode*>* mapRef, int key, BorderNode* val){
	typedef std::multimap<int, BorderNode*>::iterator iterator;
	std::pair<iterator, iterator> iterPair = mapRef->equal_range(key);
	
	for(iterator it = iterPair.first; it != iterPair.second; ++it){
		if(it->second == val){
			mapRef->erase(it);
			return;
		}
	}
	
	printf("\nWARN: Failed to remove (%d,(%d,%d))\n",key,val->r1Idx,val->r2Idx);
}

// Note: Region object does not get deconstructed
void RegionManager::_removeAt(int idx, bool removeFromMaps){
	if(idx == -1){
		throw std::runtime_error("Can't remove from index -1. Node may have had uninitialized heapIdx");
	}
	if(idx < 0 || idx >= _size){
		printf("\nIdx: %d\n",idx);
		printf("_size: %d\n",_size);
		printf("_capacity: %d\n",_capacity);
		throw std::runtime_error("Invalid index");
	}
	
	// Remove item from hash-set
	_nodeHashSet.erase(_nodeList[idx]->uniqueVal);
	
	if(removeFromMaps){
		_eraseFromMultiMap(&_bNodeLeftLookup,  _nodeList[idx]->r1Idx, _nodeList[idx]);
		_eraseFromMultiMap(&_bNodeRightLookup, _nodeList[idx]->r2Idx, _nodeList[idx]);
	}
	
	/* Replace target node with last item in tree
	  and decrease count to functionally delete the
	  previous entry
	*/
	_swap(idx,_size-1);
	_size--;
	
	// Reset index tracker
	_nodeList[_size]->heapIdx = -1;
	
	if(idx == _size){
		// Item was last in the list
		return;
	}
	
	// Rebalance heap as required
	// (at most one of these function will actually move the node)
	_bubbleUp(idx);
	_bubbleDown(idx);
}

BorderNode* RegionManager::pop(){
	if(_size==0){
		throw std::runtime_error("Can't pop from empty list.");
	}
	BorderNode* topItem = _nodeList[0];
	_removeAt(0,true);
	return topItem;
}

void RegionManager::add(BorderNode* n){
	if(_size==_capacity){
		throw std::runtime_error("Can't add element, _nodeList already at max capacity.");
	}
	if(contains(n)){
		//printf("Blocked duplicate");
		return;
	}
	
	_nodeHashSet.insert(n->uniqueVal);
	_bNodeLeftLookup.emplace(n->r1Idx,n);
	_bNodeRightLookup.emplace(n->r2Idx,n);
	
	_size++;
	_nodeList[_size-1] = n;
	n->heapIdx = _size-1;
	_bubbleUp(_size-1);
}

bool RegionManager::contains(BorderNode* n){
	return _nodeHashSet.find(n->uniqueVal) != _nodeHashSet.end();
}

int RegionManager::getSize(){
	return _size;
}

void RegionManager::debugPrint(){
	printf("[");
	for(int i=0; i<_size; i++){
		printf("%f,",_nodeList[i]->diff);
	}
	printf("]");
}

// Removes invalidated border nodes and adds new ones
void RegionManager::updateForMerge(int grownRegionIdx, int absorbedRegionIdx, Region* regionArrayRef){
	BorderNode* t;
	std::forward_list<BorderNode*> adjustItems = std::forward_list<BorderNode*>();
	typedef std::multimap<int, BorderNode*>::iterator mapIterator;
	std::pair<mapIterator,mapIterator> iterPair;
	
	/*
	 * Get all border nodes that reference one of the changed regions
	 * and add them to adjustItems as well as removing them from the heap
	 */
	
	// Where t->r1Idx == grownRegionIdx
	iterPair = _bNodeLeftLookup.equal_range(grownRegionIdx);
	for(mapIterator it = iterPair.first; it != iterPair.second; ++it){
		t = it->second;
		if(t->r1Idx != grownRegionIdx || t->heapIdx < 0 || t->heapIdx >= _size || _nodeList[t->heapIdx] != t){
			// Dead border-node
			continue;
		}
		adjustItems.push_front(t);
		_removeAt(t->heapIdx,false);
	}
	
	// Where t->r1Idx == absorbedRegionIdx
	iterPair = _bNodeLeftLookup.equal_range(absorbedRegionIdx);
	for(mapIterator it = iterPair.first; it != iterPair.second; ++it){
		t = it->second;
		if(t->r1Idx != absorbedRegionIdx || t->heapIdx < 0 || t->heapIdx >= _size || _nodeList[t->heapIdx] != t){
			// Dead border-node
			continue;
		}
		adjustItems.push_front(t);
		_removeAt(t->heapIdx,false);
	}
	
	// Where t->r2Idx == grownRegionIdx
	iterPair = _bNodeRightLookup.equal_range(grownRegionIdx);
	for(mapIterator it = iterPair.first; it != iterPair.second; ++it){
		t = it->second;
		if(t->r2Idx != grownRegionIdx || t->heapIdx < 0 || t->heapIdx >= _size || _nodeList[t->heapIdx] != t){
			// Dead border-node
			continue;
		}
		adjustItems.push_front(t);
		_removeAt(t->heapIdx,false);
	}
	
	// Where t->r2Idx == absorbedRegionIdx
	iterPair = _bNodeRightLookup.equal_range(absorbedRegionIdx);
	for(mapIterator it = iterPair.first; it != iterPair.second; ++it){
		t = it->second;
		if(t->r2Idx != absorbedRegionIdx || t->heapIdx < 0 || t->heapIdx >= _size || _nodeList[t->heapIdx] != t){
			// Dead border-node
			continue;
		}
		adjustItems.push_front(t);
		_removeAt(t->heapIdx,false);
	}
	
	/*
		Mostly remove the nodes from the multimaps
		A node such as (something,grownRegionIdx) would
		still be present in the _bNodeLeftLookup multimap
		until a merge happens that involves "something"
	*/
	_bNodeLeftLookup.erase(grownRegionIdx);
	_bNodeLeftLookup.erase(absorbedRegionIdx);
	_bNodeRightLookup.erase(grownRegionIdx);
	_bNodeRightLookup.erase(absorbedRegionIdx);
	
	
	int oldRegion;
	while(!adjustItems.empty()){
		t = adjustItems.front();
		adjustItems.pop_front();
		
		if(t->r1Idx == grownRegionIdx){
			oldRegion = t->r2Idx;
		}
		else if(t->r2Idx == grownRegionIdx){
			oldRegion = t->r1Idx;
		}
		else if(t->r1Idx == absorbedRegionIdx){
			oldRegion = t->r2Idx;
		}
		else{
			// t->r2Idx == absorbedRegionIdx
			oldRegion = t->r1Idx;
		}
		
		/* For the sake of performance, it's important the BorderNode
			space can be reused. Although not explicit, I'm 99% sure
			the compiler recycles the BorderNode space instead of
			deallocation and then re-allocating.
		 */
		delete t;
		t = new BorderNode(oldRegion,grownRegionIdx);
		
		// Calculate color-error taking into account region sizes
		t->diff = Region::getColorDiff(
									&(regionArrayRef[t->r1Idx]),
									&(regionArrayRef[t->r2Idx]) );
		
		
		// Add new border node to heap
		add(t);
	}
	// End of while loop
}
