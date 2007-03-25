/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2007-03-25 07:34:39 $
   
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

  useEvaluationCache = config->getBool("useEvaluationCache");
  evaluationTrialsPerEpoch = config->getInt("evaluationTrialsPerEpoch");
  evaluationMaxStepsPerTrial = config->getInt("evaluationMaxStepsPerTrial");
  scoresOutputFile = config->getString("scoresOutputFile");
  simulationTraceOutputFile = config->getString("simulationTraceOutputFile");
  simulationTracesToLogPerEpoch = config->getInt("simulationTracesToLogPerEpoch");
  if (simulationTracesToLogPerEpoch < 0) {
    simulationTracesToLogPerEpoch = INT_MAX;
  }
  
  simOutFile = new ofstream(simulationTraceOutputFile.c_str());
  if (! (*simOutFile)) {
    fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	    simulationTraceOutputFile.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if (useEvaluationCache) {
    getRewardSamplesCache(weights, rewards, reachedGoal);
  } else {
    getRewardSamplesSimple(weights, rewards, reachedGoal);
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
  if (NULL != sim) {
    delete sim;
    sim = NULL;
  }
}

struct PESimLogEntry {
  CMDPNode* cn;
  int a;
  int o;
  PESimLogEntry(CMDPNode* _cn, int _a, int _o) :
    cn(_cn),
    a(_a),
    o(_o)
  {}
};

typedef std::vector<PESimLogEntry> PESimLog;

void PolicyEvaluator::getRewardSamplesCache(dvector& weights, dvector& rewards,
					    std::vector<bool>& reachedGoal)
{
  CacheMDP modelCache(simModel);
    
  if (verbose) {
    scoresOutFile = new ofstream(scoresOutputFile.c_str());
    if (!scoresOutFile) {
      fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	      scoresOutputFile.c_str(), strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
    
  // pass 1: log trials
  std::vector<PESimLog> trials(evaluationTrialsPerEpoch);
  reachedGoal.resize(evaluationTrialsPerEpoch);
  for (int i=0; i < evaluationTrialsPerEpoch; i++) {
    exec->setToInitialState();
    CMDPNode* simState = modelCache.root;
    CMDPQEntry* Qa = NULL;
    reachedGoal[i] = false;
    for (int j=0; (j < evaluationMaxStepsPerTrial) || (0 == evaluationMaxStepsPerTrial);
	 j++) {

      int a;
      if (-1 == simState->userInt) {
	a = exec->chooseAction();
	simState->userInt = a;
      } else {
	a = simState->userInt;
      }

      Qa = modelCache.getQ(*simState, a);
      int o = chooseFromDistribution(Qa->opv);
      CMDPEdge* e = Qa->outcomes[o];
      assert(NULL != e);
      if (-1 == e->userInt) {
	e->userInt = 1;
      } else {
	e->userInt++;
      }
      trials[i].push_back(PESimLogEntry(simState, a, o));
      simState = e->nextState;

      if (assumeIdenticalModels) {
	exec->currentState = simState->s;
      } else {
	exec->advanceToNextState(a, o);
      }
      if (simState->isTerminal) {
	reachedGoal[i] = true;
	break;
      }
    }

    if (verbose) {
      if (i%10 == 9) {
	printf(".");
	fflush(stdout);
      }
    }
  }

  // pass 2: collate counts and calculate reweighting coefficients
  for (int si=0; si < (int)modelCache.nodeTable.size(); si++) {
    CMDPNode* cn = modelCache.nodeTable[si];
    for (int a=0; a < modelCache.getNumActions(); a++) {
      CMDPQEntry* Qa = cn->Q[a];
      if (NULL != Qa) {
	double probSum = 0.0;
	double cntSum = 0;
	for (int o=0; o < (int)Qa->getNumOutcomes(); o++) {
	  CMDPEdge* e = Qa->outcomes[o];
	  if (NULL != e) {
	    probSum += Qa->opv(o);
	    if (-1 != e->userInt) {
	      cntSum += e->userInt;
	    }
	  }
	}
#if 0
	printf("s=%d a=%d\n", si, a);
#endif
	for (int o=0; o < (int)Qa->getNumOutcomes(); o++) {
	  CMDPEdge* e = Qa->outcomes[o];
	  if (NULL != e) {
	    if (-1 != e->userInt) {
	      e->userDouble = (Qa->opv(o) / probSum) / (e->userInt / cntSum);
#if 0
	      printf("  o=%d probSum=%lf cntSum=%lf reweight=%lf\n",
		     o, probSum, cntSum, e->userDouble);
#endif
	    }
	  }
	}
      }
    }
  }

  // pass 3: go back through logs and perform reweighting
  set_to_one(weights, evaluationTrialsPerEpoch);
  rewards.resize(evaluationTrialsPerEpoch);
  for (int i=0; i < evaluationTrialsPerEpoch; i++) {
    double rewardSoFar = 0.0;
    for (int j=trials[i].size()-1; j >= 0; j--) {
      PESimLogEntry& entry = trials[i][j];
      CMDPQEntry& Qa = *entry.cn->Q[entry.a];
      double reweight = Qa.outcomes[entry.o]->userDouble;
	
      rewardSoFar = Qa.immediateReward + reweight * modelCache.getDiscount() * rewardSoFar;
    }
    rewards(i) = rewardSoFar;

    if (verbose) {
      (*scoresOutFile) << rewardSoFar << endl;
    }
  }
    
  if (verbose) {
    printf("\n");
  }
}

void PolicyEvaluator::getRewardSamplesSimple(dvector& weights, dvector& rewards,
					     std::vector<bool>& reachedGoal)
{
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
    for (int j=0; (j < evaluationMaxStepsPerTrial) || (0 == evaluationMaxStepsPerTrial);
	 j++) {
      int a = exec->chooseAction();
      sim->performAction(a);
      if (assumeIdenticalModels) {
	exec->currentState = sim->state;
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
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2007/03/25 07:09:17  trey
 * now use CacheMDP data structures more directly for better efficiency; added stratified sampling to decrease variance
 *
 * Revision 1.3  2007/03/24 22:41:01  trey
 * simplified and added caching features
 *
 * Revision 1.2  2007/03/23 02:18:01  trey
 * added hook for alternate caching version of getRewardSamples()
 *
 * Revision 1.1  2007/03/23 01:05:06  trey
 * added PolicyEvaluator
 *
 * Revision 1.1  2007/03/23 00:00:36  trey
 *
 ***************************************************************************/
