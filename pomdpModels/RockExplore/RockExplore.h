/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-03-06 04:32:47 $
   
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

  RockExploreState(void) :
    isTerminalState(false),
    robotPos(RockExplorePos(-1,-1))
  {}
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

  // Generates an action struct given an action index.
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

  static const char* getString(int ai) {
    static const char* actionTable[] = {"amn", "ame", "ams", "amw", "as",
					"ac0", "ac1", "ac2", "ac3", "ac4",
					"ac5", "ac6", "ac7", "ac8", "ac9" };
    assert(ai < 15);
    return actionTable[ai];
  }

  // A function for initializing all the fields of an action.
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
  int index;

  RockExploreBeliefEntry(void) {}
  RockExploreBeliefEntry(double _prob, int _index) : prob(_prob), index(_index) {}
};

// A vector (sparsely) representing a probability distribution over states.
typedef std::vector<RockExploreBeliefEntry> RockExploreBelief;

// A vector representing a probability distribution over observations.
typedef std::vector<double> RockExploreObsProbs;

// A vector representing the marginal probability that each rock is good.
typedef std::vector<double> RockExploreRockMarginals;

// The implementation of the RockExplore model.
struct RockExplore {
  // Parameters of the problem
  RockExploreParams params;

  // Maps from the index of a state to the corresponding state struct
  std::vector<RockExploreState> states;

  // Maps from the string identifier for a state to the state index
  std::map<std::string, int> stateLookup;

  /**********************************************************************/

  // The constructor -- you must specify the problem params at initialization.
  RockExplore(const RockExploreParams& _params);

  // Sets result to be the initial belief.  Returns result.
  RockExploreBelief& getInitialBelief(RockExploreBelief& result);

  // Returns the number of states.
  int getNumStates(void) const { return states.size(); }

  // Returns the number of distinct actions.
  int getNumActions(void) const { return params.numRocks + 5; }

  // Sets reward to be the reward for applying action ai in state si.
  // Sets outcomes to be the distribution of possible successor states.
  void getActionResult(double& reward,
		       RockExploreBelief& outcomes,
		       int si, int ai);

  // Returns the number of distinct observations.
  int getNumObservations(void) const;

  // Returns the probability of seeing observation o if action ai is applied
  // and the system transitions to state sp.
  double getObsProb(int ai, int sp, int o) const;

  // Sets result to be the distribution of possible observations when
  // action ai is applied and the system transitions to state sp.
  // Returns result.
  RockExploreObsProbs& getObsProbs(RockExploreObsProbs& obsProbs,
				   int ai, int sp) const;

  // POMDP version of getActionResult.  Sets expectedReward to be the
  // expected reward and sets obsProbs to be the distribution of
  // possible observations when from belief b action ai is applied.
  void getActionResult(double& expectedReward,
		       RockExploreObsProbs& obsProbs,
		       const RockExploreBelief& b, int ai);

  // Sets bp to be the updated belief when from belief b action ai is
  // executed and observation o is received.
  void getUpdatedBelief(RockExploreBelief& bp,
			const RockExploreBelief& b,
			int ai, int o);

  // Uses the transition model to generate all reachable states and assign
  // them index values.  This is called during initialization; before it is
  // called getNumStates() is not valid.
  void generateReachableStates(void);

  // Outputs a Cassandra-format POMDP model to the given file.
  void writeCassandraModel(const std::string& outFile);

  // Calculates the marginal probability that each rock is good from the
  // given belief b.  Sets result to be the vector of marginals.
  // Returns result.
  RockExploreRockMarginals& getMarginals(RockExploreRockMarginals& result,
					 const RockExploreBelief& b) const;

  // Generates a belief in which all the states have the given robotPos
  // and a distribution of rockIsGood values consistent with the
  // marginals specified by probRockIsGood.  This is effectively the
  // inverse of the getMarginals() function.
  RockExploreBelief& getBelief(RockExploreBelief& result,
			       const RockExplorePos& robotPos,
			       const RockExploreRockMarginals& probRockIsGood);

  // Returns the terminal state.
  static const RockExploreState& getTerminalState(void);

  // Sets result to be the string identifier for state s.  Returns result.
  std::string& getStateString(std::string& result, const RockExploreState& s) const;

  // Sets result to be the string form for the state with id si. Returns result.
  std::string& getStateString(std::string& result, int si) const;

  // Returns the index for state s.  This includes assigning an index to
  // state s if it doesn't already have one.
  int getStateId(const RockExploreState& s);

  // Sets result to be the map for state si and belief b.  Returns result.
  std::string& getMap(std::string& result, int si,
		      const RockExploreRockMarginals& probRockIsGood) const;

  // Returns a stochastically selected state index from the distribution b.
  int chooseStochasticOutcome(const RockExploreBelief& b) const;

  // Returns a stochastically selected observation from the distribution obsProbs.
  int chooseStochasticOutcome(const RockExploreObsProbs& obsProbs) const;
};

extern RockExplore* modelG;

}; // namespace zmdp

#endif // INCRockExplore_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2007/03/06 02:23:08  trey
 * working interactive mode
 *
 * Revision 1.2  2007/03/05 23:33:24  trey
 * now outputs reasonable Cassandra model
 *
 * Revision 1.1  2007/03/05 08:58:26  trey
 * initial check-in
 *
 *
 ***************************************************************************/

