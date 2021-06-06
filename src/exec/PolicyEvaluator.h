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

#ifndef ZMDP_SRC_EXEC_POLICYEVALUATOR_H_
#define ZMDP_SRC_EXEC_POLICYEVALUATOR_H_

#include <string>

#include "CacheMDP.h"
#include "MDPExec.h"
#include "MDPSim.h"

namespace zmdp {

struct PolicyEvaluator {
  PolicyEvaluator(MDP *_simModel, MDPExecCore *_exec, const ZMDPConfig *_config,
                  bool _assumeIdenticalModels);
  void getRewardSamples(dvector &rewards, double &successRate, bool _verbose);

 protected:
  MDP *simModel;
  MDP *planningModel;
  MDPExecCore *exec;
  const ZMDPConfig *config;
  bool assumeIdenticalModels;
  bool useEvaluationCache;
  int evaluationTrialsPerEpoch;
  int evaluationMaxStepsPerTrial;
  std::string scoresOutputFile;
  std::string simulationTraceOutputFile;
  int simulationTracesToLogPerEpoch;
  MDPSim *sim;
  std::ofstream *simOutFile;
  std::ofstream *scoresOutFile;
  bool verbose;
  CacheMDP *modelCache;

  void doBatch(dvector &rewards, double &successRate, int numTrials,
               int numTracesToLog);
  void doBatchCache(dvector &rewards, double &successRate, int numTrials,
                    int numTracesToLog);
  void doBatchSimple(dvector &rewards, double &successRate, int numTrials,
                     int numTracesToLog);
};

};  // namespace zmdp

#endif  // ZMDP_SRC_EXEC_POLICYEVALUATOR_H_
