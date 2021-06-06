/********** tell emacs we use -*- c++ -*- style comments *******************
 @file    RTDP.cc
 @brief   Implementation of Barto, Bradke, and Singh's RTDP algorithm.

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

/**********************************************************************
  This is my implementation of the RTDP algorithm, based on the paper

    "Learning to Act Using Real-Time Dynamic Programming."
    A. Barto, S. Bradke, and S. Singh.
    Artificial Intelligence 72(1-2): 81-138. 1995.

  Inevitably they could not include all the details of the algorithm in
  their paper, so it is possible that my implementation differs from
  theirs in important ways.  They have not signed off on this
  implementation: use at your own risk.  (And please inform me if you
  find any errors!)

  -Trey Smith, Feb. 2006
 **********************************************************************/

#include "RTDP.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <queue>

#include "MatrixUtils.h"
#include "Pomdp.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

RTDP::RTDP(void) {}

void RTDP::trialRecurse(MDPNode &cn, int depth) {
  // check for termination
  if (cn.isTerminal) {
    if (zmdpDebugLevelG >= 1) {
      printf("trialRecurse: depth=%d ubVal=%g terminal node (terminating)\n",
             depth, cn.ubVal);
    }
    return;
  }

  // cached Q values must be up to date for subsequent calls
  int maxUBAction;
  bounds->update(cn, &maxUBAction);
  trackBackup(cn);

  int simulatedOutcome = bounds->getSimulatedOutcome(cn, maxUBAction);

  if (zmdpDebugLevelG >= 1) {
    printf("  trialRecurse: depth=%d a=%d o=%d ubVal=%g\n", depth, maxUBAction,
           simulatedOutcome, cn.ubVal);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
  }

  // recurse to successor
  trialRecurse(cn.getNextState(maxUBAction, simulatedOutcome), depth + 1);

  bounds->update(cn, NULL);
  trackBackup(cn);
}

bool RTDP::doTrial(MDPNode &cn) {
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials + 1));
  }

  trialRecurse(cn, 0);
  numTrials++;

  return false;
}

};  // namespace zmdp
