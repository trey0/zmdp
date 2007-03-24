/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-03-24 22:41:01 $
   
 @file    PolicyEvaluator.h
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

#ifndef INCPolicyEvaluator_h
#define INCPolicyEvaluator_h

#include "MDPExec.h"
#include "MDPSim.h"
#include "MDPCache.h"

namespace zmdp {

struct PolicyEvaluator {
  PolicyEvaluator(MDP* _simModel,
		  MDPExec* _exec,
		  const ZMDPConfig* _config,
		  bool _assumeIdenticalModels);
  void getRewardSamples(dvector& weights,
			dvector& rewards,
			std::vector<bool>& reachedGoal,
			bool _verbose);

protected:
  MDP* simModel;
  MDP* planningModel;
  MDPExec* exec;
  const ZMDPConfig* config;
  bool assumeIdenticalModels;
  bool useEvaluationCache;
  int evaluationTrialsPerEpoch;
  int evaluationMaxStepsPerTrial;
  std::string scoresOutputFile;
  std::string simulationTraceOutputFile;
  int simulationTracesToLogPerEpoch;
  MDPSim* sim;
  std::ofstream* simOutFile;
  std::ofstream* scoresOutFile;
  bool verbose;
  EXT_NAMESPACE::hash_map<std::string, int> policyCache;
  int cacheHits, cacheQueries;

  int chooseActionCache(void);
  void getRewardSamplesInternal(dvector& weights,
				dvector& rewards,
				std::vector<bool>& reachedGoal);
};

}; // namespace zmdp

#endif // INCPolicyEvaluator_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/03/23 02:18:01  trey
 * added hook for alternate caching version of getRewardSamples()
 *
 * Revision 1.1  2007/03/23 01:05:06  trey
 * added PolicyEvaluator
 *
 *
 ***************************************************************************/
