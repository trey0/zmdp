/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.16 $  $Author: trey $  $Date: 2007-04-01 20:48:31 $

 @file    zmdpRockExplore.cc
 @brief   No brief

 Copyright (c) 2002-2006, Trey Smith.  All rights reserved.

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

#include <assert.h>
#include <sys/time.h>
#include <getopt.h>
#include <signal.h>

#include <iostream>
#include <fstream>

#include "RockExplore.h"
#include "zmdpCommonTime.h"
//#include "solverUtils.h"
#include "zmdpMainConfig.h"
#include "RockExplorePolicy.h"

#include "zmdpMainConfig.cc" // embed default config file

using namespace std;
//using namespace MatrixUtils;
using namespace zmdp;

void waitForUser(void)
{
  printf("Press [ENTER] to continue\n");
  char buf[256];
  cin.getline(buf, sizeof(buf));
}

enum SimModes {
  MODE_MANUAL,
  MODE_VISUALIZE,
  MODE_EVAL
};

struct RockExploreSim {
  int mode;
  MDPExecCore* policy;
  REBelief b;
  int si;
  int timeStep;
  double totalReward;

  RockExploreSim(int _mode, MDPExecCore* _policy) :
    mode(_mode),
    policy(_policy)
  {}

  void initialize(void) {
    b = m.getInitialBelief();
    si = m.chooseStochasticOutcome(b);
    policy->setToInitialState();
    timeStep = 0;
    totalReward = 0;
  }

  bool takeStep(void) {
    bool doVisualize = (mode != MODE_EVAL);
    
    if (doVisualize) {
      // Display current state and belief
      printf("Current map is:\n"
	     "\n"
	     "%s\n",
	     m.getMap(si,b).c_str());
    }
      
    int ai = policy->chooseAction();

    // Check if the selected policy type is implemented.
    if (0 == timeStep && -1 == ai) {
      return false;
    }

    if (MODE_VISUALIZE == mode) {
      /* display action selected by policy and wait for user to continue */
      printf("Policy selects action %s\n", m.getActionString(ai).c_str());
      waitForUser();
    }
      
    // Calculate results of action
    REActionResult out = m.getActionResult(si, ai);
    int sp = m.chooseStochasticOutcome(out.outcomes);
    totalReward += pow(RE_DISCOUNT, timeStep) * out.reward;
    int o = m.chooseStochasticOutcome(m.getObsProbs(ai, sp));
    if (doVisualize) {
      printf("Observation = o%d, immediate reward = %lf\n", o, out.reward);
    }
    REBelief bp = m.getUpdatedBelief(b, ai, o);
      
    // Advance to next state and belief
    policy->advanceToNextState(ai, o);
    si = sp;
    b = bp;
    timeStep++;

    return true;
  }

  bool run(void) {
    initialize();

    const int maxSteps = 100;
    while (1) {
      if (!takeStep()) {
	return false;
      }

      // If state is terminal, end the run
      REState s = m.states[si];
      if (s.isTerminalState) {
	if (MODE_EVAL != mode) {
	  printf("\nReached terminal state, total discounted reward = %.3lf.\n",
		 totalReward);
	}
	break;
      } else if (MODE_EVAL == mode && timeStep >= maxSteps) {
	// Stop evaluation run after at most 100 time steps.
	break;
      }
    }
    return true;
  }
};

void doManual(void)
{
  UserPolicy policy;
  RockExploreSim sim(MODE_MANUAL, &policy);
  sim.run();
}

void doVisualize(void)
{
  MDPExecCore* policy = getPolicy(getUserPolicyType());
  RockExploreSim sim(MODE_VISUALIZE, policy);
  if (!sim.run()) {
    printf("Sorry, that policy type is not yet implemented.\n");
  }
  delete policy;
}

struct EvalResult {
  double mean;
  double conf95;
};

EvalResult evaluatePolicy(int policyType)
{
  MDPExecCore* policy = getPolicy(policyType);
  RockExploreSim sim(MODE_EVAL, policy);

  const int n = 100;
  std::vector<double> samples(n);

  // do runs
  printf("Evaluating %s...", getPolicyName(policyType));
  fflush(stdout);
  for (int i=0; i < n; i++) {
    if (!sim.run()) {
      printf("(oops, policy %s not implemented)\n", getPolicyName(policyType));
      EvalResult result;
      result.mean = -999;
      result.conf95 = -999;
      return result;
    }
    samples[i] = sim.totalReward;
    printf(".");
    fflush(stdout);
  }

  // summarize statistics
  double sum = 0;
  double sqsum = 0;
  for (int i=0; i < n; i++) {
    sum += samples[i];
    sqsum += samples[i] * samples[i];
  }
  EvalResult result;
  result.mean = sum / n;

  double variance = (sqsum - sum*sum/n) / (n-1);
  // this check in case round-off error causes variance to be
  // slightly negative
  assert(variance > -1e-10);
  if (variance < 0) {
    variance = 0;
  }

  double stdev = sqrt(variance);
  stdev = sqrt(variance);
  result.conf95 = 1.96 * stdev/sqrt((double)n);
  printf(" %.3lf +/- %.3lf\n", result.mean, result.conf95);

  delete policy;
  return result;
}

void doEvaluateOne(void)
{
  int policyType = getUserPolicyType();
  EvalResult result = evaluatePolicy(policyType);
  printf("\n"
	 "Expected discounted reward 95%% confidence interval:\n");
  printf("  %-10s   %8.3lf +/- %7.3lf\n",
	 getPolicyName(policyType), result.mean, result.conf95);
}

void doEvaluateAll(void)
{
  EvalResult results[NUM_POLICIES+1];
  for (int i=1; i <= NUM_POLICIES; i++) {
    results[i] = evaluatePolicy(i);
  }
  printf("\n"
	 "Expected discounted reward 95%% confidence interval:\n");
  for (int i=1; i <= NUM_POLICIES; i++) {
    printf("  %-10s   %8.3lf +/- %7.3lf\n",
	       getPolicyName(i), results[i].mean, results[i].conf95);
  }
}

void doDisplayMDPPolicy(void)
{
  REValueFunction vfn;
  vfn.valueIterationToResidual(1e-3);

  std::vector<int> mdpPolicy(m.getNumStates());
  for (int si=0; si < m.getNumStates(); si++) {
    mdpPolicy[si] = vfn.getMaxQAction(si);
  }

  REState s;
  s.isTerminalState = false;
  s.rockIsGood.resize(RE_NUM_ROCKS);

  printf("MDP policy:\n"
	 "\n");
  for (int rgood=0; rgood < (1<<RE_NUM_ROCKS); rgood++) {
    printf("r");
    for (int r=0; r < RE_NUM_ROCKS; r++) {
      s.rockIsGood[r] = rgood & (1<<(RE_NUM_ROCKS-r-1));
      printf("%d", (int)s.rockIsGood[r]);
    }
    printf(":\n");

    for (s.robotPos.y=RE_HEIGHT-1; s.robotPos.y >= 0; s.robotPos.y--) {
      printf("  ");
      for (s.robotPos.x=0; s.robotPos.x < RE_WIDTH; s.robotPos.x++) {
	int si = m.getStateId(s);
	printf("%-3s ", m.getActionString(mdpPolicy[si]).c_str());
      }
      printf("\n");
    }
    printf("\n");
  }
}

int main(int argc, char **argv) {
  // Initialize the random number generator
  srand(time(0));
  // Seems the random number generator needs to "clear its throat" on some systems.
  rand();

  while (1) {
    printf("\nMain menu\n"
	   "\n"
	   "  1 - Write out the RockExplore model to RockExplore.pomdp\n"
	   "  2 - Manually control the robot in the simulator\n"
	   "  3 - Visualize a policy in the simulator\n"
	   "  4 - Evaluate the quality of one policy\n"
	   "  5 - Evaluate the quality of all policies (takes a while)\n"
	   "  6 - Display the MDP policy\n"
	   "\n"
	   "Your choice (ctrl-C to quit): "
	   );
    fflush(stdout);

    int choice = getUserChoice();

    switch (choice) {
    case 1:
      m.writeCassandraModel("RockExplore.pomdp");
      printf("\nModel written to RockExplore.pomdp\n");
      exit(0);
      break;
    case 2:
      doManual();
      exit(0);
    case 3:
      doVisualize();
      exit(0);
      break;
    case 4:
      doEvaluateOne();
      exit(0);
      break;
    case 5:
      doEvaluateAll();
      exit(0);
      break;
    case 6:
      doDisplayMDPPolicy();
      exit(0);
      break;
    default:
      printf("\n*** Sorry, I didn't understand that choice ***\n");
    }
  }

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2007/03/23 00:03:33  trey
 * fixed to reflect migration from PomdpExec to MDPExec base class
 *
 * Revision 1.14  2007/03/07 09:10:50  trey
 * added display of MDP policy
 *
 * Revision 1.13  2007/03/07 08:50:35  trey
 * cleaned up for improved readability
 *
 * Revision 1.12  2007/03/07 08:34:14  trey
 * fixed bugs introduced during refactoring
 *
 * Revision 1.11  2007/03/07 08:12:27  trey
 * refactored things
 *
 * Revision 1.10  2007/03/07 05:46:43  trey
 * implemented evaluator, fixed bugs in sim
 *
 * Revision 1.9  2007/03/07 03:52:12  trey
 * tweaked appearance of map
 *
 * Revision 1.8  2007/03/06 08:46:56  trey
 * many tweaks
 *
 * Revision 1.7  2007/03/06 06:52:30  trey
 * avoided potential issues with random numbers
 *
 * Revision 1.6  2007/03/06 06:37:52  trey
 * implementing heuristics
 *
 * Revision 1.5  2007/03/06 04:43:06  trey
 * debugged missing calls to policy
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
