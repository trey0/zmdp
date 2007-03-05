/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-03-05 08:58:26 $
  
 @file    RockExplore.cc
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

#include "RockExplore.h"

#define RE_TERMINAL_STATE_ID (0)

namespace zmdp {

// Sets result to be the string identifier for state s.  Returns result.
std::string& RockExplore::getStateString(std::string& result, const RockExploreState& s) const
{
  // String identifiers are in the form "st" (for the special terminal state) or
  // "sx2y3r0110", meaning (x=2, y=3, rockIsGood[0]=0, rockIsGood[1]=1, ...)
  if (s.isTerminalState) {
    result = "st";
  } else {
    std::ostringstream outs;
    outs << "s"
	 << "x" << s.robotPos.x
	 << "y" << s.robotPos.y
	 << "r";
    for (int i=0; i < params.numRocks; i++) {
      outs << s.rockIsGood[i];
    }
    result = outs.str();
  }
  return result;
}

// Returns the index for state s.  This includes assigning an index to
// state s if it doesn't already have one.
int RockExplore::getStateId(const RockExploreState& s)
{
  // ss is the string representation of state s
  std::string ss;
  getStateString(ss,s);

  typeof(stateLookup.begin()) x = stateLookup.find(ss);
  if (x == stateLookup.end()) {
    // ss not found in stateLookup.  Assign a new index and return it
    int newIndex = states.size();
    states.push_back(s);
    stateLookup[ss] = newIndex;
    return newIndex;
  } else {
    // Return the existing index
    return x->second;
  }
}

// Sets result to be the map for state s and belief b.  Returns result.
std::string& RockExplore::getMap(std::string& result,
				 const RockExploreState& s,
				 const std::vector<double>& probRockIsGood) const
{
  if (s.isTerminalState) {
    result = "[terminal state]";
  } else {
    std::ostringstream outs;
    RockExplorePos pos;

    // Top boundary
    for (pos.x=0; pos.x < params.width*2+3; pos.x++) {
      outs << "#";
    }
    outs << std::endl;

    for (pos.y=params.height-1; pos.y >= 0; pos.y--) {
      // Left boundary
      outs << "# ";
      for (pos.x=0; pos.x < params.width; pos.x++) {
	if (pos == s.robotPos) {
	  // Robot marked with '*'
	  outs << "*";
	} else {
	  bool isRock = false;
	  for (int r=0; r < params.numRocks; r++) {
	    if (pos == params.rockPos[r]) {
	      if (s.rockIsGood[r]) {
		// Good rocks marked with 'A', 'B', 'C', etc.
		outs << ((char) ('A'+r));
	      } else {
		// Bad rocks marked with 'a', 'b', 'c', etc.
		outs << ((char) ('a'+r));
	      }
	      isRock = true;
	      break;
	    }
	  }
	  if (!isRock) {
	    // Empty map locations marked with '.'
	    outs << ".";
	  }
	}
	outs << " ";
      }
      // Right boundary
      outs << "x" << std::endl;
    }

    // Bottom boundary
    for (pos.x=0; pos.x < params.width*2+3; pos.x++) {
      outs << "#";
    }
    outs << std::endl;

    // Probability of rocks being good
    outs << "[";
    for (int r=0; r < params.numRocks; r++) {
      char pbuf[20];
      snprintf(pbuf, sizeof(pbuf), "%5.3lf", probRockIsGood[r]);
      outs << ((char) ('A'+r)) << " " << pbuf << " ";
    }
    outs << "]" << std::endl;

    result = outs.str();
  }
  return result;
}

// Sets reward to be the reward for applying action ai in state si.
// Sets outcomes to be the distribution of possible successor states.
void RockExplore::getActionResult(double& reward,
				  RockExploreBelief& outcomes,
				  int si, int ai)
{
  RockExploreState s = states[si];
  RockExploreAction a(ai);
  RockExploreState nextState = s;

  reward = 0;
  outcomes.clear();

  if (s.isTerminalState) {
    // If in terminal state, self-transition and get zero reward.
    reward = 0;
    outcomes.push_back(RockExploreBeliefEntry(1.0, RE_TERMINAL_STATE_ID));
  } else {
    bool isRock;
    switch (a.actionType) {
    case ACT_MOVE:
      nextState.robotPos.addDelta(a.deltaPos);
      if (0 <= nextState.robotPos.x && nextState.robotPos.x < params.width
	  && 0 <= nextState.robotPos.y && nextState.robotPos.y < params.height) {
	// New position is in bounds.  Transition to the new position
	// and apply the move cost.
	reward = params.costMove;
	outcomes.push_back(RockExploreBeliefEntry(1.0, getStateId(nextState)));
      } else if (nextState.robotPos.x == params.width) {
	// Reached exit area.  Transition to terminal state and apply
	// the move cost.
	reward = params.costMove;
	outcomes.push_back(RockExploreBeliefEntry(1.0, RE_TERMINAL_STATE_ID));
      } else {
	// Illegal attempt to move off the map.  No change in position and
	// apply the illegal action cost.
	reward = params.costIllegal;
	outcomes.push_back(RockExploreBeliefEntry(1.0, si));
      }
      break;
    case ACT_SAMPLE:
      isRock = false;
      for (int r=0; r < params.numRocks; r++) {
	if (s.robotPos == params.rockPos[r]) {
	  // Sampling rock r
	  if (s.rockIsGood[r]) {
	    // Rock r is good
	    reward = params.rewardSampleGood;
	  } else {
	    // Rock r is bad
	    reward = params.costSampleBad;
	  }
	  nextState.rockIsGood[r] = false;
	  isRock = true;
	}
	if (!isRock) {
	  // Sampling empty cell.  Apply illegal action cost.
	  reward = params.costIllegal;
	}
	outcomes.push_back(RockExploreBeliefEntry(1.0, getStateId(nextState)));
      }
      break;
    case ACT_CHECK:
      // Apply check cost and no change in state.
      reward = params.costCheck;
      outcomes.push_back(RockExploreBeliefEntry(1.0, si));
      break;
    default:
      assert(0); // never reach this point
    }
  }
}

int RockExplore::getNumObservations(void) const
{
  return 2;
}

// Returns the probability of seeing observation o if action ai is applied
// and the system transitions to state si.
double RockExplore::getObsProb(int ai, int si, int o)
{
  RockExploreState s = states[si];
  RockExploreAction a(ai);

  if (a.actionType == ACT_CHECK) {
    // If rock is good, 80% chance of reading 1.  If rock is bad, 20% chance
    // of reading 1.
    if (s.rockIsGood[a.rockIndex]) {
      double obsProb[] = { 0.2, 0.8 };
      return obsProb[o];
    } else {
      double obsProb[] = { 0.8, 0.2 };
      return obsProb[o];
    }
  } else {
    // Actions other than check give no information (always the same observation)
    return (o == 0) ? 1.0 : 0.0;
  }
}


}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
