/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2007-04-22 22:41:19 $
   
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
#include "CacheMDP.h"

namespace zmdp {

struct PolicyEvaluator {
  PolicyEvaluator(MDP* _simModel,
		  MDPExecCore* _exec,
		  const ZMDPConfig* _config,
		  bool _assumeIdenticalModels);
  void getRewardSamples(dvector& rewards, double& successRate, bool _verbose);

protected:
  MDP* simModel;
  MDP* planningModel;
  MDPExecCore* exec;
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
  CacheMDP* modelCache;

  void doBatch(dvector& rewards, double& successRate, int numTrials,
	       int numTracesToLog);
  void doBatchCache(dvector& rewards, double& successRate, int numTrials,
		    int numTracesToLog);
  void doBatchSimple(dvector& rewards, double& successRate, int numTrials,
		     int numTracesToLog);
};

}; // namespace zmdp

#endif // INCPolicyEvaluator_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2007/03/25 21:38:18  trey
 * fixed policy evaluation to avoid discarding the cache between batches
 *
 * Revision 1.6  2007/03/25 17:38:25  trey
 * reworked how statistics are collected so that the benefits of reweighting show up in the confidence interval calculation
 *
 * Revision 1.5  2007/03/25 15:15:42  trey
 * removed weights output from getRewardSamples(); added back in logging of simulation trace
 *
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
 *
 ***************************************************************************/
