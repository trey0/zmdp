/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2006, Trey Smith.

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

#ifndef INCCustomMDP_h
#define INCCustomMDP_h

#include <iostream>
#include <string>
#include <vector>

#include "AbstractBound.h"
#include "MDPModel.h"
#include "zmdpConfig.h"

namespace zmdp {

struct CustomMDP : public MDP {
  state_vector myInitState;
  int myNumStates;

  // You can declare any additional variables you need here.

  CustomMDP(const ZMDPConfig &config);
  ~CustomMDP(void);

  const state_vector &getInitialState(void);
  bool getIsTerminalState(const state_vector &s);
  outcome_prob_vector &getOutcomeProbVector(outcome_prob_vector &result,
                                            const state_vector &s, int a);
  state_vector &getNextState(state_vector &result, const state_vector &s, int a,
                             int o);
  double getReward(const state_vector &s, int a);

  double getInitialLowerBoundValue(const state_vector &s);
  double getInitialUpperBoundValue(const state_vector &s);

  AbstractBound *newLowerBound(const ZMDPConfig *_config);
  AbstractBound *newUpperBound(const ZMDPConfig *_config);
};

}; // namespace zmdp

#endif // INCCustomMDP_h
