/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-11-07 20:09:42 $
   
 @file    CustomMDP.h
 @brief   No brief

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

#include "zmdpConfig.h"
#include "MDP.h"
#include "AbstractBound.h"

namespace zmdp {

struct CustomMDP : public MDP {
  state_vector myInitState;
  int myNumStates;

  // You can declare any additional variables you need here.

  CustomMDP(const ZMDPConfig& config);
  ~CustomMDP(void);

  const state_vector& getInitialState(void) const;
  bool getIsTerminalState(const state_vector& s) const;
  outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,
					    const state_vector& s, int a)
       const;
  state_vector& getNextState(state_vector& result, const state_vector& s, int a,
			     int o) const;
  double getReward(const state_vector& s, int a) const;

  double getInitialLowerBoundValue(const state_vector& s) const;
  double getInitialUpperBoundValue(const state_vector& s) const;

  AbstractBound* newLowerBound(const ZMDPConfig* _config) const;
  AbstractBound* newUpperBound(const ZMDPConfig* _config) const;
};

}; // namespace zmdp

#endif // INCCustomMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

