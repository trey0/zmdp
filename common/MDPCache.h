/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-02-13 19:07:22 $
   
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
  bool isSolved; // used by LRTDP

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
 * Revision 1.1  2006/02/08 19:21:44  trey
 * initial check-in
 *
 *
 ***************************************************************************/

