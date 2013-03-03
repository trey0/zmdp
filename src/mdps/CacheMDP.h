/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-03-25 07:08:16 $
   
 @file    CacheMDP.h
 @brief   No brief

 Copyright (c) 2007, Trey Smith.

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

#ifndef INCCacheMDP_h
#define INCCacheMDP_h

#include <iostream>
#include <string>
#include <vector>

#include "zmdpConfig.h"
#include "MDPModel.h"
#include "AbstractBound.h"

namespace zmdp {

struct CMDPNode;

struct CMDPEdge {
  CMDPNode* nextState;
  // code that uses CacheMDP can associate arbitrary extra data with an edge
  int userInt;
  double userDouble;
};

struct CMDPQEntry {
  double immediateReward;
  outcome_prob_vector opv;
  std::vector<CMDPEdge*> outcomes;

  size_t getNumOutcomes(void) const { return outcomes.size(); }
};

struct CMDPNode {
  state_vector s;
  int si;
  bool isTerminal;
  std::vector<CMDPQEntry*> Q;
  // code that uses CacheMDP can associate arbitrary extra data with a node
  int userInt;

  ~CMDPNode(void) {
    FOR_EACH (elt, Q) {
      if (NULL != (*elt)) {
	delete (*elt);
      }
    }
  }
  bool isFringe(void) const { return Q.empty(); }
  size_t getNumActions(void) const { return Q.size(); }
};

typedef EXT_NAMESPACE::hash_map<std::string, int> CMDPHash;
typedef std::vector<CMDPNode*> CMDPNodeTable;

struct CacheMDP : public MDP {
  MDP* problem;
  CMDPNode* root;
  CMDPHash lookup;
  CMDPNodeTable nodeTable;
  state_vector initialSI;
  
  CacheMDP(MDP* _problem);
  ~CacheMDP(void);
  
  const state_vector& getInitialState(void);
  bool getIsTerminalState(const state_vector& s);
  outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,
					    const state_vector& s, int a);
  state_vector& getNextState(state_vector& result, const state_vector& s, int a,
			     int o);
  double getReward(const state_vector& s, int a);
  const state_vector& translateState(state_vector& result, const state_vector& s);

  AbstractBound* newLowerBound(const ZMDPConfig* _config) { assert(0); }
  AbstractBound* newUpperBound(const ZMDPConfig* _config) { assert(0); }

  CMDPNode* getNode(const state_vector& s);
  CMDPQEntry* getQ(CMDPNode& cn, int a);
  CMDPNode* getNodeX(const state_vector& s);

};

}; // namespace zmdp

#endif // INCCacheMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/03/24 22:44:15  trey
 * initial check-in
 *
 *
 ***************************************************************************/

