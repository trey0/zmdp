/********** tell emacs we use -*- c++ -*- style comments *******************
 @file    BoundPairCore.cc
 @brief   Common code used by multiple RTDP variants found in this
          directory.

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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <queue>

#include "BoundPairCore.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

void BoundPairCore::addGetNodeHandler(GetNodeHandler getNodeHandler,
                                      void *handlerData) {
  getNodeHandlers.push_back(GetNodeHandlerStruct(getNodeHandler, handlerData));
}

// relies on correct cached Q values!
int BoundPairCore::getMaxUBAction(MDPNode &cn) {
  double bestVal = -99e+20;
  int bestAction = -1;
  FOR(a, cn.getNumActions()) {
    const MDPQEntry &Qa = cn.Q[a];
    if (Qa.ubVal > bestVal) {
      bestVal = Qa.ubVal;
      bestAction = a;
    }
  }
  return bestAction;
}

int BoundPairCore::getSimulatedOutcome(MDPNode &cn, int a) {
  double r = unit_rand();
  int result = 0;
  MDPQEntry &Qa = cn.Q[a];
  FOR(o, Qa.getNumOutcomes()) {
    MDPEdge *e = Qa.outcomes[o];
    if (NULL != e) {
      r -= e->obsProb;
      if (r <= 0) {
        result = o;
        break;
      }
    }
  }

  return result;
}

}; // namespace zmdp
