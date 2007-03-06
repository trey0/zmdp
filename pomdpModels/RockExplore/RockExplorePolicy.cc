/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-03-06 04:32:33 $
  
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
void MDPValueFunction::init(void)
{
  V.clear();
  V.resize(modelG->states.size(), 0.0);
  Vp.resize(modelG->states.size());
}

// Performs a sweep of value iteration, updating all states.  Returns the
// maximum residual between the functions before and after the sweep.
double MDPValueFunction::valueIterationSweep(void) {
  double maxResidual = 0.0;
  for (int s=0; s < (int)V.size(); s++) {
    Vp[s] = getUpdatedValue(s);
    maxResidual = std::max(maxResidual, fabs(V[s] - Vp[s]));
  }
  return maxResidual;
}

// Performs value iteration until the maximum residual between
// successive sweeps is smaller than eps. 
void MDPValueFunction::valueIterationToResidual(double eps)
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
double MDPValueFunction::getValue(int s) const
{
  return V[s];
}

// Returns the expected value of a belief V(b) = sum_s P(s | b) V(s)
double MDPValueFunction::getValue(const RockExploreBelief& b) const
{
  double expectedValue = 0.0;
  for (int i=0; i < (int)b.size(); i++) {
    int s = b[i].index;
    expectedValue += b[i].prob * V[s];
  }
  return expectedValue;
}

// Returns HV(s) = max_a [ R(s,a) + discount * sum_s' P(s' | s,a) V(s') ].
double MDPValueFunction::getUpdatedValue(int s) const
{
  double maxVal = -99e+20;
  for (int a=0; a < modelG->getNumActions(); a++) {
    double reward;
    RockExploreBelief outcomes;
    modelG->getActionResult(reward, outcomes, s, a);

    double val = reward + modelG->params.discountFactor * getValue(outcomes);
    maxVal = std::max(maxVal, val);
  }
  return maxVal;
}

// Returns HV(b) = max_a [ R(b,a) + discount * sum_o P(b' | b,a,o) V(b') ]
double MDPValueFunction::getUpdatedValue(const RockExploreBelief& b) const
{
  double maxVal = -99e+20;
  for (int a=0; a < modelG->getNumActions(); a++) {
    double expectedReward;
    RockExploreObsProbs obsProbs;
    modelG->getActionResult(expectedReward, obsProbs, b, a);

    double val = 0.0;
    for (int o=0; o < modelG->getNumObservations(); o++) {
      RockExploreBelief bp;
      modelG->getUpdatedBelief(bp, b, a, o);
      val += obsProbs[o] * getValue(bp);
    }
    val = expectedReward + modelG->params.discountFactor * val;
    maxVal = std::max(maxVal, val);
  }
  return maxVal;
}

static int getUserAction(void)
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

    printf("\n*** Sorry, I didn't understand that action ***\n");
  }
}

int UserPolicy::chooseAction(void)
{
  return getUserAction();
}

HeuristicPolicy::HeuristicPolicy(void)
{
  vfn.valueIterationToResidual(1e-5);
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
  return 1;
}

// Chooses an action according to the voting heuristic.
int VotingPolicy::chooseAction(void)
{
  return 1;
}

// Chooses an action according to the most likely state heuristic.
int MostLikelyPolicy::chooseAction(void)
{
  return 1;
}

// Chooses an action according to the two-step lookahead heuristic.
int TwoStepPolicy::chooseAction(void)
{
  return 1;
}

enum PolicyTypes {
  P_QMDP=1,
  P_VOTING=2,
  P_MOST_LIKELY=3,
  P_TWO_STEP=4,
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

static int getPolicyType(void)
{
  while (1) {
    printf("\nPolicy type menu\n"
	   "\n"
	   "  1 - QMDP heuristic\n"
	   "  2 - Voting heuristic\n"
	   "  3 - Most likely state heuristic\n"
	   "  4 - Two-step lookahead heuristic\n"
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

PomdpExecCore* getPolicy(void)
{
  int policyType = getPolicyType();

  switch (policyType) {
  case P_QMDP:
    return new QMDPPolicy();
  case P_VOTING:
    return new VotingPolicy();
  case P_MOST_LIKELY:
    return new MostLikelyPolicy();
  case P_TWO_STEP:
    return new TwoStepPolicy();
  case P_ZMDP: {
    ZMDPConfig* config = new ZMDPConfig();
    config->readFromString("<defaultConfig>", defaultConfig.data);
    MaxPlanesLowerBoundExec* policy = new MaxPlanesLowerBoundExec;
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
 *
 ***************************************************************************/
