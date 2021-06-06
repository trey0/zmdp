/********** tell emacs we use -*- c++ -*- style comments *******************
 @file    LRTDP.cc
 @brief   Implementation of Bonet and Geffner's LRTDP algorithm.

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
  This is my implementation of the LRTDP algorithm, based on the paper

  "Labeled RTDP: Improving the Convergence of Real Time Dynamic Programming."
    B. Bonet and H. Geffner. In Proc. of ICAPS, 2003.

  Inevitably they could not include all the details of the algorithm in
  their paper, so it is possible that my implementation differs from
  theirs in important ways.  They have not signed off on this
  implementation: use at your own risk.  (And please inform me if you
  find any errors!)

  -Trey Smith, Feb. 2006
 **********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <stack>

#include "LRTDP.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

LRTDP::LRTDP(void) {}

void LRTDP::getNodeHandler(MDPNode &cn) {
  LRTDPExtraNodeData *searchData = new LRTDPExtraNodeData;
  cn.searchData = searchData;
  searchData->isSolved = cn.isTerminal;
}

void LRTDP::staticGetNodeHandler(MDPNode &s, void *handlerData) {
  LRTDP *x = (LRTDP *)handlerData;
  x->getNodeHandler(s);
}

bool &LRTDP::getIsSolved(const MDPNode &cn) {
  return ((LRTDPExtraNodeData *)cn.searchData)->isSolved;
}

void LRTDP::cacheQ(MDPNode &cn) {
  double oldUBVal = cn.ubVal;
  // bounds->update() changes both Q values and cn.ubVal
  bounds->update(cn, NULL);
  trackBackup(cn);
  // keep the changes to Q but undo the change to cn.ubVal
  cn.ubVal = oldUBVal;
}

// assumes correct Q values are already cached (using cacheQ)
double LRTDP::residual(MDPNode &cn) {
  int maxUBAction = bounds->getMaxUBAction(cn);
  return fabs(cn.ubVal - cn.Q[maxUBAction].ubVal);
}

bool LRTDP::checkSolved(MDPNode &cn) {
  bool rv = true;
  NodeStack open, closed;
  int a;

  if (!getIsSolved(cn))
    open.push(&cn);
  while (!open.empty()) {
    MDPNode &n = *open.pop();
    closed.push(&n);

    cacheQ(n);
    if (residual(n) > targetPrecision) {
      rv = false;
      continue;
    }
    a = bounds->getMaxUBAction(n);
    MDPQEntry &Qa = n.Q[a];
    FOR(o, Qa.getNumOutcomes()) {
      MDPEdge *e = Qa.outcomes[o];
      if (NULL != e) {
        MDPNode &sn = *e->nextState;
        if (!getIsSolved(sn) && !open.contains(&sn) && !closed.contains(&sn)) {
          open.push(&sn);
        }
      }
    }
  }

  int numClosedStates = -1;
  if (zmdpDebugLevelG >= 1) {
    numClosedStates = closed.size();
  }
  if (rv) {
    // label relevant states
    while (!closed.empty()) {
      MDPNode &n = *closed.pop();
      getIsSolved(n) = true;
    }
  } else {
    // update states with residuals and ancestors
    while (!closed.empty()) {
      MDPNode &n = *closed.pop();
      updateInternal(n);
    }
  }

  if (zmdpDebugLevelG >= 1) {
    printf("  checkSolved: s=[%s] numClosedStates=%d rv=%d\n",
           sparseRep(cn.s).c_str(), numClosedStates, rv);
  }
  return rv;
}

void LRTDP::updateInternal(MDPNode &cn) {
  cacheQ(cn);
  int maxUBAction = bounds->getMaxUBAction(cn);
  cn.ubVal = cn.Q[maxUBAction].ubVal;
}

bool LRTDP::trialRecurse(MDPNode &cn, int depth) {
  // check for termination
  if (getIsSolved(cn)) {
    if (zmdpDebugLevelG >= 1) {
      printf("  trialRecurse: depth=%d ubVal=%g solved node (terminating)\n",
             depth, cn.ubVal);
    }
    return true;
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
  bool solvedAtNextDepth =
      trialRecurse(cn.getNextState(maxUBAction, simulatedOutcome), depth + 1);

  if (solvedAtNextDepth) {
    return checkSolved(cn);
  } else {
    return false;
  }
}

bool LRTDP::doTrial(MDPNode &cn) {
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials + 1));
  }

  trialRecurse(cn, 0);
  numTrials++;

  return getIsSolved(cn);
}

void LRTDP::derivedClassInit(void) {
  bounds->addGetNodeHandler(&LRTDP::staticGetNodeHandler, this);
}

}; // namespace zmdp
