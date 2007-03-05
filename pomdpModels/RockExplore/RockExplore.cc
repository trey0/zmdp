/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-03-05 23:33:24 $
  
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

// The constructor -- you must specify the problem params at initialization.
RockExplore::RockExplore(const RockExploreParams& _params) :
  params(_params)
{
  // Make sure the terminal state is assigned an id before any other state
  // so that it gets the id 0, handy for debugging.
  getStateId(getTerminalState());
}

// Sets result to be the initial belief.  Returns result.
RockExploreBelief& RockExplore::getInitialBelief(RockExploreBelief& result)
{
  // Initialize the probability of each rock being good to the prior value
  // for the problem.
  RockExploreRockMarginals probRockIsGood(params.numRocks, params.rockGoodPrior);

  // Construct a belief distribution from the initial robot position and
  // the priors.
  return getBelief(result, params.initPos, probRockIsGood);
}

// Returns the terminal state.
const RockExploreState& RockExplore::getTerminalState(void) {
  static RockExploreState s;
  s.isTerminalState = true;
  return s;
}

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

// Sets result to be the string form for the state with id si. Returns result.
std::string& RockExplore::getStateString(std::string& result, int si) const
{
  return getStateString(result, states[si]);
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
				 const RockExploreRockMarginals& probRockIsGood) const
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
    outs << endl;

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
      outs << "x" << endl;
    }

    // Bottom boundary
    for (pos.x=0; pos.x < params.width*2+3; pos.x++) {
      outs << "#";
    }
    outs << endl;

    // Probability of rocks being good
    outs << "[";
    for (int r=0; r < params.numRocks; r++) {
      char pbuf[20];
      snprintf(pbuf, sizeof(pbuf), "%5.3lf", probRockIsGood[r]);
      outs << ((char) ('A'+r)) << " " << pbuf << " ";
    }
    outs << "]" << endl;

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
    outcomes.push_back(RockExploreBeliefEntry(1.0, getStateId(getTerminalState())));
  } else {
    switch (a.actionType) {
    case ACT_MOVE:
      // Apply the change in position.
      nextState.robotPos.x += a.deltaPos.x;
      nextState.robotPos.y += a.deltaPos.y;

      // Check if the move stays on the map.
      if (0 <= nextState.robotPos.x && nextState.robotPos.x < params.width
	  && 0 <= nextState.robotPos.y && nextState.robotPos.y < params.height) {
	// New position is in bounds.  Transition to the new position
	// and apply the move cost.
	reward = params.costMove;
	outcomes.push_back(RockExploreBeliefEntry(1.0, getStateId(nextState)));
      } else if (nextState.robotPos.x == params.width) {
	// Reached exit area.  Transition to terminal state and receive
	// the reward for exiting.
	reward = params.rewardExit;
	outcomes.push_back(RockExploreBeliefEntry(1.0, getStateId(getTerminalState())));
      } else {
	// Illegal attempt to move off the map.  No change in position and
	// apply the illegal action cost.
	reward = params.costIllegal;
	outcomes.push_back(RockExploreBeliefEntry(1.0, si));
      }
      break;
    case ACT_SAMPLE: {
      bool samplingInEmptyCell = true;
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
	reward = params.costIllegal;
      }
      outcomes.push_back(RockExploreBeliefEntry(1.0, getStateId(nextState)));
      break;
    }
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
  // Possible observations are 0 or 1.
  return 2;
}

// Returns the probability of seeing observation o if action ai is applied
// and the system transitions to state si.
double RockExplore::getObsProb(int ai, int si, int o) const
{
  // Translate from state id si to state struct and from action id ai to action
  // struct.
  RockExploreState s = states[si];
  RockExploreAction a(ai);

  if (a.actionType == ACT_CHECK && !s.isTerminalState) {
    // If rock is bad (row 0 of the table), the relative probabilities
    // of observations 0 and 1 are 80/20.  If rock is good (row 1 of the
    // table), the probabilities are 20/80.
    static double obsProbTable[] = { 0.8, 0.2,
				     0.2, 0.8 };

    // This code block calculates the Manhattan distance from the robot
    // to the rock.  Note that in the basic version of the model the
    // sensor noise does not depend on the distance.
    /*
    RockExplorePos rockPos = params.rockPos[a.rockIndex];
    int manhattanDistance = std::abs(rockPos.x - s.robotPos.x)
      + std::abs(rockPos.y - s.robotPos.y);
    */

    int obsProbRow = s.rockIsGood[a.rockIndex] ? 1 : 0;
    int obsProbCol = o;
    return obsProbTable[2*obsProbRow + obsProbCol];
  } else {
    // Actions other than check give no useful information (always
    // return observation 0)
    return (o == 0) ? 1.0 : 0.0;
  }
}

// Outputs a Cassandra-format POMDP model to the given file.
void RockExplore::writeCassandraModel(const std::string& outFile)
{
  // Open the output file
  std::ofstream out(outFile.c_str());
  if (!out) {
    cerr << "ERROR: couldn't open " << outFile << " for writing: "
	      << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  // Initialize the stateQueue to hold the possible initial states from b0.
  RockExploreBelief b0;
  getInitialBelief(b0);
  std::queue<int> stateQueue;
  for (int i=0; i < (int)b0.size(); i++) {
    stateQueue.push(b0[i].si);
  }

  // Generate all the reachable states through breadth-first search.
  std::map<int,bool> visited;
  double reward;
  RockExploreBelief outcomes;
  while (!stateQueue.empty()) {
    int si = stateQueue.front();
    stateQueue.pop();

    // Process si if it is not already marked as visited.
    if (visited.find(si) == visited.end()) {
      // Mark si as visited.
      visited[si] = true;
      
      // Generate a list of outcomes from applying each action to si
      // and add the outcomes to the stateQueue.
      for (int ai=0; ai < getNumActions(); ai++) {
	getActionResult(reward, outcomes,
			si, ai);
	for (int i=0; i < (int)outcomes.size(); i++) {
	  stateQueue.push(outcomes[i].si);
	}
      }
    }
  }

  // Write the preamble.
  out << "discount: " << params.discountFactor << endl;
  out << "values: reward" << endl;

  out << "actions: ";
  for (int ai=0; ai < getNumActions(); ai++) {
    out << RockExploreAction::getString(ai) << " ";
  }
  out << endl;

  out << "observations: ";
  for (int o=0; o < getNumObservations(); o++) {
    out << "o" << o << " ";
  }
  out << endl;

  out << "states: ";
  std::string ss;
  for (int si=0; si < (int)states.size(); si++) {
    out << getStateString(ss,si) << " ";
  }
  out << endl << endl;

  // Write the main body.
  char buf[256];
  for (int si=0; si < (int)states.size(); si++) {
    std::string ss, sps;
    getStateString(ss, si);

    for (int ai=0; ai < getNumActions(); ai++) {
      getActionResult(reward, outcomes, si, ai);

      // Output R line for state=si, action=ai
      snprintf(buf, sizeof(buf), "R: %-3s : %-10s : * : * %f",
	       RockExploreAction::getString(ai), ss.c_str(), reward);
      out << buf << endl;

      // Output T lines for state=si, action=ai
      for (int i=0; i < (int)outcomes.size(); i++) {
	getStateString(sps, outcomes[i].si);
	snprintf(buf, sizeof(buf), "T: %-3s : %-10s : %-10s %f",
		 RockExploreAction::getString(ai), ss.c_str(), sps.c_str(),
		 outcomes[i].prob);
	out << buf << endl;
      }

      // Output O lines for action=ai, outcome=si
      for (int o=0; o < getNumObservations(); o++) {
	double obsProb = getObsProb(ai, si, o);
	snprintf(buf, sizeof(buf), "O: %-3s : %-10s : o%d %f",
		 RockExploreAction::getString(ai), ss.c_str(), o, obsProb);
	out << buf << endl;
      }
      out << endl;
    }
  }

  out.close();
}

// Stochastically selects an outcome state s according to the
// probabilities in the belief b.  Returns the state index for s.
int RockExplore::chooseStochasticOutcome(const RockExploreBelief& b) const
{
  // Generate a random floating point value between 0 and 1.
  double p = ((double) rand()) / RAND_MAX;

  // Select an outcome based on p.
  for (int i=0; i < (int)b.size(); i++) {
    p -= b[i].prob;
    if (p <= 0) {
      return b[i].si;
    }
  }

  // If the probabilities in b add up to 1.0, we should reach this point
  // with probability 0.  However, due to roundoff error b may not be
  // perfectly normalized, so this fallback case is nice to avoid a
  // crash.
  assert(p < 1e-10);
  return b[0].si;
}

// Calculates the marginal probability that each rock is good from the
// given belief b.  Sets result to be the vector of marginals.  Returns
// result.
RockExploreRockMarginals&
RockExplore::getMarginals(RockExploreRockMarginals& result,
			  const RockExploreBelief& b) const
{
  // Initialize the result vector to all zero values.
  result.resize(params.numRocks, 0.0);

  // Iterate through the outcomes in the belief vector.
  for (int i=0; i < (int)b.size(); i++) {
    // Translate from the state index of the outcome, b[i].si, to the
    // corresponding state struct s.
    RockExploreState s = states[b[i].si];
    for (int r=0; r < params.numRocks; r++) {
      // If rock r is good in outcome i, add the probability of
      // outcome i to result[r].
      if (s.rockIsGood[r]) {
	result[r] += b[i].prob;
      }
    }
  }
  return result;
}

// Generates a belief in which all the states have the given robotPos
// and a distribution of rockIsGood values consistent with the marginals
// specified by probRockIsGood.  This is effectively the inverse of the
// getMarginals() function.
RockExploreBelief&
RockExplore::getBelief(RockExploreBelief& result,
		       const RockExplorePos& robotPos,
		       const RockExploreRockMarginals& probRockIsGood)
{
  result.clear();

  // Fill in the variables of sp that will be the same across all outcomes
  // in the result.
  RockExploreState sp;
  sp.isTerminalState = false;
  sp.robotPos = robotPos;

  // Initialize to the 'first' rockIsGood vector, in which all rockIsGood
  // entries are false.
  std::vector<bool> rockIsGood(params.numRocks, false);

  while (1) {
    // Calculate probability of the current rockIsGood vector according
    // to the specified marginals.
    double prob = 1.0;
    for (int r=0; r < params.numRocks; r++) {
      prob *= (rockIsGood[r]) ? probRockIsGood[r] : (1.0 - probRockIsGood[r]);
    }

    // If the probability is non-zero, add an entry to the resulting belief.
    if (prob > 0.0) {
      sp.rockIsGood = rockIsGood;
      std::string sps;
      result.push_back(RockExploreBeliefEntry(prob, getStateId(sp)));
    }

    // Advance to next rockIsGood vector.  (This code basically adds one
    // to a bit vector.)
    bool reachedLastCombination = true;
    for (int r=params.numRocks-1; r >= 0; r--) {
      if (!rockIsGood[r]) {
	rockIsGood[r] = true;
	for (int rr=r+1; rr < params.numRocks; rr++) {
	  rockIsGood[rr] = false;
	}
	reachedLastCombination = false;
	break;
      }
    }
    if (reachedLastCombination) break;
  }

  return result;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/03/05 08:58:26  trey
 * initial check-in
 *
 *
 ***************************************************************************/
