/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-03-05 23:33:24 $
   
 @file    RockSample.h
 @brief   This is a C++ implementation of the RockSample model.

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

#ifndef INCRockSample_h
#define INCRockSample_h

#include <iostream>
#include <string>
#include <vector>

struct Pos2D {
  int x, y;
};

struct RockSampleProblemParams {
  // the dimensions of the map in the x and y axes respectively
  int width, height;
  
  // the starting position of the robot
  Pos2D initPos;

  // the number of rocks in the map
  int numRocks;

  // the position of each rock
  Pos2D* rockPos;

  // the prior probability of each rock being good
  double* rockGoodPrior;

  // the reward received for exiting the map
  
};

struct RockSampleState {
  // if true, we are in the unique 'terminal' state and the other fields are ignored
  bool isTerminalState;

  // position of the robot
  int x, y;

  // a boolean value indicating whether each rock is good
  bool* rockIsGood;
};

struct RockSample : public MDP {
  state_vector myInitState;
  int myNumStates;

  // You can declare any additional variables you need here.

  RockSample(const ZMDPConfig& config);
  ~RockSample(void);

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

#endif // INCRockSample_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

