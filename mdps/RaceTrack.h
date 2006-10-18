/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2006-10-18 18:06:06 $
   
 @file    RaceTrack.h
 @brief   No brief

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

#ifndef INCRaceTrack_h
#define INCRaceTrack_h

#include <iostream>
#include <string>
#include <vector>

#include "MDP.h"
#include "AbstractBound.h"

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
  RaceTrack(const std::string& specFileName);

  void readFromFile(const std::string& specFileName);

  // returns the initial state
  const state_vector& getInitialState(void) const;

  // returns true if state is terminal
  bool getIsTerminalState(const state_vector& s) const;

  // sets result to be the vector of outcome probabilities when from
  // state s action a is selected
  outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,
					    const state_vector& s, int a)
       const;

  // sets result to be the next state when from state s action a is
  // selected and outcome o results
  state_vector& getNextState(state_vector& result, const state_vector& s, int a,
			     int o) const;

  // returns the expected immediate reward when from state s action a is selected
  double getReward(const state_vector& s, int a) const;

  AbstractBound* newLowerBound(const ZMDPConfig& config) const;
  AbstractBound* newUpperBound(const ZMDPConfig& config) const;
};

}; // namespace zmdp

#endif // INCRaceTrack_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.6  2006/02/19 22:19:25  trey
 * added high-uncertainty "wind" version of the problem
 *
 * Revision 1.5  2006/02/17 18:17:35  trey
 * added useMaxCost and maxCost fields, allowing RaceTrack to be used as an undiscounted problem
 *
 * Revision 1.4  2006/02/06 19:27:24  trey
 * fixed several problems
 *
 * Revision 1.3  2006/02/01 18:03:14  trey
 * fixed compile-time errors, not quite done yet
 *
 * Revision 1.2  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2006/01/31 18:12:30  trey
 * initial check-in in mdps directory
 *
 *
 ***************************************************************************/

