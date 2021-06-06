/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#ifndef ZMDP_SRC_MDPS_RACETRACK_H_
#define ZMDP_SRC_MDPS_RACETRACK_H_

#include <iostream>
#include <string>
#include <vector>

#include "AbstractBound.h"
#include "MDPModel.h"

namespace zmdp {

struct TrackMap;

class RaceTrack : public MDP {
 public:
  double errorProbability;
  bool useErrorIsWind;
  bool useMaxCost;
  double maxCost;
  state_vector bogusInitialState, terminalState;
  TrackMap *tmap;

  RaceTrack(void);
  ~RaceTrack(void);
  explicit RaceTrack(const std::string &specFileName);

  void readFromFile(const std::string &specFileName);

  // returns the initial state
  const state_vector &getInitialState(void);

  // returns true if state is terminal
  bool getIsTerminalState(const state_vector &s);

  // sets result to be the vector of outcome probabilities when from
  // state s action a is selected
  outcome_prob_vector &getOutcomeProbVector(outcome_prob_vector &result,
                                            const state_vector &s, int a);

  // sets result to be the next state when from state s action a is
  // selected and outcome o results
  state_vector &getNextState(state_vector &result, const state_vector &s, int a,
                             int o);

  // returns the expected immediate reward when from state s action a is
  // selected
  double getReward(const state_vector &s, int a);

  AbstractBound *newLowerBound(const ZMDPConfig *_config);
  AbstractBound *newUpperBound(const ZMDPConfig *_config);
};

};  // namespace zmdp

#endif  // ZMDP_SRC_MDPS_RACETRACK_H_
