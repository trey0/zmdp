/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-03-06 04:32:47 $

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
  PomdpExecCore* exec;
  RockExploreBelief b;
  int si;

  RockExploreSim(int _mode, PomdpExecCore* _exec) :
    mode(_mode),
    exec(_exec)
  {}

  void initialize(void) {
    modelG->getInitialBelief(b);
    si = modelG->chooseStochasticOutcome(b);
  }

  void takeStep(void) {
    bool doVisualize = (mode != MODE_EVAL);
    
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
      
    int ai = exec->chooseAction();
    if (MODE_VISUALIZE == mode) {
      /* display action selected by policy and wait for user to continue */
      printf("Policy selects action %s\n", RockExploreAction::getString(ai));
      waitForUser();
    }
      
    // Calculate results of action
    double reward;
    RockExploreBelief outcomes;
    modelG->getActionResult(reward, outcomes, si, ai);
    int sp = modelG->chooseStochasticOutcome(outcomes);

    RockExploreObsProbs obsProbs;
    modelG->getObsProbs(obsProbs, ai, sp);
    int o = modelG->chooseStochasticOutcome(obsProbs);

    if (doVisualize) {
      printf("Observation = o%d, reward = %lf\n", o, reward);
    }

    RockExploreBelief bp;
    modelG->getUpdatedBelief(bp, b, ai, o);
      
    // Advance to next state and belief
    si = sp;
    b = bp;
  }

  void run(void) {
    initialize();

    while (1) {
      takeStep();

      // If state is terminal, end the run
      RockExploreState s = modelG->states[si];
      if (s.isTerminalState) {
	printf("\nReached terminal state, all done.\n");
	break;
      }
    }
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
  sim.run();
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
  srand(time(NULL));

  RockExploreParams p;
  p.width = 4;
  p.height = 4;
  p.initPos = RockExplorePos(0,2);
  p.rockGoodPrior = 0.7;
  p.costMove = -1;
  p.costCheck = -1;
  p.costIllegal = -100;
  p.costSampleBad = -10;
  p.rewardSampleGood = 10;
  p.rewardExit = 10;
  p.discountFactor = 0.95;

  p.numRocks = 4;
  p.rockPos.push_back(RockExplorePos(1,3));
  p.rockPos.push_back(RockExplorePos(2,1));
  p.rockPos.push_back(RockExplorePos(3,1));
  p.rockPos.push_back(RockExplorePos(1,0));

  RockExplore model(p);
  modelG = &model;

  while (1) {
    printf("\nMain menu\n"
	   "\n"
	   "  1 - Output the RockExplore model in Cassandra format to RockExplore.pomdp\n"
	   "  2 - Manually control the robot in the simulator\n"
	   "  3 - Visualize a policy in the simulator\n"
	   "  4 - Evaluate the quality of a policy\n"
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
