/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-01-28 03:01:05 $
   
 @file    MDPSim.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

namespace pomdp {

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

  // figure out actual next state and observation
#define CHOOSE_EPS (1e-10)

  // draw outcome o
  outcome_prob_vector opv;
  model->getOutcomeProbVector(opv, state, a);
  int o = chooseFromDistribution(opv);

  state_vector sp;
  model->getNextState(sp, state, a, o);

  if (simOutFile) {
    (*simOutFile) << "sim: [" << sparseRep(state) << "] " << a << " ["
		  << sparseRep(sp) << "] " << o << endl;
  }

  // change state and elapsed time
  state = sp;
  elapsedTime++;

  // we used to check termination *before* the transition to take into account
  // weirdness in the hallway and hallway2 problems; but that's resolved now.
  if (model->getIsTerminalState(state)) {
    terminated = true;
    if (simOutFile) {
      (*simOutFile) << "terminated" << endl;
    }
  }
}

}; // namespace pomdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
