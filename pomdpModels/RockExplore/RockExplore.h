/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-03-05 08:58:26 $
   
 @file    RockExplore.h
 @brief   The RockExplore problem is closely related to the RockSample problem
          in my paper "Heuristic Search Value Iteration for POMDPs" (UAI 2004).
          This is a model for the problem.

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

#ifndef INCRockExplore_h
#define INCRockExplore_h

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace zmdp {

// A 2D position.
struct RockExplorePos {
  int x, y;

  RockExplorePos(void) {}
  RockExplorePos(int _x, int _y) : x(_x), y(_y) {}
  bool operator==(const RockExplorePos& a) const {
    return (x == a.x) && (y == a.y);
  }
  RockExplorePos addDelta(RockExplorePos& deltaPos) {
    RockExplorePos result;
    result.x = x + deltaPos.x;
    result.y = y + deltaPos.y;
    return result;
  }
};

// Parameters that define an instance of the RockExplore problem.
struct RockExploreParams {
  // Dimensions of the map in the x and y axes respectively
  int width, height;
  
  // Starting position of the robot
  RockExplorePos initPos;

  // The prior probability of each rock being good
  double rockGoodPrior;

  // Cost of executing a move action
  double costMove;

  // Cost of executing a check action
  double costCheck;

  // Cost of executing an illegal action (for example, moving off the
  // map)
  double costIllegal;

  // Cost of sampling a bad rock
  double costSampleBad;

  // Reward received for sampling a good rock
  double rewardSampleGood;

  // Reward received for exiting the map
  double rewardExit;

  // Discount factor
  double discountFactor;

  // Number of rocks in the map (gives length of vectors)
  int numRocks;

  // Vector giving the position of each rock
  std::vector<RockExplorePos> rockPos;
};

// A world state in the RockExplore problem.
struct RockExploreState {
  // If true, the system is in the unique 'terminal state' and the other
  // fields are ignored
  bool isTerminalState;

  // Position of the robot
  RockExplorePos robotPos;

  // Vector indicating whether each rock is good
  std::vector<bool> rockIsGood;
};

enum RockExploreActionTypes {
  ACT_MOVE,
  ACT_SAMPLE,
  ACT_CHECK,
};

// An action in the RockExplore problem.
struct RockExploreAction {
  // Type of action: ACT_MOVE, ACT_SAMPLE, or ACT_CHECK
  int actionType;

  // Specifies change in position (if any)
  RockExplorePos deltaPos;

  // If actionType is ACT_CHECK, specifies which rock to check
  int rockIndex;

  RockExploreAction(void) {}
  RockExploreAction(int ai) {
    switch (ai) {
    case 0: init(ACT_MOVE,   RockExplorePos( 0, 1), -1); break;
    case 1: init(ACT_MOVE,   RockExplorePos( 1, 0), -1); break;
    case 2: init(ACT_MOVE,   RockExplorePos( 0,-1), -1); break;
    case 3: init(ACT_MOVE,   RockExplorePos(-1, 0), -1); break;
    case 4: init(ACT_SAMPLE, RockExplorePos( 0, 0), -1); break;
    default:
      init(ACT_CHECK, RockExplorePos(0,0), ai-5);
    }
  }
  void init(int _actionType, RockExplorePos _deltaPos, int _rockIndex)
  {
    actionType = _actionType;
    deltaPos = _deltaPos;
    rockIndex = _rockIndex;
  }
};

// An entry in a probability distribution over states.
struct RockExploreBeliefEntry {
  double prob;
  int si;

  RockExploreBeliefEntry(void) {}
  RockExploreBeliefEntry(double _prob, int _si) : prob(_prob), si(_si) {}
};

// A probability distribution over states.
typedef std::vector<RockExploreBeliefEntry> RockExploreBelief;

// The implementation of the RockExplore model.
struct RockExplore {
  // Parameters of the model
  RockExploreParams params;

  // A mapping from the index of a state to the corresponding variable
  // values
  std::vector<RockExploreState> states;

  // A mapping from the string identifier for a state to the state index
  std::map<std::string, int> stateLookup;

  // Sets result to be the string identifier for state s.  Returns result.
  std::string& getStateString(std::string& result, const RockExploreState& s) const;

  // Returns the index for state s.  This includes assigning an index to
  // state s if it doesn't already have one.
  int getStateId(const RockExploreState& s);

  // Sets result to be the map for state s and belief b.  Returns result.
  std::string& getMap(std::string& result,
		      const RockExploreState& s,
		      const std::vector<double>& probRockIsGood) const;

  // Sets reward to be the reward for applying action ai in state si.
  // Sets outcomes to be the distribution of possible successor states.
  void getActionResult(double& reward,
		       RockExploreBelief& outcomes,
		       int si, int ai);

  // Returns the number of distinct observations.
  int getNumObservations(void) const;

  // Returns the probability of seeing observation o if action ai is applied
  // and the system transitions to state si.
  double getObsProb(int ai, int si, int o);
};

}; // namespace zmdp

#endif // INCRockExplore_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

