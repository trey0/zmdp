/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-03-24 22:44:33 $
   
 @file    GenericDiscreteMDP.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith.

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

#ifndef INCGenericDiscreteMDP_h
#define INCGenericDiscreteMDP_h

#include <iostream>
#include <string>
#include <vector>

#include "zmdpConfig.h"
#include "MDPModel.h"
#include "AbstractBound.h"

using namespace sla;

namespace zmdp {

struct GenericDiscreteMDP : public CassandraModel {
  bool boundsInitialized;
  double globalLowerBound;
  double globalUpperBound;

  GenericDiscreteMDP(const std::string& fileName,
		     const ZMDPConfig* _config);

  // returns the expected immediate reward when from belief b action a is selected
  double getReward(const belief_vector& b, int a);

  // POMDP-as-belief-MDP implementations for virtual functions declared in MDP
  const state_vector& getInitialState(void);
  bool getIsTerminalState(const state_vector& s);
  outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,
					    const state_vector& b,
					    int a);
  state_vector& getNextState(state_vector& result,
			     const state_vector& s,
			     int a, int o);

  double getLongTermFactor(void);

  AbstractBound* newLowerBound(const ZMDPConfig* _config);
  AbstractBound* newUpperBound(const ZMDPConfig* _config);
};

}; // namespace zmdp

#endif // INCGenericDiscreteMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/11/09 20:46:28  trey
 * first working version
 *
 * Revision 1.1  2006/11/08 16:47:04  trey
 * initial check-in
 *
 *
 ***************************************************************************/

