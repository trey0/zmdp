/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2007-03-07 08:50:35 $
   
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

#include "REBasicPomdp.h"

namespace zmdp {

/**********************************************************************
 * TUNABLE PARAMETERS
 **********************************************************************/

#define RE_WIDTH (4)
#define RE_HEIGHT (4)
#define RE_INIT_X (0)
#define RE_INIT_Y (1)
#define RE_ROCK_GOOD_PRIOR (0.7)
#define RE_COST_MOVE (-1)
#define RE_COST_CHECK (-1)
#define RE_COST_ILLEGAL (-100)
#define RE_COST_SAMPLE_BAD (-15)
#define RE_REWARD_SAMPLE_GOOD (15)
#define RE_REWARD_EXIT (15)
#define RE_DISCOUNT (0.95)
#define RE_NUM_ROCKS (4)
#define RE_ROCK_POSITIONS { 1,3,  2,1,  3,1,  0,0 }

/**********************************************************************
 * HELPER DATA STRUCTURES
 **********************************************************************/

// NOTE: The REPos and REState data structures really belong here
// because they are specific to the RockExplore domain.  However, it was
// easier to implement everything with those data structures defined in
// REBasicPomdp.h.

// Action types for RockExplore.
enum REActionTypes {
  ACT_MOVE,
  ACT_SAMPLE,
  ACT_CHECK,
};

// An action in the RockExplore problem.
struct REAction {
  // Type of action: ACT_MOVE, ACT_SAMPLE, or ACT_CHECK
  int actionType;

  // Specifies change in position (if any)
  REPos deltaPos;

  // If actionType is ACT_CHECK, specifies which rock to check
  int rockIndex;

  REAction(void) {}

  // Generates an action struct given an action index.
  REAction(int ai) {
    switch (ai) {
    case 0: init(ACT_MOVE,   REPos( 0, 1), -1); break;
    case 1: init(ACT_MOVE,   REPos( 1, 0), -1); break;
    case 2: init(ACT_MOVE,   REPos( 0,-1), -1); break;
    case 3: init(ACT_MOVE,   REPos(-1, 0), -1); break;
    case 4: init(ACT_SAMPLE, REPos( 0, 0), -1); break;
    default:
      init(ACT_CHECK, REPos(0,0), ai-5);
    }
  }

  // A function for initializing all the fields of an action.
  void init(int _actionType, REPos _deltaPos, int _rockIndex)
  {
    actionType = _actionType;
    deltaPos = _deltaPos;
    rockIndex = _rockIndex;
  }
};

// A vector representing the marginal probability that each rock is good.
typedef std::vector<double> RERockProbs;

/**********************************************************************
 * THE MAIN MODEL
 **********************************************************************/

struct RockExplore : public REBasicPomdp {
  RockExplore(void);
  
  // The fixed locations of the rocks in the map.
  std::vector<REPos> rockPos;

  /**********************************************************************
   * FUNCTIONS DESCRIBING THE BASIC MODEL
   **********************************************************************/
  
  // Returns the number of states, actions, or observations.
  int getNumStates(void) { return states.size(); }
  int getNumActions(void) { return RE_NUM_ROCKS + 5; }
  int getNumObservations(void) { return 2; }

  // Returns the initial belief b0.
  REBelief getInitialBelief(void);

  // Returns the results of from state si applying action ai.
  REActionResult getActionResult(int si, int ai);

  // Returns the probability of seeing observation o when action
  // ai is applied and the system transitions to state sp.
  double getObsProb(int ai, int sp, int o);

  /**********************************************************************
   * I/O FUNCTIONS -- SUBCLASSES IMPLEMENT THESE
   **********************************************************************/
  
  // Return the string representation of the given state, action, or observation.
  std::string getStateString(int si);
  std::string getActionString(int ai);
  std::string getObservationString(int oi);

  // Returns the human-readable map for the given belief.
  std::string getMap(int si, const REBelief& b);

  /**********************************************************************
   * INTERNAL HELPER FUNCTIONS
   **********************************************************************/
  
  // Calculates the marginal probability that each rock is good from the
  // given belief b.  Returns the vector of marginals.
  RERockProbs getRockProbs(const REBelief& b) const;

  // Returns a belief in which all the states have the given robotPos
  // and a distribution of rockIsGood values consistent with the
  // marginals specified by probRockIsGood.  This is effectively the
  // inverse of the getRockProbs() function.
  REBelief getBelief(const REPos& robotPos,
		     const RERockProbs& rockProbs);

  // Returns the terminal state.
  static const REState& getTerminalState(void);

  // Returns the string identifier for state s.  Returns result.
  std::string getStateString(const REState& s);
};

extern RockExplore m;

}; // namespace zmdp

#endif // INCRockExplore_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2007/03/07 08:34:14  trey
 * fixed bugs introduced during refactoring
 *
 * Revision 1.6  2007/03/07 08:12:27  trey
 * refactored things
 *
 * Revision 1.5  2007/03/06 07:57:00  trey
 * added getMostLikelyState()
 *
 * Revision 1.4  2007/03/06 04:32:47  trey
 * working towards heuristic policies
 *
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

