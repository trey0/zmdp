/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-03-23 02:18:01 $
   
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

namespace zmdp {

struct PolicyEvaluator {
  MDP* simModel;
  MDP* planningModel;
  MDPExec* exec;
  const ZMDPConfig* config;

  PolicyEvaluator(MDP* _simModel,
		  MDPExec* _exec,
		  const ZMDPConfig* _config);
  void getRewardSamples(dvector& weights,
			dvector& rewards,
			std::vector<bool>& reachedGoal,
			bool verbose);

protected:
  void getRewardSamplesSimple(dvector& weights,
			      dvector& rewards,
			      std::vector<bool>& reachedGoal,
			      bool verbose);
  void getRewardSamplesCache(dvector& weights,
			     dvector& rewards,
			     std::vector<bool>& reachedGoal,
			     bool verbose);
};

}; // namespace zmdp

#endif // INCPolicyEvaluator_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/03/23 01:05:06  trey
 * added PolicyEvaluator
 *
 *
 ***************************************************************************/
