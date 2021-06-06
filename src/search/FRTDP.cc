/********** tell emacs we use -*- c++ -*- style comments *******************
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

#include "FRTDP.h"

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

#define FRTDP_INIT_MAX_DEPTH (10)
#define FRTDP_MAX_DEPTH_ADJUST_RATIO (1.1)
#define FRTDP_QUALITY_MARGIN (1e-5)

namespace zmdp {

FRTDP::FRTDP(void) {
  oldMaxDepth = 0;
  maxDepth = FRTDP_INIT_MAX_DEPTH;
}

void FRTDP::getNodeHandler(MDPNode &cn) {
  FRTDPExtraNodeData *searchData = new FRTDPExtraNodeData;
  cn.searchData = searchData;
  double excessWidth =
      cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  searchData->prio =
      (excessWidth <= 0) ? RT_PRIO_MINUS_INFINITY : log(excessWidth);
}

void FRTDP::staticGetNodeHandler(MDPNode &s, void *handlerData) {
  FRTDP *x = reinterpret_cast<FRTDP *>(handlerData);
  x->getNodeHandler(s);
}

double &FRTDP::getPrio(const MDPNode &cn) {
  return (reinterpret_cast<FRTDPExtraNodeData *>(cn.searchData))->prio;
}

void FRTDP::getMaxPrioOutcome(MDPNode &cn, int a, FRTDPUpdateResult &r) const {
  r.maxPrio = -99e+20;
  r.maxPrioOutcome = -1;
  double prio;
  MDPQEntry &Qa = cn.Q[a];
  FOR(o, Qa.getNumOutcomes()) {
    MDPEdge *e = Qa.outcomes[o];
    if (NULL != e) {
      prio = log(problem->getDiscount() * e->obsProb) + getPrio(*e->nextState);
      if (prio > r.maxPrio) {
        r.maxPrio = prio;
        r.maxPrioOutcome = o;
      }
#if 0
      MDPNode& sn = *e->nextState;
      printf("    a=%d o=%d obsProb=%g nslb=%g nsub=%g nsprio=%g prio=%g\n",
             a, o, e->obsProb, sn.lbVal, sn.ubVal, getPrio(sn), prio);
      if (getPrio(*e->nextState) < -99e+20) {
        MDPNode& sn = *e->nextState;
        printf("ns: s=[%s] [%g .. %g] prio=%g\n",
               denseRep(sn.s).c_str(), sn.lbVal, sn.ubVal, getPrio(sn));
      }
#endif
    }
  }
}

void FRTDP::update(MDPNode &cn, FRTDPUpdateResult &r) {
  double oldUBVal = cn.ubVal;
  bounds->update(cn, &r.maxUBAction);
  trackBackup(cn);

  r.ubResidual = oldUBVal - cn.ubVal;

  getMaxPrioOutcome(cn, r.maxUBAction, r);

  double excessWidth =
      cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  getPrio(cn) = std::min(r.maxPrio, (excessWidth <= 0) ? RT_PRIO_MINUS_INFINITY
                                                       : log(excessWidth));

  // getPrio(cn) = r.maxPrio;
}

void FRTDP::trialRecurse(MDPNode &cn, double logOcc, int depth) {
  FRTDPUpdateResult r;
  update(cn, r);

  double excessWidth =
      cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  double occ = (logOcc < -50) ? 0 : exp(logOcc);
  double updateQuality = r.ubResidual * occ;

#if 0
  // now done in update() itself
  getPrio(cn) = std::min(getPrio(cn), (excessWidth <= 0)
                         ? RT_PRIO_MINUS_INFINITY : log(excessWidth));
#endif

  if (zmdpDebugLevelG >= 1) {
    printf("  trialRecurse: depth=%d [%g .. %g] a=%d o=%d\n", depth, cn.lbVal,
           cn.ubVal, r.maxUBAction, r.maxPrioOutcome);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
  }

#if 0
  printf("  tr: maxUBAction=%d ubResidual=%g\n",
         r.maxUBAction, r.ubResidual);
  printf("  tr: maxPrioOutcome=%d maxPrio=%g\n",
         r.maxPrioOutcome, r.maxPrio);
#endif

  if (depth > oldMaxDepth) {
    newQualitySum += updateQuality;
    newNumUpdates++;
  } else {
    oldQualitySum += updateQuality;
    oldNumUpdates++;
  }

  if (excessWidth <= 0 || depth > maxDepth) {
    if (zmdpDebugLevelG >= 1) {
      printf("  trialRecurse: depth=%d excessWidth=%g (terminating)\n", depth,
             excessWidth);
      printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
    }

    return;
  }

  // recurse to successor
  assert(-1 != r.maxPrioOutcome);
  double obsProb = cn.Q[r.maxUBAction].outcomes[r.maxPrioOutcome]->obsProb;
  double weight = problem->getDiscount() * obsProb;
  double nextLogOcc = logOcc + log(weight);
  trialRecurse(cn.getNextState(r.maxUBAction, r.maxPrioOutcome), nextLogOcc,
               depth + 1);

  update(cn, r);
}

bool FRTDP::doTrial(MDPNode &cn) {
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials + 1));
  }

  oldQualitySum = 0;
  oldNumUpdates = 0;
  newQualitySum = 0;
  newNumUpdates = 0;

  trialRecurse(cn,
               /* logOcc = */ log(1.0),
               /* depth = */ 0);

  double updateQualityDiff;
  if (0 == oldQualitySum) {
    updateQualityDiff = 1000;
  } else if (0 == newNumUpdates) {
    updateQualityDiff = -1000;
  } else {
    double oldMean = oldQualitySum / oldNumUpdates;
    double newMean = newQualitySum / newNumUpdates;
    updateQualityDiff = newMean - oldMean;
  }

  if (updateQualityDiff > -FRTDP_QUALITY_MARGIN) {
    oldMaxDepth = maxDepth;
    maxDepth *= FRTDP_MAX_DEPTH_ADJUST_RATIO;
    if (zmdpDebugLevelG >= 1) {
      printf("endTrial: updateQualityDiff=%g oldMaxDepth=%g maxDepth=%g\n",
             updateQualityDiff, oldMaxDepth, maxDepth);
    }
  } else {
    if (zmdpDebugLevelG >= 1) {
      printf("endTrial: updateQualityDiff=%g maxDepth=%g (no change)\n",
             updateQualityDiff, maxDepth);
    }
  }

#if 0
  printf("endTrial: oldQualitySum=%g oldNumUpdates=%d newQualitySum=%g "
         "newNumUpdates=%d\n",
         oldQualitySum, oldNumUpdates, newQualitySum, newNumUpdates);
#endif

  numTrials++;

  return (cn.ubVal - cn.lbVal < targetPrecision);
}

void FRTDP::derivedClassInit(void) {
  bounds->addGetNodeHandler(&FRTDP::staticGetNodeHandler, this);
}

};  // namespace zmdp
