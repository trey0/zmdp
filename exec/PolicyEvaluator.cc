/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-03-24 22:41:01 $
   
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
#include "CacheMDP.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

PolicyEvaluator::PolicyEvaluator(MDP* _simModel,
				 MDPExec* _exec,
				 const ZMDPConfig* _config,
				 bool _assumeIdenticalModels) :
  simModel(_simModel),
  exec(_exec),
  config(_config),
  assumeIdenticalModels(_assumeIdenticalModels),
  sim(NULL),
  simOutFile(NULL),
  scoresOutFile(NULL)
{}

void PolicyEvaluator::getRewardSamples(dvector& weights,
				       dvector& rewards,
				       std::vector<bool>& reachedGoal,
				       bool _verbose)
{
  verbose = _verbose;
  getRewardSamplesInternal(weights, rewards, reachedGoal);
}

int PolicyEvaluator::chooseActionCache(void)
{
  std::string hs = hashable(exec->currentState);
  typeof(policyCache.begin()) keyValuePair = policyCache.find(hs);
  cacheQueries++;
  if (policyCache.end() == keyValuePair) {
    int a = exec->chooseAction();
    policyCache[hs] = a;
    return a;
  } else {
    cacheHits++;
    return keyValuePair->second;
  }
}

void PolicyEvaluator::getRewardSamplesInternal(dvector& weights,
					       dvector& rewards,
					       std::vector<bool>& reachedGoal)
{
  useEvaluationCache = config->getBool("useEvaluationCache");
  evaluationTrialsPerEpoch = config->getInt("evaluationTrialsPerEpoch");
  evaluationMaxStepsPerTrial = config->getInt("evaluationMaxStepsPerTrial");
  scoresOutputFile = config->getString("scoresOutputFile");
  simulationTraceOutputFile = config->getString("simulationTraceOutputFile");
  simulationTracesToLogPerEpoch = config->getInt("simulationTracesToLogPerEpoch");
  if (simulationTracesToLogPerEpoch < 0) {
    simulationTracesToLogPerEpoch = INT_MAX;
  }
  
  if (useEvaluationCache) {
    simModel = new CacheMDP(simModel);
  }
  sim = new MDPSim(simModel);

  simOutFile = new ofstream(simulationTraceOutputFile.c_str());
  if (! (*simOutFile)) {
    fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	    simulationTraceOutputFile.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  sim->simOutFile = simOutFile;

  if (verbose) {
    scoresOutFile = new ofstream(scoresOutputFile.c_str());
    if (!scoresOutFile) {
      fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	      scoresOutputFile.c_str(), strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if (useEvaluationCache) {
    cacheHits = 0;
    cacheQueries = 0;
  }

  set_to_one(weights, evaluationTrialsPerEpoch);
  reachedGoal.resize(evaluationTrialsPerEpoch);

  // do evaluation
  rewards.resize(evaluationTrialsPerEpoch);
  for (int i=0; i < evaluationTrialsPerEpoch; i++) {
    if (i >= simulationTracesToLogPerEpoch) {
      sim->simOutFile = NULL; // stop logging
    }

    sim->restart();
    exec->setToInitialState();
    reachedGoal[i] = false;
    for (int j=0; (j < evaluationMaxStepsPerTrial) || (0 == evaluationMaxStepsPerTrial); j++) {
      int a;
      if (useEvaluationCache) {
	a = chooseActionCache();
      } else {
	a = exec->chooseAction();
      }
      sim->performAction(a);
      if (assumeIdenticalModels) {
	if (useEvaluationCache) {
	  ((CacheMDP*) simModel)->translateState(exec->currentState, sim->state);
	} else {
	  exec->currentState = sim->state;
	}
      } else {
	exec->advanceToNextState(a, sim->lastOutcomeIndex);
      }
      if (sim->terminated) {
	reachedGoal[i] = true;
	break;
      }
    }
    rewards(i) = sim->rewardSoFar;
    if (verbose) {
      (*scoresOutFile) << sim->rewardSoFar << endl;
      if (i%10 == 9) {
	printf(".");
	fflush(stdout);
      }
    }
  }
  if (verbose) {
    printf("\n");
  }

  if (useEvaluationCache) {
#if 0
    printf("cache hits %d queries %d rate %3.1lf%%\n",
	   cacheHits, cacheQueries, 100 * ((double) cacheHits) / cacheQueries);
#endif
    
    // reset policy cache before next evaluation epoch
    policyCache.clear();
  }

  // cleanup
  if (NULL != simOutFile) {
    delete simOutFile;
    simOutFile = NULL;
  }
  if (NULL != scoresOutFile) {
    delete scoresOutFile;
    scoresOutFile = NULL;
  }
  delete sim;
  sim = NULL;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/03/23 02:18:01  trey
 * added hook for alternate caching version of getRewardSamples()
 *
 * Revision 1.1  2007/03/23 01:05:06  trey
 * added PolicyEvaluator
 *
 * Revision 1.1  2007/03/23 00:00:36  trey
 *
 ***************************************************************************/
