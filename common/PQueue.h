/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    PQueue.h
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License.  You may
 obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.

 ***************************************************************************/

#ifndef INCPQueue_h
#define INCPQueue_h

#include <vector>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

namespace zmdp {

template <typename ValueType, typename PriorityType,
	  typename CompareType = std::less<PriorityType> >
class PQueue {
protected:
  typedef size_t HeapIndex;
  struct PQEntry {
    ValueType v;
    PriorityType prio;
    HeapIndex heapIndex;
    PQEntry(ValueType _v, PriorityType _prio, HeapIndex _heapIndex) :
      v(_v), prio(_prio), heapIndex(_heapIndex)
    {}
  };
  typedef EXT_NAMESPACE::hash_map<ValueType, PQEntry*> Lookup;

public:
  PQueue(void) {}
  CompareType comp;

  const ValueType& top(void) const {
    return heap[0]->v;
  }
  const PriorityType& getTopPriority(void) const {
    return heap[0]->prio;
  }
  bool contains(const ValueType& v) const { return (lookup.find(v) != lookup.end()); }
  size_t size(void) const { return heap.size(); }
  bool empty(void) const { return heap.empty(); }

  void setPriority(const ValueType& v, const PriorityType& prio) {
    typeof(lookup.begin()) pr = lookup.find(v);
    if (lookup.end() == pr) {
      PQEntry* e = new PQEntry(v, prio, heap.size());
      heap.push_back(e);
      lookup[v] = e;
      _upHeap(e->heapIndex);
    } else {
      PQEntry* e = pr->second;
      e->prio = prio;
      _heapify(e->heapIndex);
    }
  }
  void pop(void) {
    _erase(heap[0]);
  }
  void erase(const ValueType& v) {
    _erase(lookup[v]);
  }
  void clear(void) {
    FOR_EACH (eltp, heap) {
      delete *eltp;
    }
    heap.clear();
    lookup.clear();
  }

  // useful for debugging
  void printEntry(PQEntry* e) const {
    std::cout << e->v << " " << e->prio << " " << e->heapIndex;
  }
  void print(void) const {
    std::cout << "heap:" << std::endl;
    FOR (i, heap.size()) {
      std::cout << "  " << i << " ";
      printEntry(heap[i]);
      std::cout << std::endl;
    }
#if 0
    std::cout << "lookup:" << std::endl;
    FOR_EACH (pr, lookup) {
      std::cout << "  " << pr->first << " ";
      printEntry(pr->second);
      std::cout << std::endl;
    }
#endif
  }
  
protected:
  std::vector<PQEntry*> heap;
  Lookup lookup;

  int _getParentIndex(HeapIndex index) {
    return (index-1)/2;
  }
  PQEntry* _getEntry(HeapIndex index) {
    if (index < heap.size()) {
      return heap[index];
    } else {
      return NULL;
    }
  }
  void _swap(HeapIndex index1, HeapIndex index2) {
    PQEntry*& e1 = heap[index1];
    PQEntry*& e2 = heap[index2];
    std::swap(e1,e2);
    std::swap(e1->heapIndex, e2->heapIndex);
  }
  // if no change return 0 else return index of child that
  // swapped with parent
  HeapIndex _bobble(HeapIndex parentIndex) {
    HeapIndex i0 = parentIndex*2 + 1;
    HeapIndex i1 = parentIndex*2 + 2;
    PQEntry* parent = _getEntry(parentIndex);
    PQEntry* child0 = _getEntry(i0);
    PQEntry* child1 = _getEntry(i1);
    if (child0 && comp(child0->prio, parent->prio)) {
      if (child1 && comp(child1->prio, child0->prio)) {
	_swap(parentIndex, i1);
	return i1;
      } else {
	_swap(parentIndex, i0);
	return i0;
      }
    } else {
      if (child1 && comp(child1->prio, parent->prio)) {
	_swap(parentIndex, i1);
	return i1;
      } else {
	return 0;
      }
    }
    return 0; // avoid bogus warning (never reach this point)
  }
  bool _upHeap(int index) {
    int changeCount = 0;
    while (1) {
      if (0 == index) {
	// reached root, done
	break;
      }
      index = _getParentIndex(index);
      if (0 == _bobble(index)) {
	// no change, done
	break;
      } else {
	changeCount++;
      }
    }
    return (changeCount > 0);
  }
  bool _downHeap(int index) {
    int changeCount = 0;
    while (1) {
      index = _bobble(index);
      if (0 == index) {
	// no change, done
	break;
      } else {
	changeCount++;
      }
    }
    return (changeCount > 0);
  }
  void _heapify(int index) {
    _upHeap(index) || _downHeap(index);
  }
  void _erase(PQEntry* e) {
    HeapIndex startIndex = e->heapIndex;
    bool doSwap = (startIndex != heap.size()-1);
    if (doSwap) {
      _swap(startIndex, heap.size()-1);
    }
    heap.pop_back();
    lookup.erase(e->v);
    delete e;
    if (doSwap) {
      _heapify(startIndex);
    }
  }

  // declare these protected because default implementations don't work
  PQueue(const PQueue& b) {}
  PQueue& operator=(const PQueue& b) { return *this; }
};

}; // namespace zmdp

#endif // INCPQueue_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.3  2005/12/06 20:28:15  trey
 * now you can specify the priority comparison operator to use
 *
 * Revision 1.2  2005/12/05 16:05:13  trey
 * api now a better match to std::priority_queue
 *
 * Revision 1.1  2005/11/29 04:43:05  trey
 * initial check-in
 *
 *
 ***************************************************************************/

