/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2002-2006, Trey Smith. All rights reserved.

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

// #include <assert.h>
#include "MDPExec.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "MatrixUtils.h"
#include "zmdpCommonDefs.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

MDPExec::MDPExec(void) : mdp(NULL), currentStateInitialized(false) {}

bool MDPExec::getStateIsTerminal(void) const {
  return mdp->getIsTerminalState(currentState);
}

state_vector &MDPExec::getState(state_vector &s) const {
  s = currentState;
  return s;
}

int MDPExec::getRandomOutcome(int a) const {
  obs_prob_vector opv;
  return chooseFromDistribution(
      mdp->getOutcomeProbVector(opv, currentState, a));
}

};  // namespace zmdp
