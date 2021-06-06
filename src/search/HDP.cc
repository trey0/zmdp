/********** tell emacs we use -*- c++ -*- style comments *******************
 @file    HDP.cc
 @brief   Implementation of Bonet and Geffner's HDP algorithm.

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
  This is my implementation of the HDP algorithm, based on the paper

    "Faster heuristic Search Algorithms for Planning with
       Uncertainty and Full Feedback."
    B. Bonet and H. Geffner. In Proc. of IJCAI, 2003.

  Inevitably they could not include all the details of the algorithm in
  their paper, so it is possible that my implementation differs from
  theirs in important ways.  They have not signed off on this
  implementation: use at your own risk.  (And please inform me if you
  find any errors!)

  -Trey Smith, Feb. 2006
 **********************************************************************/

#include "HDP.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <stack>

#include "MatrixUtils.h"
#include "Pomdp.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

HDP::HDP(void) {}

void HDP::getNodeHandler(MDPNode &cn) {
  HDPExtraNodeData *searchData = new HDPExtraNodeData;
  cn.searchData = searchData;
  searchData->isSolved = cn.isTerminal;
  searchData->idx = RT_IDX_PLUS_INFINITY;
  searchData->low = RT_IDX_PLUS_INFINITY;
}

void HDP::staticGetNodeHandler(MDPNode &s, void *handlerData) {
  HDP *x = reinterpret_cast<HDP *>(handlerData);
  x->getNodeHandler(s);
}

bool &HDP::getIsSolved(const MDPNode &cn) {
  return (reinterpret_cast<HDPExtraNodeData *>(cn.searchData))->isSolved;
}

int &HDP::getLow(const MDPNode &cn) {
  return (reinterpret_cast<HDPExtraNodeData *>(cn.searchData))->low;
}

int &HDP::getIdx(const MDPNode &cn) {
  return (reinterpret_cast<HDPExtraNodeData *>(cn.searchData))->idx;
}

void HDP::cacheQ(MDPNode &cn) {
  double oldUBVal = cn.ubVal;
  // bounds->update() changes both Q values and cn.ubVal
  bounds->update(cn, NULL);
  trackBackup(cn);
  // keep the changes to Q but undo the change to cn.ubVal
  cn.ubVal = oldUBVal;
}

// assumes correct Q values are already cached (using cacheQ)
double HDP::residual(MDPNode &cn) {
  int maxUBAction = bounds->getMaxUBAction(cn);
  return fabs(cn.ubVal - cn.Q[maxUBAction].ubVal);
}

void HDP::updateInternal(MDPNode &cn) {
  cacheQ(cn);
  int maxUBAction = bounds->getMaxUBAction(cn);
  cn.ubVal = cn.Q[maxUBAction].ubVal;
}

bool HDP::trialRecurse(MDPNode &cn, int depth) {
  if (zmdpDebugLevelG >= 1) {
    printf("  trialRecurse: depth=%d ubVal=%g\n", depth, cn.ubVal);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
  }

  // base case
  if (getIsSolved(cn)) {
    if (zmdpDebugLevelG >= 1) {
      printf("  trialRecurse: solved node (terminating)\n");
    }
    return false;
  }

  // check residual
  cacheQ(cn);
  int maxUBAction = bounds->getMaxUBAction(cn);
  // FIX do not recalculate maxUBAction in residual()
  if (residual(cn) > targetPrecision) {
    cn.ubVal = cn.Q[maxUBAction].ubVal;

    if (zmdpDebugLevelG >= 1) {
      printf("  trialRecurse: big residual (terminating)\n");
    }
    return true;
  }

  // mark state as active
  visited.push(&cn);
  nodeStack.push(&cn);
  getIdx(cn) = getLow(cn) = index;
  index++;

  // recursive call
  bool flag = false;
  MDPQEntry &Qa = cn.Q[maxUBAction];
  // printf("    pre low=%d idx=%d\n", getLow(cn), getIdx(cn));
  FOR(o, Qa.getNumOutcomes()) {
    MDPEdge *e = Qa.outcomes[o];
    if (NULL != e) {
      MDPNode &sn = *e->nextState;
      // printf("      a=%d o=%d sn=[%s] sn.idx=%d\n", maxUBAction, o,
      // denseRep(sn.s).c_str(), getIdx(sn));

      if (RT_IDX_PLUS_INFINITY == getIdx(sn)) {
        if (trialRecurse(sn, depth + 1)) {
          flag = true;
        }
        getLow(cn) = std::min(getLow(cn), getLow(sn));
      } else if (nodeStack.contains(&sn)) {
        getLow(cn) = std::min(getLow(cn), getIdx(sn));
      }
    }
  }
  // printf("    post low=%d idx=%d\n", getLow(cn), getIdx(cn));

  // update if necessary
  if (flag) {
    bounds->update(cn, NULL);
    trackBackup(cn);
    return true;
  } else if (getIdx(cn) == getLow(cn)) {
    // try to label
    printf("  marking %d nodes solved\n", static_cast<int>(nodeStack.size()));
    while (nodeStack.top() != &cn) {
      MDPNode &sn = *nodeStack.pop();
      getIsSolved(sn) = true;
    }
    nodeStack.pop();
    getIsSolved(cn) = true;
  }

  return flag;
}

bool HDP::doTrial(MDPNode &cn) {
  if (getIsSolved(cn)) {
    printf("-*- doTrial: root node is solved, terminating\n");
    return true;
  }

  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials + 1));
  }

  index = 0;
  trialRecurse(cn, 0);
  // reset idx to +infinity for visited states
  while (!visited.empty()) {
    getIdx(*visited.top()) = RT_IDX_PLUS_INFINITY;
    visited.pop();
  }
  nodeStack.clear();
  RT_CLEAR_STD_STACK(visited);

  numTrials++;

  return false;
}

void HDP::derivedClassInit(void) {
  bounds->addGetNodeHandler(&HDP::staticGetNodeHandler, this);
}

};  // namespace zmdp
