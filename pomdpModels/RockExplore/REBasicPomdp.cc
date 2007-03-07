/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-03-07 08:12:27 $
  
 @file    REBasicPomdp.cc
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

#include "REBasicPomdp.h"
#include "RockExplore.h"

using namespace std;

namespace zmdp {

// Sets result to be the distribution of possible observations when
// action ai is applied and the system transitions to state sp.
// Returns result.
REObsProbs REBasicPomdp::getObsProbs(int ai, int sp)
{
  REObsProbs obsProbs;
  obsProbs.clear();
  obsProbs.resize(getNumObservations(), 0.0);
  for (int o=0; o < getNumObservations(); o++) {
    obsProbs[o] = getObsProb(ai, sp, o);
  }
  return obsProbs;
}

// Returns the expected reward and observation probabilities when from
// belief b action ai is applied.
REObsProbsResult REBasicPomdp::getBeliefResult(const REBelief& b, int ai)
{
  REObsProbsResult result;

  result.obsProbs.clear();
  result.obsProbs.resize(getNumObservations(), 0.0);
  result.expectedReward = 0.0;

  for (int i=0; i < (int)b.size(); i++) {
    int si = b[i].index;
    REActionResult out = getActionResult(si, ai);

    // E[R | b, a] = sum_s [ P(s) * R(s,a) ]
    result.expectedReward += b[i].prob * out.reward;

    for (int j=0; j < (int)out.outcomes.size(); j++) {
      // sp is the index for the outcome state.
      int sp = out.outcomes[j].index;

      for (int o=0; o < getNumObservations(); o++) {
	// P(o | b, a) = sum_{s,sp} [ P(s | b) * P(sp | s, a) * P(o | a, sp) ]
	result.obsProbs[o] += b[i].prob * out.outcomes[j].prob * getObsProb(ai, sp, o);
      }
    }
  }

  // Normalize
  double sum = 0.0;
  for (int o=0; o < getNumObservations(); o++) {
    sum += result.obsProbs[o];
  }
  assert(sum > 0.0);
  for (int o=0; o < getNumObservations(); o++) {
    result.obsProbs[o] /= sum;
  }

  return result;
}

// Returns the updated belief when from belief b action ai is executed
// and observation o is received.
REBelief REBasicPomdp::getUpdatedBelief(const REBelief& b, int ai, int o)
{
  REBelief bp;

  // This (temporary) dense data structure will allow us to efficiently
  // combine the probabilities of outcomes that arise from different
  // starting states.
  std::vector<double> bpDense(getNumStates());

  for (int i=0; i < (int)b.size(); i++) {
    int si = b[i].index;
    REActionResult out = getActionResult(si, ai);

    for (int j=0; j < (int)out.outcomes.size(); j++) {
      // sp is the index for the outcome state.
      int sp = out.outcomes[j].index;

      // P(sp | b, a, o) = sum_s [ P(s | b) * P(sp | s, a) * P(o | a, sp) ]
      bpDense[sp] += b[i].prob * out.outcomes[j].prob * getObsProb(ai, sp, o);
    }
  }

  // Transform dense vector bpDense into standard sparse vector format bp.
  bp.clear();
  double sum = 0.0;
  for (int sp=0; sp < getNumStates(); sp++) {
    if (bpDense[sp] > 0.0) {
      bp.push_back(REBeliefEntry(bpDense[sp], sp));
      sum += bpDense[sp];
    }
  }

  // Normalize bp.
  assert(sum > 0.0);
  for (int i=0; i < (int)bp.size(); i++) {
    bp[i].prob /= sum;
  }

  return bp;
}

// Returns the index for state s.  This includes assigning an index to
// state s if it doesn't already have one.
int REBasicPomdp::getStateId(const REState& s)
{
  // Convert from state struct to string representation.
  std::string ss = getStateString(s);

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

// Uses the transition model to generate all reachable states and assign
// them index values.  This is called during initialization; before it is
// called getNumStates() is not valid.
void REBasicPomdp::generateReachableStates(void)
{
  // Initialize the stateQueue to hold the possible initial states from b0.
  REBelief b0 = getInitialBelief();
  std::queue<int> stateQueue;
  for (int i=0; i < (int)b0.size(); i++) {
    stateQueue.push(b0[i].index);
  }

  // Generate all the reachable states through breadth-first search.
  std::map<int,bool> visited;
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
	REActionResult out = getActionResult(si, ai);
	for (int i=0; i < (int)out.outcomes.size(); i++) {
	  stateQueue.push(out.outcomes[i].index);
	}
      }
    }
  }
}


// Outputs a Cassandra-format POMDP model to the given file.
void REBasicPomdp::writeCassandraModel(const std::string& outFile)
{
  // Open the output file
  std::ofstream out(outFile.c_str());
  if (!out) {
    cerr << "ERROR: couldn't open " << outFile << " for writing: "
	      << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  //////////////////////////////////////////////////////////////////////
  // Write the preamble.
  out << "discount: " << RE_DISCOUNT << endl;
  out << "values: reward" << endl;

  // Output "actions" line
  out << "actions: ";
  for (int ai=0; ai < getNumActions(); ai++) {
    out << getActionString(ai) << " ";
  }
  out << endl;

  // Output "observations" line
  out << "observations: ";
  for (int o=0; o < getNumObservations(); o++) {
    out << getObservationString(o) << " ";
  }
  out << endl;

  // Output "states" line
  out << "states: ";
  for (int si=0; si < getNumStates(); si++) {
    out << getStateString(si) << " ";
  }
  out << endl;

  // Generate sparse representation of initial belief and unpack into
  // dense representation.
  REBelief b0 = getInitialBelief();
  std::vector<double> denseB0(getNumStates(), 0.0);
  for (int i=0; i < (int)b0.size(); i++) {
    denseB0[b0[i].index] = b0[i].prob;
  }

  // Output "start" line
  out << "start: ";
  for (int si=0; si < (int)states.size(); si++) {
    out << denseB0[si] << " ";
  }
  out << endl << endl;

  //////////////////////////////////////////////////////////////////////
  // Write the main body.
  char buf[256];
  for (int si=0; si < (int)states.size(); si++) {
    std::string ss = getStateString(si);

    for (int ai=0; ai < getNumActions(); ai++) {
      REActionResult res = getActionResult(si, ai);
      std::string as = getActionString(ai);

      // Output R line for state=si, action=ai
      snprintf(buf, sizeof(buf), "R: %-3s : %-10s : * : * %f",
	       as.c_str(), ss.c_str(), res.reward);
      out << buf << endl;

      // Output T lines for state=si, action=ai
      for (int i=0; i < (int)res.outcomes.size(); i++) {
	std::string sps = getStateString(res.outcomes[i].index);
	snprintf(buf, sizeof(buf), "T: %-3s : %-10s : %-10s %f",
		 as.c_str(), ss.c_str(), sps.c_str(),
		 res.outcomes[i].prob);
	out << buf << endl;
      }

      // Output O lines for action=ai, outcome=si
      for (int o=0; o < getNumObservations(); o++) {
	double obsProb = getObsProb(ai, si, o);
	snprintf(buf, sizeof(buf), "O: %-3s : %-10s : o%d %f",
		 as.c_str(), ss.c_str(), o, obsProb);
	out << buf << endl;
      }
      out << endl;
    }
  }

  out.close();
}

// Returns a stochastically selected state index from the distribution b.
int REBasicPomdp::chooseStochasticOutcome(const REBelief& b)
{
  // Generate a random floating point value between 0 and 1.
  double p = ((double) std::rand()) / RAND_MAX;
  
  // Select an outcome based on p.
  for (int i=0; i < (int)b.size(); i++) {
    p -= b[i].prob;
    if (p <= 0) {
      return b[i].index;
    }
  }
  assert(0); // never reach this point
}

// Returns a stochastically selected observation from the distribution obsProbs.
int REBasicPomdp::chooseStochasticOutcome(const REObsProbs& obsProbs)
{
  // Generate a random floating point value between 0 and 1.
  double p = ((double) std::rand()) / RAND_MAX;

  // Select an outcome based on p.
  for (int o=0; o < (int)obsProbs.size(); o++) {
    p -= obsProbs[o];
    if (p <= 0) {
      return o;
    }
  }
  assert(0); // never reach this point
}

// Returns the most likely state according to the distribution b.
int REBasicPomdp::getMostLikelyState(const REBelief& b)
{
  double maxProb = 0.0;
  int maxProbState = -1;
  
  for (int i=0; i < (int)b.size(); i++) {
    if (b[i].prob > maxProb) {
      maxProb = b[i].prob;
      maxProbState = b[i].index;
    }
  }
  return maxProbState;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
