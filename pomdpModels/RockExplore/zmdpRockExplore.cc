/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.9 $  $Author: trey $  $Date: 2007-03-07 03:52:12 $

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
  PomdpExecCore* policy;
  RockExploreBelief b;
  int si;
  int timeStep;
  double totalReward;

  RockExploreSim(int _mode, PomdpExecCore* _policy) :
    mode(_mode),
    policy(_policy)
  {}

  void initialize(void) {
    modelG->getInitialBelief(b);
    si = modelG->chooseStochasticOutcome(b);
    policy->setToInitialBelief();
    timeStep = 0;
  }

  bool takeStep(void) {
    bool doVisualize = (mode != MODE_EVAL);
    
    // Check if the selected policy type is implemented.
    if (0 == timeStep && -1 == policy->chooseAction()) {
      return false;
    }

    if (doVisualize) {
      // Display current state and belief
      std::string rmap;
      RockExploreRockMarginals probRockIsGood;
      modelG->getMarginals(probRockIsGood, b);
      modelG->getMap(rmap, si, probRockIsGood);
      printf("Current map is:\n"
	     "\n"
	     "%s",
	     rmap.c_str());
    }
      
    int ai = policy->chooseAction();
    if (MODE_VISUALIZE == mode) {
      /* display action selected by policy and wait for user to continue */
      printf("\nPolicy selects action %s\n", RockExploreAction::getString(ai));
      waitForUser();
    }
      
    // Calculate results of action
    double reward;
    RockExploreBelief outcomes;
    modelG->getActionResult(reward, outcomes, si, ai);
    int sp = modelG->chooseStochasticOutcome(outcomes);
    totalReward += pow(modelG->params.discountFactor, timeStep) * reward;

    RockExploreObsProbs obsProbs;
    modelG->getObsProbs(obsProbs, ai, sp);
    int o = modelG->chooseStochasticOutcome(obsProbs);

    if (doVisualize) {
      printf("Observation = o%d, immediate reward = %lf\n", o, reward);
    }

    RockExploreBelief bp;
    modelG->getUpdatedBelief(bp, b, ai, o);
      
    // Advance to next state and belief
    policy->advanceToNextBelief(ai, o);
    si = sp;
    b = bp;

    return true;
  }

  bool run(void) {
    initialize();

    while (1) {
      if (!takeStep()) {
	return false;
      }

      // If state is terminal, end the run
      RockExploreState s = modelG->states[si];
      if (s.isTerminalState) {
	if (MODE_EVAL != mode) {
	  printf("\nReached terminal state, total discounted reward = %.3lf.\n",
		 totalReward);
	}
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
  PomdpExecCore* policy = getPolicy();
  RockExploreSim sim(MODE_VISUALIZE, policy);
  if (!sim.run()) {
    printf("Sorry, that policy type is not yet implemented.\n");
  }
  delete policy;
}

void doEvaluate(void)
{
  PomdpExecCore* policy = getPolicy();
  RockExploreSim sim(MODE_EVAL, policy);
  // FIX
  sim.run();
  delete policy;
}

int main(int argc, char **argv) {
  // Initialize the random number generator
  srand(time(0));
  // Seems the random number generator needs to "clear its throat" on some systems.
  rand();

  RockExploreParams p;
  p.width = 4;
  p.height = 4;
  p.initPos = RockExplorePos(0,1);
  p.rockGoodPrior = 0.7;
  p.costMove = -1;
  p.costCheck = -1;
  p.costIllegal = -100;
  p.costSampleBad = -15;
  p.rewardSampleGood = 15;
  p.rewardExit = 15;
  p.discountFactor = 0.95;

  p.numRocks = 4;
  p.rockPos.push_back(RockExplorePos(1,3));
  p.rockPos.push_back(RockExplorePos(2,1));
  p.rockPos.push_back(RockExplorePos(3,1));
  p.rockPos.push_back(RockExplorePos(0,0));

  RockExplore model(p);
  modelG = &model;

  while (1) {
    printf("\nMain menu\n"
	   "\n"
	   "  1 - Write out the RockExplore model to RockExplore.pomdp\n"
	   "  2 - Manually control the robot in the simulator\n"
	   "  3 - Visualize a policy in the simulator\n"
	   "  4 - Evaluate the quality of all policies\n"
	   "\n"
	   "Your choice (ctrl-C to quit): "
	   );
    fflush(stdout);

    int choice = getUserChoice();

    switch (choice) {
    case 1:
      model.writeCassandraModel("RockExplore.pomdp");
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
      doEvaluate();
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
