/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-04-08 22:46:21 $
   
 @file    MDPModel.h
 @brief   No brief

 Copyright (c) 2005-2006, Trey Smith. All rights reserved.

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

#ifndef INCMDPModel_h
#define INCMDPModel_h

#include <iostream>
#include <string>
#include <vector>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#include "zmdpConfig.h"

using namespace sla;

namespace zmdp {

struct AbstractBound;

// Represents an MDP where state is continuous, time is discrete,
// actions are discrete, and the possible outcomes of an action form a
// discrete probability distribution.  This data structure can
// represent the belief MDP corresponding to a discrete POMDP.
struct MDP {
  std::string fileName; // filename model was read from (or empty string if unknown)
  int numStateDimensions, numActions;
  double discount;

  virtual ~MDP(void) {}

  int getNumStateDimensions(void) const { return numStateDimensions; }
  int getNumActions(void) const { return numActions; }
  double getDiscount(void) const { return discount; }

  // returns the initial state
  virtual const state_vector& getInitialState(void) = 0;

  // returns true if state is terminal
  virtual bool getIsTerminalState(const state_vector& s) = 0;

  // sets result to be the vector of outcome probabilities when from
  // state s action a is selected
  virtual outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,
						    const state_vector& s, int a) = 0;

  // sets result to be the next state when from state s action a is
  // selected and outcome o results
  virtual state_vector& getNextState(state_vector& result, const state_vector& s, int a,
				     int o) = 0;

  // returns the expected immediate reward when from state s action a is selected
  virtual double getReward(const state_vector& s, int a) = 0;

  // returns a new lower bound or upper bound that is valid for
  // this MDP.  notes:
  // * the resulting bound must be initialized before it is used, and
  //   initialization may take significant computation time.
  // * some types of MDP might not define one of these bounds, which
  //   could be signaled by returning NULL.  so far this hasn't been
  //   explored.
  virtual AbstractBound* newLowerBound(const ZMDPConfig* _config) = 0;
  virtual AbstractBound* newUpperBound(const ZMDPConfig* _config) = 0;
};

}; // namespace zmdp

#endif // INCMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/03/24 22:37:42  trey
 * removed const tag from member functions in MDP so that implementations which cache query results are possible
 *
 * Revision 1.1  2006/11/08 16:33:45  trey
 * renamed MDP.h to MDPModel.h to avoid confusion with mdp.h, stupid case insensitive Mac OS X
 *
 * Revision 1.9  2006/10/24 02:08:40  trey
 * changed some functions from pass-by-ref to pass-by-pointer to better match the rest of the system
 *
 * Revision 1.8  2006/10/18 18:05:20  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.7  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.6  2006/04/05 21:36:16  trey
 * moved OBS_IS_ZERO_EPS from MDP.h to zmdpCommonDefs.h
 *
 * Revision 1.5  2006/04/04 17:26:29  trey
 * moved OBS_IS_ZERO_EPS to MDP.h because it is used all over
 *
 * Revision 1.4  2006/02/06 19:29:23  trey
 * removed numOutcomes field; some MDPs have a varying number of outcomes depending on state
 *
 * Revision 1.3  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:09:11  trey
 * added generic mechanism for getting bounds
 *
 * Revision 1.1  2006/01/28 03:01:05  trey
 * initial check-in
 *
 *
 ***************************************************************************/

