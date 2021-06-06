/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2005-2006, Trey Smith. All rights reserved.

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

#ifndef INCMDPModel_h
#define INCMDPModel_h

#include <iostream>
#include <string>
#include <vector>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#include "zmdpConfig.h"

using namespace sla;

namespace zmdp {

struct AbstractBound;

// Represents an MDP where state is continuous, time is discrete,
// actions are discrete, and the possible outcomes of an action form a
// discrete probability distribution.  This data structure can
// represent the belief MDP corresponding to a discrete POMDP.
struct MDP {
  std::string
      fileName; // filename model was read from (or empty string if unknown)
  int numStateDimensions, numActions;
  double discount;

  virtual ~MDP(void) {}

  int getNumStateDimensions(void) const { return numStateDimensions; }
  int getNumActions(void) const { return numActions; }
  double getDiscount(void) const { return discount; }

  // returns the initial state
  virtual const state_vector &getInitialState(void) = 0;

  // returns true if state is terminal
  virtual bool getIsTerminalState(const state_vector &s) = 0;

  // sets result to be the vector of outcome probabilities when from
  // state s action a is selected
  virtual outcome_prob_vector &getOutcomeProbVector(outcome_prob_vector &result,
                                                    const state_vector &s,
                                                    int a) = 0;

  // sets result to be the next state when from state s action a is
  // selected and outcome o results
  virtual state_vector &getNextState(state_vector &result,
                                     const state_vector &s, int a, int o) = 0;

  // returns the expected immediate reward when from state s action a is
  // selected
  virtual double getReward(const state_vector &s, int a) = 0;

  // returns a new lower bound or upper bound that is valid for
  // this MDP.  notes:
  // * the resulting bound must be initialized before it is used, and
  //   initialization may take significant computation time.
  // * some types of MDP might not define one of these bounds, which
  //   could be signaled by returning NULL.  so far this hasn't been
  //   explored.
  virtual AbstractBound *newLowerBound(const ZMDPConfig *_config) = 0;
  virtual AbstractBound *newUpperBound(const ZMDPConfig *_config) = 0;
};

}; // namespace zmdp

#endif // INCMDP_h
