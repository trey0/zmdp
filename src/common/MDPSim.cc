/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-03-22 18:21:08 $
   
 @file    MDPSim.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <iostream>
#include <sstream>

#include "MDPSim.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

MDPSim::MDPSim(MDP* _model) :
  model(_model)
{
  simOutFile = NULL;
  restart();
}

// sets us back to the initial belief (choosing state randomly according to
// initial belief) and zeros elapsed time and reward
void MDPSim::restart(void) {
  elapsedTime = 0;
  rewardSoFar = 0;
  lastOutcomeIndex = -1;
  terminated = false;
  state.resize(model->getInitialState().size());
  state = model->getInitialState();
  if (simOutFile) {
    (*simOutFile) << ">>> begin" << endl;
  }
}

void MDPSim::performAction(int a) {
  if (terminated) {
    cerr << "ERROR: trying to perform action when simulation is terminated"
	 << " (elapsedTime=" << elapsedTime << ")" << endl;
    exit(EXIT_FAILURE);
  }

  // increment reward
  double imm_reward = model->getReward(state,a);
  rewardSoFar += pow(model->discount, elapsedTime) * imm_reward;

  // draw outcome index o and corresponding successor state sp
  outcome_prob_vector opv;
  state_vector sp;
  model->getOutcomeProbVector(opv, state, a);
  int o = chooseFromDistribution(opv);
  model->getNextState(sp, state, a, o);

  // log transition information
  if (simOutFile) {
    (*simOutFile) << "sim: [" << sparseRep(state) << "] " << a << " ["
		  << sparseRep(sp) << "] " << o << endl;
  }

  // bring sim variables up to date
  state = sp;
  elapsedTime++;
  lastOutcomeIndex = o;

  // check for termination
  if (model->getIsTerminalState(state)) {
    terminated = true;
    if (simOutFile) {
      (*simOutFile) << "terminated" << endl;
    }
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.2  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2006/01/28 03:01:05  trey
 * initial check-in
 *
 *
 ***************************************************************************/
