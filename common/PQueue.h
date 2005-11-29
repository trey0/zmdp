/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2005-11-29 04:43:05 $
   
 @file    PQueue.h
 @brief   No brief

 Copyright (c) 2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ***************************************************************************/

#ifndef INCPQueue_h
#define INCPQueue_h

#include <vector>

#include "pomdpCommonDefs.h"
#include "pomdpCommonTypes.h"

namespace pomdp {

template <class ValueType, class PriorityType>
struct PQueue {
  typedef size_t HeapIndex;
  struct PQEntry {
    ValueType v;
    PriorityType prio;
    HeapIndex heapIndex;
    PQEntry(ValueType _v, PriorityType _prio, HeapIndex _heapIndex) :
      v(_v), prio(_prio), heapIndex(_heapIndex)
    {}
  };

  PQueue(void) {}

  void setPriority(const ValueType& v, const PriorityType prio) {
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
  const PQEntry& top(void) {
    return *heap[0];
  }
  void pop(void) {
    _erase(heap[0]);
  }
  void erase(const ValueType& v) {
    _erase(lookup[v]);
  }
  size_t size(void) { return heap.size(); }
  bool empty(void) { return heap.empty(); }
  void printEntry(PQEntry* e) {
    std::cout << e->v << " " << e->prio << " " << e->heapIndex;
  }
  void print(void) {
    std::cout << "heap:" << std::endl;
    FOR (i, heap.size()) {
      std::cout << "  " << i << " ";
      printEntry(heap[i]);
      std::cout << std::endl;
    }
    std::cout << "lookup:" << std::endl;
    FOR_EACH (pr, lookup) {
      std::cout << "  " << pr->first << " ";
      printEntry(pr->second);
      std::cout << std::endl;
    }
  }
  
protected:
  typedef EXT_NAMESPACE::hash_map<ValueType, PQEntry*> Lookup;

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
    if (child0 && child0->prio < parent->prio) {
      if (child1 && child1->prio < child0->prio) {
	_swap(parentIndex, i1);
	return i1;
      } else {
	_swap(parentIndex, i0);
	return i0;
      }
    } else {
      if (child1 && child1->prio < parent->prio) {
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

}; // namespace pomdp

#endif // INCPQueue_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

