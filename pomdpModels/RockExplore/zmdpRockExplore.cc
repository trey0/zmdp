/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-03-06 02:23:08 $

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

#include "zmdpMainConfig.cc" // embed default config file

using namespace std;
//using namespace MatrixUtils;
using namespace zmdp;

RockExplore* modelG = NULL;

enum PolicyTypes {
  P_QMDP=1,
  P_VOTING=2,
  P_MOST_LIKELY=3,
  P_TWO_STEP=4,
  P_ZMDP=5
};

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

int getUserAction(void)
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

int getPolicyType(void)
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

void doManual(void)
{
  RockExploreBelief b, bp;
  double reward;
  RockExploreBelief outcomes;
  RockExploreObsProbs obsProbs;
  RockExploreRockMarginals probRockIsGood;
  RockExploreState s;
  std::string rmap;

  modelG->getInitialBelief(b);
  int si = modelG->chooseStochasticOutcome(b);

  while (1) {
    // Display current state and belief
    modelG->getMarginals(probRockIsGood, b);
    modelG->getMap(rmap, si, probRockIsGood);
    printf("Current map is:\n"
	   "\n"
	   "%s",
	   rmap.c_str());

    int ai = getUserAction();

    // Calculate results of action
    modelG->getActionResult(reward, outcomes, si, ai);
    int sp = modelG->chooseStochasticOutcome(outcomes);
    modelG->getObsProbs(obsProbs, ai, sp);
    int o = modelG->chooseStochasticOutcome(obsProbs);
    printf("Observation = o%d, reward = %lf\n", o, reward);
    modelG->getUpdatedBelief(bp, b, ai, o);

    // Advance to next state and belief
    si = sp;
    b = bp;

    // If state is terminal, end the run
    s = modelG->states[si];
    if (s.isTerminalState) {
      printf("\nReached terminal state, all done.\n");
      break;
    }
  }
}

void doVisualize(void)
{
  int policyType = getPolicyType();
  // fill me in
}

void doEvaluate(void)
{
  int policyType = getPolicyType();
  // fill me in
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
 * Revision 1.2  2007/03/05 23:33:24  trey
 * now outputs reasonable Cassandra model
 *
 * Revision 1.1  2007/03/05 08:58:26  trey
 * initial check-in
 *
 *
 ***************************************************************************/
