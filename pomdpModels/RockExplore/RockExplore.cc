/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.12 $  $Author: trey $  $Date: 2007-03-07 08:50:35 $
  
 @file    RockExplore.cc
 @brief   No brief

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <getopt.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <queue>

#include "RockExplore.h"

using namespace std;

namespace zmdp {

// Initialization code.
RockExplore::RockExplore(void)
{
  // Set up the rockPos data structure.
  int rockCoords[] = RE_ROCK_POSITIONS;
  for (int i=0; i < RE_NUM_ROCKS; i++) {
    rockPos.push_back(REPos(rockCoords[i*2], rockCoords[i*2+1]));
  }

  // Make sure the terminal state is assigned an id before any other state
  // so that it gets the id 0, handy for debugging.
  getStateId(getTerminalState());

  // Assign indices to all reachable states.
  generateReachableStates();
}

// Sets result to be the initial belief.  Returns result.
REBelief RockExplore::getInitialBelief(void)
{
  // Initialize the probability of each rock being good to the prior value
  // for the problem.
  RERockProbs rockProbs(RE_NUM_ROCKS, RE_ROCK_GOOD_PRIOR);

  // Construct a belief distribution from the initial robot position and
  // the priors.
  return getBelief(REPos(RE_INIT_X, RE_INIT_Y), rockProbs);
}

// Returns the results of from state si applying action ai.
REActionResult RockExplore::getActionResult(int si, int ai)
{
  REState s = states[si];
  REAction a(ai);
  REState nextState = s;

  REActionResult result;
  result.reward = 0;

  if (s.isTerminalState) {
    // If in terminal state, self-transition and get zero reward.
    result.reward = 0;
    result.outcomes.push_back(REBeliefEntry(1.0, getStateId(getTerminalState())));
  } else {
    switch (a.actionType) {
    case ACT_MOVE:
      // Apply the change in position.
      nextState.robotPos.x += a.deltaPos.x;
      nextState.robotPos.y += a.deltaPos.y;

      // Check if the move stays on the map.
      if (0 <= nextState.robotPos.x && nextState.robotPos.x < RE_WIDTH
	  && 0 <= nextState.robotPos.y && nextState.robotPos.y < RE_HEIGHT) {
	// New position is in bounds.  Transition to the new position
	// and apply the move cost.
	result.reward = RE_COST_MOVE;
	result.outcomes.push_back(REBeliefEntry(1.0, getStateId(nextState)));
      } else if (nextState.robotPos.x == RE_WIDTH) {
	// Reached exit area.  Transition to terminal state and receive
	// the reward for exiting.
	result.reward = RE_REWARD_EXIT;
	result.outcomes.push_back(REBeliefEntry(1.0, getStateId(getTerminalState())));
      } else {
	// Illegal attempt to move off the map.  No change in position and
	// apply the illegal action cost.
	result.reward = RE_COST_ILLEGAL;
	result.outcomes.push_back(REBeliefEntry(1.0, si));
      }
      break;
    case ACT_SAMPLE: {
      bool samplingInEmptyCell = true;
      for (int r=0; r < RE_NUM_ROCKS; r++) {
	if (s.robotPos == rockPos[r]) {
	  // Sampling rock r
	  if (s.rockIsGood[r]) {
	    // Rock r is good
	    result.reward = RE_REWARD_SAMPLE_GOOD;
	  } else {
	    // Rock r is bad
	    result.reward = RE_COST_SAMPLE_BAD;
	  }
	  // After sampling, set the rock to be 'bad' so that no further
	  // reward can be gained from sampling it a second time.
	  nextState.rockIsGood[r] = false;
	  // Record that the location was not an empty cell.
	  samplingInEmptyCell = false;
	  break;
	}
      }
      if (samplingInEmptyCell) {
	// Sampling empty cell.  Apply illegal action cost.
	result.reward = RE_COST_ILLEGAL;
      }
      result.outcomes.push_back(REBeliefEntry(1.0, getStateId(nextState)));
      break;
    }
    case ACT_CHECK:
      // Apply check cost and no change in state.
      result.reward = RE_COST_CHECK;
      result.outcomes.push_back(REBeliefEntry(1.0, si));
      break;
    default:
      assert(0); // never reach this point
    }
  }

  return result;
}

// Returns the probability of seeing observation o if action ai is applied
// and the system transitions to state sp.
double RockExplore::getObsProb(int ai, int sp, int o)
{
  // Translate from state id sp to state struct and from action id ai to action
  // struct.
  REState s = states[sp];
  REAction a(ai);

  if (a.actionType == ACT_CHECK && !s.isTerminalState) {
    // Each matrix in obsProbTable gives observation probabilities.  The
    // top row is probabilities for when the rock is bad; the bottom row
    // is for when the rock is good.  Reading from left to right in the
    // row you get the probabilities for seeing observations 0 and 1.
    static double obsProbTable[] = {
      // Matrix for when the robot is in the same cell as the rock
      1.0, 0.0,
      0.0, 1.0,

      // Matrix for when the robot is at Manhattan distance > 0 from the rock
       0.95, 0.05,
      0.05, 0.95
    };

    // Calculate the Manhattan distance between the robot and the rock
    // it is checking.
    REPos rpos = rockPos[a.rockIndex];
    int manhattanDistance = std::abs(rpos.x - s.robotPos.x)
      + std::abs(rpos.y - s.robotPos.y);

    // Set up to index into the probability table.
    int obsProbMatrix = (manhattanDistance > 0) ? 1 : 0;
    int obsProbRow = s.rockIsGood[a.rockIndex] ? 1 : 0;
    int obsProbCol = o;

    return obsProbTable[4*obsProbMatrix + 2*obsProbRow + obsProbCol];
  } else {
    // Actions other than check give no useful information (always
    // return observation 0)
    return (o == 0) ? 1.0 : 0.0;
  }
}

// Return the string representation of the given state.
std::string RockExplore::getStateString(int si)
{
  return getStateString(states[si]);
}

// Return the string representation of the given action.
std::string RockExplore::getActionString(int ai)
{
  static const char* actionTable[] = {"amn", "ame", "ams", "amw", "as",
				      "ac0", "ac1", "ac2", "ac3", "ac4",
				      "ac5", "ac6", "ac7", "ac8", "ac9" };
  assert(ai < 15);
  return actionTable[ai];
}

// Return the string representation of the given observation.
std::string RockExplore::getObservationString(int oi)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "o%d", oi);
  return buf;
}

// Returns the human-readable map for the given belief.
std::string RockExplore::getMap(int si, const REBelief& b)
{
  RERockProbs rockProbs = getRockProbs(b);
  REState s = states[si];

  if (s.isTerminalState) {
    return "[terminal state]";
  } else {
    std::ostringstream outs;
    REPos pos;

    // Top boundary
    for (pos.x=0; pos.x < RE_WIDTH*2+3; pos.x++) {
      outs << "#";
    }
    outs << endl;

    for (pos.y=RE_HEIGHT-1; pos.y >= 0; pos.y--) {
      // Left boundary
      outs << "# ";
      for (pos.x=0; pos.x < RE_WIDTH; pos.x++) {
	int rockNum = -1;
	for (int r=0; r < RE_NUM_ROCKS; r++) {
	  if (pos == rockPos[r]) {
	    rockNum = r;
	    break;
	  }
	}
	if (pos == s.robotPos) {
	  // Robot marked with '*'
	  outs << "*";
	} else if (rockNum != -1) {
	  // Rock marked with rock number
	  outs << rockNum;
	} else {
	  // Empty space marked with "."
	  outs << ".";
	}
	// Mark character to the right of a rock with +/- depending
	// on whether the rock is good.  Otherwise just leave an
	// extra space.
	outs << ((rockNum != -1) ? (s.rockIsGood[rockNum] ? "+" : "-") : " ");
      }
      // Right boundary
      outs << "x" << endl;
    }

    // Bottom boundary
    for (pos.x=0; pos.x < RE_WIDTH*2+3; pos.x++) {
      outs << "#";
    }
    outs << endl;

    // Probability of rocks being good
    outs << "Rock probs: ";
    for (int r=0; r < RE_NUM_ROCKS; r++) {
      char pbuf[20];
      snprintf(pbuf, sizeof(pbuf), "%5.3lf", rockProbs[r]);
      outs << r << "=" << pbuf << " ";
    }
    outs << endl;

    return outs.str();
  }
}


// Calculates the marginal probability that each rock is good from the
// given belief b.  Returns the vector of marginals.
RERockProbs RockExplore::getRockProbs(const REBelief& b) const
{
  RERockProbs result;

  // Initialize the result vector to all zero values.
  result.clear();
  result.resize(RE_NUM_ROCKS, 0.0);

  // Iterate through the outcomes in the belief vector.
  for (int i=0; i < (int)b.size(); i++) {
    // Translate from the state index of the outcome, b[i].index, to the
    // corresponding state struct s.
    REState s = states[b[i].index];
    for (int r=0; r < RE_NUM_ROCKS; r++) {
      // If rock r is good in outcome i, add the probability of
      // outcome i to result[r].
      if (s.rockIsGood[r]) {
	result[r] += b[i].prob;
      }
    }
  }
  return result;
}

// Returns a belief in which all the states have the given robotPos
// and a distribution of rockIsGood values consistent with the
// marginals specified by probRockIsGood.  This is effectively the
// inverse of the getRockProbs() function.
REBelief RockExplore::getBelief(const REPos& robotPos,
				const RERockProbs& rockProbs)
{
  REBelief result;

  // Fill in the variables of sp that will be the same across all outcomes
  // in the result.
  REState sp;
  sp.isTerminalState = false;
  sp.robotPos = robotPos;

  // Initialize to the 'first' rockIsGood vector, in which all rockIsGood
  // entries are false.
  std::vector<bool> rockIsGood(RE_NUM_ROCKS, false);

  while (1) {
    // Calculate probability of the current rockIsGood vector according
    // to the specified marginals.
    double prob = 1.0;
    for (int r=0; r < RE_NUM_ROCKS; r++) {
      prob *= (rockIsGood[r]) ? rockProbs[r] : (1.0 - rockProbs[r]);
    }

    // If the probability is non-zero, add an entry to the resulting belief.
    if (prob > 0.0) {
      sp.rockIsGood = rockIsGood;
      result.push_back(REBeliefEntry(prob, getStateId(sp)));
    }

    // Advance to next rockIsGood vector.  (This code basically adds one
    // to a bit vector.)
    bool reachedLastCombination = true;
    for (int r=RE_NUM_ROCKS-1; r >= 0; r--) {
      if (!rockIsGood[r]) {
	rockIsGood[r] = true;
	for (int rr=r+1; rr < RE_NUM_ROCKS; rr++) {
	  rockIsGood[rr] = false;
	}
	reachedLastCombination = false;
	break;
      }
    }
    if (reachedLastCombination) break;
  }

#if 0
  cout << "getBelief: ";
  for (int i=0; i < (int)result.size(); i++) {
    cout << result[i].prob << "," << getStateString(result[i].index) << " ";
  }
  cout << endl;
#endif

  return result;
}

// Returns the terminal state.
const REState& RockExplore::getTerminalState(void) {
  static REState s;
  s.isTerminalState = true;
  return s;
}

// Returns the string identifier for state s.
std::string RockExplore::getStateString(const REState& s)
{
  // String identifiers are in the form "st" (for the special terminal state) or
  // "sx2y3r0110", meaning (x=2, y=3, rockIsGood[0]=0, rockIsGood[1]=1, ...)
  std::string result;
  if (s.isTerminalState) {
    result = "st";
  } else {
    std::ostringstream outs;
    outs << "s"
	 << "x" << s.robotPos.x
	 << "y" << s.robotPos.y
	 << "r";
    for (int i=0; i < RE_NUM_ROCKS; i++) {
      outs << s.rockIsGood[i];
    }
    result = outs.str();
  }
  return result;
}

RockExplore m;

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2007/03/07 08:34:14  trey
 * fixed bugs introduced during refactoring
 *
 * Revision 1.10  2007/03/07 08:12:27  trey
 * refactored things
 *
 * Revision 1.9  2007/03/06 08:46:56  trey
 * many tweaks
 *
 * Revision 1.8  2007/03/06 07:57:01  trey
 * added getMostLikelyState()
 *
 * Revision 1.7  2007/03/06 07:48:43  trey
 * cleaned up
 *
 * Revision 1.6  2007/03/06 06:52:30  trey
 * avoided potential issues with random numbers
 *
 * Revision 1.5  2007/03/06 06:37:52  trey
 * implementing heuristics
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
