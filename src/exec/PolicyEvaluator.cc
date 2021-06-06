/********** tell emacs we use -*- c++ -*- style comments *******************
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
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "CacheMDP.h"
#include "MDPSim.h"
#include "MatrixUtils.h"
#include "PolicyEvaluator.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

PolicyEvaluator::PolicyEvaluator(MDP *_simModel, MDPExecCore *_exec,
                                 const ZMDPConfig *_config,
                                 bool _assumeIdenticalModels)
    : simModel(_simModel), exec(_exec), config(_config),
      assumeIdenticalModels(_assumeIdenticalModels), sim(NULL),
      simOutFile(NULL), scoresOutFile(NULL), modelCache(NULL) {}

void PolicyEvaluator::getRewardSamples(dvector &rewards, double &successRate,
                                       bool _verbose) {
  timeval startTime = getTime();

  verbose = _verbose;

  useEvaluationCache = config->getBool("useEvaluationCache");
  evaluationTrialsPerEpoch = config->getInt("evaluationTrialsPerEpoch");
  evaluationMaxStepsPerTrial = config->getInt("evaluationMaxStepsPerTrial");
  scoresOutputFile = config->getString("scoresOutputFile");
  simulationTraceOutputFile = config->getString("simulationTraceOutputFile");
  simulationTracesToLogPerEpoch =
      config->getInt("simulationTracesToLogPerEpoch");
  if (simulationTracesToLogPerEpoch < 0) {
    simulationTracesToLogPerEpoch = INT_MAX;
  }

  simOutFile = new ofstream(simulationTraceOutputFile.c_str());
  if (!(*simOutFile)) {
    fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
            simulationTraceOutputFile.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (verbose) {
    scoresOutFile = new ofstream(scoresOutputFile.c_str());
    if (!scoresOutFile) {
      fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
              scoresOutputFile.c_str(), strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  // parameter 30 is arbitrary, trying to guarantee valid statistics
  int numBatches = std::min(evaluationTrialsPerEpoch, 30);
  int numTrialsPerBatch = evaluationTrialsPerEpoch / numBatches;

  rewards.resize(numBatches);
  double successRateSum = 0.0;
  int startTrialIndex = 0;
  for (int i = 0; i < numBatches; i++) {
    dvector batchRewards;
    double batchSuccessRate;
    doBatch(batchRewards, batchSuccessRate, numTrialsPerBatch,
            std::max(0, simulationTracesToLogPerEpoch - startTrialIndex));
    rewards(i) = sum(batchRewards) / numTrialsPerBatch;
    successRateSum += batchSuccessRate;
    startTrialIndex += numTrialsPerBatch;
  }
  printf("\n");

  printf("batchRewards: ");
  for (int i = 0; i < numBatches; i++) {
    printf("%.4lf ", rewards(i));
  }
  printf("\n");

  successRate = successRateSum / numBatches;

  // cleanup
#define DELETE_AND_NULL(x)                                                     \
  if (NULL != (x)) {                                                           \
    delete (x);                                                                \
    (x) = NULL;                                                                \
  }

  DELETE_AND_NULL(simOutFile);
  DELETE_AND_NULL(scoresOutFile);
  DELETE_AND_NULL(sim);
  DELETE_AND_NULL(modelCache);

  printf("(policy evaluation took %.3lf seconds)\n",
         timevalToSeconds(getTime() - startTime));
}

void PolicyEvaluator::doBatch(dvector &rewards, double &successRate,
                              int numTrials, int numTracesToLog) {
  if (useEvaluationCache) {
    doBatchCache(rewards, successRate, numTrials, numTracesToLog);
  } else {
    doBatchSimple(rewards, successRate, numTrials, numTracesToLog);
  }
}

struct PESimLogEntry {
  CMDPNode *cn;
  int a;
  int o;
  PESimLogEntry(CMDPNode *_cn, int _a, int _o) : cn(_cn), a(_a), o(_o) {}
};

typedef std::vector<PESimLogEntry> PESimLog;

void PolicyEvaluator::doBatchCache(dvector &rewards, double &successRate,
                                   int numTrials, int numTracesToLog) {
  if (NULL == modelCache) {
    modelCache = new CacheMDP(simModel);
  }

  ofstream *simOutFileTmp = simOutFile;

  // pass 0: clear old count data if any
  for (int si = 0; si < (int)modelCache->nodeTable.size(); si++) {
    CMDPNode *cn = modelCache->nodeTable[si];
    for (int a = 0; a < modelCache->getNumActions(); a++) {
      CMDPQEntry *Qa = cn->Q[a];
      if (NULL != Qa) {
        for (int o = 0; o < (int)Qa->getNumOutcomes(); o++) {
          CMDPEdge *e = Qa->outcomes[o];
          if (NULL != e) {
            e->userInt = -1;
          }
        }
      }
    }
  }

  // pass 1: record trials
  std::vector<PESimLog> trials(numTrials);
  int numTrialsReachedGoal = 0;
  for (int i = 0; i < numTrials; i++) {
    if (i >= numTracesToLog) {
      simOutFileTmp = NULL; // stop logging
    }

    if (simOutFileTmp) {
      (*simOutFileTmp) << ">>> begin" << endl;
    }
    exec->setToInitialState();
    CMDPNode *simState = modelCache->root;
    CMDPQEntry *Qa = NULL;
    for (int j = 0;
         (j < evaluationMaxStepsPerTrial) || (0 == evaluationMaxStepsPerTrial);
         j++) {

      int a;
      if (-1 == simState->userInt) {
        a = exec->chooseAction();
        simState->userInt = a;
      } else {
        a = simState->userInt;
      }

      Qa = modelCache->getQ(*simState, a);
      int o = chooseFromDistribution(Qa->opv);
      CMDPEdge *e = Qa->outcomes[o];
      assert(NULL != e);
      if (-1 == e->userInt) {
        e->userInt = 1;
      } else {
        e->userInt++;
      }
      trials[i].push_back(PESimLogEntry(simState, a, o));

      if (simOutFileTmp) {
        (*simOutFileTmp) << "sim: [" << sparseRep(simState->s) << "] " << a
                         << " [" << sparseRep(e->nextState->s) << "] " << o
                         << endl;
      }

      simState = e->nextState;

      if (assumeIdenticalModels) {
        ((MDPExec *)exec)->currentState = simState->s;
      } else {
        exec->advanceToNextState(a, o);
      }
      if (simState->isTerminal) {
        numTrialsReachedGoal++;
        if (simOutFileTmp) {
          (*simOutFileTmp) << "terminated" << endl;
        }
        break;
      }
    }

    if (verbose) {
      if (i % 10 == 9) {
        printf(".");
        fflush(stdout);
      }
    }
  }
  if (verbose) {
    printf("#");
    fflush(stdout);
  }

  // pass 2: collate counts and calculate reweighting coefficients
  for (int si = 0; si < (int)modelCache->nodeTable.size(); si++) {
    CMDPNode *cn = modelCache->nodeTable[si];
    for (int a = 0; a < modelCache->getNumActions(); a++) {
      CMDPQEntry *Qa = cn->Q[a];
      if (NULL != Qa) {
        double probSum = 0.0;
        double cntSum = 0;
        for (int o = 0; o < (int)Qa->getNumOutcomes(); o++) {
          CMDPEdge *e = Qa->outcomes[o];
          if (NULL != e) {
            if (-1 != e->userInt) {
              probSum += Qa->opv(o);
              cntSum += e->userInt;
            }
          }
        }
#if 0
	printf("s=%d a=%d probSum=%lf cntSum=%lf\n", si, a, probSum, cntSum);
#endif
        for (int o = 0; o < (int)Qa->getNumOutcomes(); o++) {
          CMDPEdge *e = Qa->outcomes[o];
          if (NULL != e) {
            if (-1 != e->userInt) {
              if (probSum == 1.0) {
                e->userDouble = Qa->opv(o) / (e->userInt / cntSum);
              } else {
                e->userDouble = 1.0;
              }
#if 0
	      printf("  o=%d n=%d opv(o)=%lf reweight=%lf\n",
		     o, e->userInt, Qa->opv(o), e->userDouble);
#endif
            }
          }
        }
      }
    }
  }

  // pass 3: go back through logs and perform reweighting
  rewards.resize(numTrials);
  double batchSumReward = 0.0;
  for (int i = 0; i < numTrials; i++) {
    double rewardSoFar = 0.0;
    for (int j = trials[i].size() - 1; j >= 0; j--) {
      PESimLogEntry &entry = trials[i][j];
      CMDPQEntry &Qa = *entry.cn->Q[entry.a];
      double reweight = Qa.outcomes[entry.o]->userDouble;

      rewardSoFar = Qa.immediateReward +
                    reweight * modelCache->getDiscount() * rewardSoFar;
    }
    rewards(i) = rewardSoFar;

    batchSumReward += rewardSoFar;
  }

  if (verbose) {
    (*scoresOutFile) << batchSumReward / numTrials << endl;
  }

  successRate = ((double)numTrialsReachedGoal) / numTrials;
}

void PolicyEvaluator::doBatchSimple(dvector &rewards, double &successRate,
                                    int numTrials, int numTracesToLog) {
  sim = new MDPSim(simModel);

  sim->simOutFile = simOutFile;

  int numTrialsReachedGoal = 0;

  // do evaluation
  rewards.resize(numTrials);
  for (int i = 0; i < numTrials; i++) {
    if (i >= numTracesToLog) {
      sim->simOutFile = NULL; // stop logging
    }

    sim->restart();
    exec->setToInitialState();
    for (int j = 0;
         (j < evaluationMaxStepsPerTrial) || (0 == evaluationMaxStepsPerTrial);
         j++) {
      int a = exec->chooseAction();
      sim->performAction(a);
      if (assumeIdenticalModels) {
        ((MDPExec *)exec)->currentState = sim->state;
      } else {
        exec->advanceToNextState(a, sim->lastOutcomeIndex);
      }
      if (sim->terminated) {
        numTrialsReachedGoal++;
        break;
      }
    }
    rewards(i) = sim->rewardSoFar;
    if (verbose) {
      (*scoresOutFile) << sim->rewardSoFar << endl;
      if (i % 10 == 9) {
        printf(".");
        fflush(stdout);
      }
    }
  }
  if (verbose) {
    printf("#");
    fflush(stdout);
  }

  successRate = ((double)numTrialsReachedGoal) / numTrials;
}

}; // namespace zmdp
