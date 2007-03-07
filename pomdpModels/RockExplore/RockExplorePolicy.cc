/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2007-03-07 03:52:34 $
  
 @file    RockExplorePolicy.cc
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

#include "zmdpMainConfig.h"

#include "RockExplorePolicy.h"

using namespace std;

namespace zmdp {

// Initializes the value function to have zero value for all states.
void REValueFunction::init(void)
{
  V.clear();
  V.resize(modelG->states.size(), 0.0);
  Vp.clear();
  Vp.resize(modelG->states.size());
}

// Performs a sweep of value iteration, updating all states.  Returns the
// maximum residual between the functions before and after the sweep.
double REValueFunction::valueIterationSweep(void) {
  double maxResidual = 0.0;
  for (int s=0; s < (int)V.size(); s++) {
    Vp[s] = getUpdatedValue(s);
    maxResidual = std::max(maxResidual, fabs(V[s] - Vp[s]));
  }
  V = Vp;
  return maxResidual;
}

// Performs value iteration until the maximum residual between
// successive sweeps is smaller than eps. 
void REValueFunction::valueIterationToResidual(double eps)
{
  printf("Generating MDP value function");
  fflush(stdout);
  init();
  while (1) {
    double residual = valueIterationSweep();
    if (residual < eps) break;

    printf(".");
    fflush(stdout);
  }
  printf(" done.\n");
}

// Returns the value V(s).
double REValueFunction::getValue(int s) const
{
  return V[s];
}

// Returns Q(s,a).
double REValueFunction::getQ(int s, int a) const
{
  double Rsa;
  RockExploreBelief outcomes;
  modelG->getActionResult(Rsa, outcomes, s, a);
  return Rsa + modelG->params.discountFactor * getValue(outcomes);
}

// Returns arg max_a Q(s,a).
int REValueFunction::getMaxQAction(int s) const
{
  double maxQsa = -99e+20;
  int maxQAction = -1;
  for (int a=0; a < modelG->getNumActions(); a++) {
    double Qsa = getQ(s,a);
    if (Qsa > maxQsa) {
      maxQsa = Qsa;
      maxQAction = a;
    }
  }
  return maxQAction;
}

// Returns the value of a belief V(b) = sum_s b(s) V(s)
double REValueFunction::getValue(const RockExploreBelief& b) const
{
  double expectedValue = 0.0;
  for (int i=0; i < (int)b.size(); i++) {
    int s = b[i].index;
    expectedValue += b[i].prob * V[s];
  }
  return expectedValue;
}

// Returns Q(b,a).
double REValueFunction::getQ(const RockExploreBelief& b, int a) const
{
  double Rba;
  RockExploreObsProbs obsProbs;
  modelG->getActionResult(Rba, obsProbs, b, a);
  
  double nextStateVal = 0.0;
  for (int o=0; o < modelG->getNumObservations(); o++) {
    RockExploreBelief nextBelief;
    if (obsProbs[o] > 0.0) {
      modelG->getUpdatedBelief(nextBelief, b, a, o);
      
      nextStateVal += obsProbs[o] * getValue(nextBelief);
    }
  }
  return Rba + modelG->params.discountFactor * nextStateVal;
}

// Returns HV(s) = max_a Q(s,a).
double REValueFunction::getUpdatedValue(int s) const
{
  return getQ(s, getMaxQAction(s));
}

// Returns arg max_a Q(b,a).
int REValueFunction::getMaxQAction(const RockExploreBelief& b) const
{
  double maxQba = -99e+20;
  int maxQAction = -1;
  for (int a=0; a < modelG->getNumActions(); a++) {
    double Qba = getQ(b,a);
    if (Qba > maxQba) {
      maxQba = Qba;
      maxQAction = a;
    }
  }
  return maxQAction;
}

// Returns HV(b) = max_a Q(b,a).
double REValueFunction::getUpdatedValue(const RockExploreBelief& b) const
{
  return getQ(b, getMaxQAction(b));
}

// Chooses an action according to the "user" policy, meaning we
// just ask the user for an action.
int UserPolicy::chooseAction(void)
{
  while (1) {
    cout << "\nChoose action from [";
    for (int ai=0; ai < modelG->getNumActions(); ai++) {
      cout << RockExploreAction::getString(ai) << " ";
    }
    cout << "]: ";
    cout.flush();

    std::string as;
    cin >> as;
    for (int ai=0; ai < modelG->getNumActions(); ai++) {
      if (as == RockExploreAction::getString(ai)) {
	return ai;
      }
    }

    printf("\n*** Sorry, I didn't understand that action. ***\n");
  }
}

// Informs the policy that the system is at the initial belief.
void HeuristicPolicy::setToInitialBelief(void)
{
  modelG->getInitialBelief(b);
}

// Informs the policy that action a was applied and observation o was received.
void HeuristicPolicy::advanceToNextBelief(int a, int o)
{
  RockExploreBelief bp;
  modelG->getUpdatedBelief(bp, b, a, o);
  b = bp;
}

// Chooses an action according to the QMDP heuristic.
int QMDPPolicy::chooseAction(void)
{
  return vfn.getMaxQAction(b);
}

// Chooses an action according to the voting heuristic.
int VotingPolicy::chooseAction(void)
{
  // Initialize votes for each action to 0.
  std::vector<double> voteTotals(modelG->getNumActions(), 0.0);

  // Each state s votes for the best action to take from that state; the
  // votes are weighted according to b(s).
  for (int i=0; i < (int)b.size(); i++) {
    int s = b[i].index;
    voteTotals[vfn.getMaxQAction(s)] += b[i].prob;
  }

  // Calculate the action with the most votes.
  int bestAction = -1;
  double bestVoteTotal = -99e+20;
  for (int a=0; a < modelG->getNumActions(); a++) {
    if (voteTotals[a] > bestVoteTotal) {
      bestAction = a;
      bestVoteTotal = voteTotals[a];
    }
  }

  return bestAction;
}

// Chooses an action according to the most likely state heuristic.
int MostLikelyPolicy::chooseAction(void)
{
  // Calculate the most likely state s* and return argmax_a Q(s*, a).
  return vfn.getMaxQAction(modelG->getMostLikelyState(b));
}

/**********************************************************************
 * BEGIN DUAL-MODE FUNCTIONS
 **********************************************************************/

// Arbitrary threshold value.  Smaller values cause the dual-mode heuristic
// to more readily employ entropy-minimizing actions.
#define DUAL_MODE_ENTROPY_THRESHOLD (0.1)

// Returns the normalized entropy.
//   H(b) = -sum_s b(s) log(b(s))  -- Cassandra p. 264.
//   HBar(b) = H(b)/H(u)           -- Cassandra p. 266
double getHBar(const RockExploreBelief& b)
{
  double sum = 0.0;
  for (unsigned int i=0; i < b.size(); i++) {
    double p = b[i].prob;
    if (p > 0.0) {
      sum += p * log(p);
    }
  }
  double Hb = -sum;
  double Hu = -log(1.0 / ((double) modelG->getNumStates()));
  return Hb / Hu;
}

// Chooses an action according to the dual-mode heuristic.
int DualModePolicy::chooseAction(void)
{
  if (getHBar(b) > DUAL_MODE_ENTROPY_THRESHOLD) {

    // Take action that minimizes expected entropy on the next time step:
    //
    //   SH(b,a) = sum_o P(o | b,a) HBar(tau(b,a,o)) -- Cassandra p. 264
    //   a^* = min_a SH(b,a)                         -- Cassandra p. 266
    //
    // (This is the DM, not ADM, heuristic.)

    double minSH = 99e+20;
    int minSHAction = -1;
    for (int a=0; a < modelG->getNumActions(); a++) {
      double reward;
      RockExploreObsProbs obsProbs;
      modelG->getActionResult(reward, obsProbs, b, a);

      double sumHBar = 0.0;
      for (int o=0; o < modelG->getNumObservations(); o++) {
	if (obsProbs[o] > 0.0) {
	  RockExploreBelief nextBelief;
	  modelG->getUpdatedBelief(nextBelief, b, a, o);
	  sumHBar += obsProbs[o] * getHBar(nextBelief);
	}
      }

      if (sumHBar < minSH) {
	minSH = sumHBar;
	minSHAction = a;
      }
    }
    return minSHAction;

  } else {

    // Take the QMDP action.
    return vfn.getMaxQAction(b);

  }
}

/**********************************************************************
 * END DUAL-MODE FUNCTIONS
 **********************************************************************/

enum PolicyTypes {
  P_QMDP=1,
  P_VOTING=2,
  P_MOST_LIKELY=3,
  P_DUAL_MODE=4,
  P_ZMDP=5
};

// Accepts a numeric value input on console.
int getUserChoice(void)
{
  int choice;
  char buf[256];
  cin.getline(buf, sizeof(buf));
  if (1 != sscanf(buf, "%d", &choice)) {
    return -1;
  }
  return choice;
}

// Queries the user for their desired policy type
static int getPolicyType(void)
{
  while (1) {
    printf("\nPolicy type menu\n"
	   "\n"
	   "  1 - QMDP heuristic\n"
	   "  2 - Voting heuristic\n"
	   "  3 - Most likely state heuristic\n"
	   "  4 - Dual-mode heuristic\n"
	   "  5 - Read zmdpSolve-generated policy from out.policy\n"
	   "\n"
	   "Your choice: "
	   );
    fflush(stdout);

    int choice = getUserChoice();

    if (1 <= choice && choice <= 5) {
      return choice;
    } else {
      printf("\n*** Sorry, I didn't understand that choice ***\n");
    }
  }
}

// Queries the user for their desired policy type and returns a policy.
PomdpExecCore* getPolicy(void)
{
  // Only need to run value iteration once even if running multiple policies.
  static REValueFunction* vfn = NULL;
  if (NULL == vfn) {
    vfn = new REValueFunction();
    vfn->valueIterationToResidual(1e-3);
  }

  int policyType = getPolicyType();

  switch (policyType) {
  case P_QMDP:
    return new QMDPPolicy(*vfn);
  case P_VOTING:
    return new VotingPolicy(*vfn);
  case P_MOST_LIKELY:
    return new MostLikelyPolicy(*vfn);
  case P_DUAL_MODE:
    return new DualModePolicy(*vfn);
  case P_ZMDP: {
    ZMDPConfig* config = new ZMDPConfig();
    config->readFromString("<defaultConfig>", defaultConfig.data);
    MaxPlanesLowerBoundExec* policy = new MaxPlanesLowerBoundExec();
    policy->init("RockExplore.pomdp",
		 /* useFastParser = */ false,
		 "out.policy",
		 *config);
    return policy;
  }
  default:
    assert(0); // never reach this point
  };
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2007/03/06 08:46:56  trey
 * many tweaks
 *
 * Revision 1.3  2007/03/06 07:49:22  trey
 * refactored, implemented TwoStepPolicy
 *
 * Revision 1.2  2007/03/06 06:37:52  trey
 * implementing heuristics
 *
 * Revision 1.1  2007/03/06 04:32:33  trey
 * initial check-in
 *
 *
 ***************************************************************************/
