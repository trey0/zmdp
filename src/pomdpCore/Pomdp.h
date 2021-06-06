/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#ifndef INCPomdp_h
#define INCPomdp_h

#include <iostream>
#include <string>
#include <vector>

#include "CassandraModel.h"

using namespace sla;

namespace zmdp {

struct Pomdp : public CassandraModel {
  Pomdp(const std::string &fileName, const ZMDPConfig *_config);

  // returns the initial belief
  const belief_vector &getInitialBelief(void) const;

  // sets result to be the vector of observation probabilities when from
  // belief b action a is selected
  obs_prob_vector &getObsProbVector(obs_prob_vector &result,
                                    const belief_vector &b, int a) const;

  // sets result to be the next belief when from belief b action a is
  // selected and observation o is observed
  belief_vector &getNextBelief(belief_vector &result, const belief_vector &b,
                               int a, int o) const;

  // returns the expected immediate reward when from belief b action a is
  // selected
  double getReward(const belief_vector &b, int a);

  AbstractBound *newLowerBound(const ZMDPConfig *_config);
  AbstractBound *newUpperBound(const ZMDPConfig *_config);

  // POMDP-as-belief-MDP aliases for functions implemented in MDP
  int getBeliefSize(void) const { return getNumStateDimensions(); }
  int getNumObservations(void) const { return numObservations; }
  void setBeliefSize(int beliefSize) { numStateDimensions = beliefSize; }
  void setNumObservations(int _numObservations) {
    numObservations = _numObservations;
  }

  // POMDP-as-belief-MDP implementations for virtual functions declared in MDP
  const state_vector &getInitialState(void) { return getInitialBelief(); }
  bool getIsTerminalState(const state_vector &s);
  outcome_prob_vector &getOutcomeProbVector(outcome_prob_vector &result,
                                            const state_vector &b, int a) {
    return getObsProbVector(result, b, a);
  }
  state_vector &getNextState(state_vector &result, const state_vector &s, int a,
                             int o) {
    return getNextBelief(result, s, a, o);
  }

protected:
  void readFromFileCassandra(const std::string &fileName);
  void readFromFileFast(const std::string &fileName);

  void debugDensity(void);
};

}; // namespace zmdp

#endif // INCPomdp_h
