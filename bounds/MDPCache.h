/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-04 17:21:17 $
   
 @file    MDPCache.h
 @brief   Data structures for caching the explicit search graph
          while solving an MDP.

 Copyright (c) 2005, Trey Smith. All rights reserved.

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
  MDPNode* nextState;
};

struct MDPQEntry {
  double immediateReward;
  std::vector<MDPEdge*> outcomes;
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
  void* searchData;
  void* boundsData;

  bool isFringe(void) const { return Q.empty(); }
  size_t getNumActions(void) const { return Q.size(); }
  MDPNode& getNextState(int a, int o) { return *Q[a].outcomes[o]->nextState; }
};

typedef EXT_NAMESPACE::hash_map<std::string, MDPNode*> MDPHash;

}; // namespace zmdp

#endif // INCMDPCache_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/02/17 18:10:05  trey
 * added low and idx fields to MDPNode for use by HDP
 *
 * Revision 1.3  2006/02/13 21:47:53  trey
 * added prio field in MDPNode
 *
 * Revision 1.2  2006/02/13 19:07:22  trey
 * added MDPNode::getNextState() convenience method
 *
 * Revision 1.1  2006/02/08 19:21:44  trey
 * initial check-in
 *
 *
 ***************************************************************************/

