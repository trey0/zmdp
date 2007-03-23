/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-03-23 01:05:06 $
   
 @file    PolicyEvaluator.cc
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

//#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "PolicyEvaluator.h"
#include "MDPSim.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

PolicyEvaluator::PolicyEvaluator(MDP* _simModel,
				 MDPExec* _exec,
				 const ZMDPConfig* _config) :
  simModel(_simModel),
  exec(_exec),
  config(_config)
{}

void PolicyEvaluator::getRewardSamples(dvector& weights,
				       dvector& rewards,
				       bool verbose)
{
  int evaluationTrialsPerEpoch = config->getInt("evaluationTrialsPerEpoch");
  int evaluationMaxStepsPerTrial = config->getInt("evaluationMaxStepsPerTrial");
  std::string scoresOutputFile = config->getString("scoresOutputFile");
  std::string simulationTraceOutputFile = config->getString("simulationTraceOutputFile");
  int simulationTracesToLogPerEpoch = config->getInt("simulationTracesToLogPerEpoch");
  if (simulationTracesToLogPerEpoch < 0) {
    simulationTracesToLogPerEpoch = INT_MAX;
  }
  
  MDPSim* sim = new MDPSim(simModel);

  ofstream simOutFile(simulationTraceOutputFile.c_str());
  if (!simOutFile) {
    fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	    simulationTraceOutputFile.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  sim->simOutFile = &simOutFile;

  ofstream scoresOut;
  if (verbose) {
    scoresOut.open(scoresOutputFile.c_str());
    if (!scoresOut) {
      fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	      scoresOutputFile.c_str(), strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  set_to_one(weights, evaluationTrialsPerEpoch);

  // do evaluation
  rewards.resize(evaluationTrialsPerEpoch);
  for (int i=0; i < evaluationTrialsPerEpoch; i++) {
    if (i >= simulationTracesToLogPerEpoch) {
      sim->simOutFile = NULL; // stop logging
    }

    sim->restart();
    exec->setToInitialState();
    for (int j=0; (j < evaluationMaxStepsPerTrial) || (0 == evaluationMaxStepsPerTrial); j++) {
      int a = exec->chooseAction();
      sim->performAction(a);
      exec->advanceToNextState(a, sim->lastOutcomeIndex);
      if (sim->terminated) break;
    }
    rewards(i) = sim->rewardSoFar;
    if (verbose) {
      scoresOut << sim->rewardSoFar << endl;
      if (i%10 == 9) {
	printf(".");
	fflush(stdout);
      }
    }
  }
  if (verbose) {
    printf("\n");
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/03/23 00:00:36  trey
 *
 ***************************************************************************/
