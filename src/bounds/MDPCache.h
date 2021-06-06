/********** tell emacs we use -*- c++ -*- style comments *******************
 @file    MDPCache.h
 @brief   Data structures for caching the explicit search graph
          while solving an MDP.

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

#ifndef INCMDPCache_h
#define INCMDPCache_h

#include <iostream>
#include <string>
#include <vector>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

using namespace sla;

namespace zmdp {

struct MDPNode;

struct MDPEdge {
  double obsProb;
  MDPNode *nextState;
};

struct MDPQEntry {
  double immediateReward;
  std::vector<MDPEdge *> outcomes;
  double lbVal, ubVal;

  size_t getNumOutcomes(void) const { return outcomes.size(); }
};

struct MDPNode {
  state_vector s;
  bool isTerminal;
  std::vector<MDPQEntry> Q;
  double lbVal, ubVal;
  // these fields are used for different purposes depending on the search
  //   strategy and value function representation
  void *searchData;
  void *boundsData;

  bool isFringe(void) const { return Q.empty(); }
  size_t getNumActions(void) const { return Q.size(); }
  MDPNode &getNextState(int a, int o) { return *Q[a].outcomes[o]->nextState; }
};

typedef EXT_NAMESPACE::hash_map<std::string, MDPNode *> MDPHash;

int getNodeCacheStorage(const MDPHash *lookup, int whichMetric);

}; // namespace zmdp

#endif // INCMDPCache_h
