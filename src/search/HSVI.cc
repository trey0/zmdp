/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-10-30 20:00:15 $
   
 @file    HSVI.cc
 @brief   No brief

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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "HSVI.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define HSVI_INIT_MAX_DEPTH (10)
#define HSVI_MAX_DEPTH_ADJUST_RATIO (1.1)
#define HSVI_IMPROVEMENT_CONSTANT (0.95)

namespace zmdp {

HSVI::HSVI(void)
{
#if USE_HSVI_ADAPTIVE_DEPTH
  maxDepth = HSVI_INIT_MAX_DEPTH;
#endif
}

void HSVI::getMaxExcessUncOutcome(MDPNode& cn, int depth, HSVIUpdateResult& r) const
{
  r.maxExcessUnc = -99e+20;
  r.maxExcessUncOutcome = -1;
  double width;
  MDPQEntry& Qa = cn.Q[r.maxUBAction];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      MDPNode& sn = *e->nextState;
      width = e->obsProb *
	(sn.ubVal - sn.lbVal - trialTargetPrecision * pow(problem->getDiscount(), -(depth+1)));
      if (width > r.maxExcessUnc) {
	r.maxExcessUnc = width;
	r.maxExcessUncOutcome = o;
      }
#if 0
      printf("    a=%d o=%d obsProb=%g nslb=%g nsub=%g nsdiff=%g width=%g\n",
	     r.maxUBAction, o, e->obsProb, sn.lbVal, sn.ubVal, sn.ubVal - sn.lbVal, width);
#endif
    }
  }
}

void HSVI::update(MDPNode& cn, int depth, HSVIUpdateResult& r)
{
  double oldUBVal = cn.ubVal;
  bounds->update(cn, &r.maxUBAction);
  trackBackup(cn);
  
  r.ubResidual = oldUBVal - r.maxUBVal;

  getMaxExcessUncOutcome(cn, depth, r);
}

void HSVI::trialRecurse(MDPNode& cn, double logOcc, int depth)
{
  double excessUnc = cn.ubVal - cn.lbVal - trialTargetPrecision
    * pow(problem->getDiscount(), -depth);

  if (excessUnc <= 0
#if USE_HSVI_ADAPTIVE_DEPTH      
      || depth > maxDepth
#endif
      ) {
    if (zmdpDebugLevelG >= 1) {
      printf("  trialRecurse: depth=%d excessUnc=%g (terminating)\n",
	     depth, excessUnc);
      printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
    }

    return;
  }

  HSVIUpdateResult r;
  update(cn, depth, r);

#if USE_HSVI_ADAPTIVE_DEPTH
  double occ = (logOcc < -50) ? 0 : exp(logOcc);
  double updateQuality = r.ubResidual * occ;
  if (depth > oldMaxDepth) {
    oldQualitySum += updateQuality;
    oldNumUpdates++;
  } else {
    newQualitySum += updateQuality;
    newNumUpdates++;
  }
#endif

  if (zmdpDebugLevelG >= 1) {
    printf("  trialRecurse: depth=%d [%g .. %g] a=%d o=%d\n",
	   depth, cn.lbVal, cn.ubVal, r.maxUBAction, r.maxExcessUncOutcome);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
  }

  // recurse to successor
  assert(-1 != r.maxExcessUncOutcome);
  double obsProb = cn.Q[r.maxUBAction].outcomes[r.maxExcessUncOutcome]->obsProb;
  double weight = problem->getDiscount() * obsProb;
  double nextLogOcc = logOcc + log(weight);
  trialRecurse(cn.getNextState(r.maxUBAction, r.maxExcessUncOutcome),
	       nextLogOcc, depth+1);

  update(cn, depth, r);
}

bool HSVI::doTrial(MDPNode& cn)
{
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials+1));
  }

#if USE_HSVI_ADAPTIVE_DEPTH
  oldQualitySum = 0;
  oldNumUpdates = 0;
  newQualitySum = 0;
  newNumUpdates = 0;
#endif

  trialTargetPrecision = (cn.ubVal - cn.lbVal) * HSVI_IMPROVEMENT_CONSTANT;

  trialRecurse(cn,
	       /* logOcc = */ log(1.0),
	       /* depth = */ 0);

#if USE_HSVI_ADAPTIVE_DEPTH
  double updateQualityRatio;
  if (0 == oldQualitySum) {
    updateQualityRatio = 1000;
  } else if (0 == newNumUpdates) {
    updateQualityRatio = 0;
  } else {
    double oldMean = oldQualitySum / oldNumUpdates;
    double newMean = newQualitySum / newNumUpdates;
    updateQualityRatio = newMean / oldMean;
  }
  
  if (updateQualityRatio >= 1.0) {
    oldMaxDepth = maxDepth;
    maxDepth *= HSVI_MAX_DEPTH_ADJUST_RATIO;
    if (zmdpDebugLevelG >= 1) {
      printf("endTrial: updateQualityRatio=%g oldMaxDepth=%g maxDepth=%g\n",
	     updateQualityRatio, oldMaxDepth, maxDepth);
    }
  } else {
    if (zmdpDebugLevelG >= 1) {
      printf("endTrial: updateQualityRatio=%g maxDepth=%g (no change)\n",
	     updateQualityRatio, maxDepth);
    }
  }
#endif // if USE_HSVI_ADAPTIVE_DEPTH

  numTrials++;

  return (cn.ubVal - cn.lbVal < targetPrecision);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/10/19 19:31:16  trey
 * added support for backup logging
 *
 * Revision 1.2  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.1  2006/04/27 20:18:08  trey
 * renamed WRTDP to HSVI, maybe less confusing this way
 *
 * Revision 1.2  2006/04/12 19:48:47  trey
 * fixed sign error in excess uncertainty calculation; fixed some code to properly be inside a #if USE_HSVI_ADAPTIVE_DEPTH block
 *
 * Revision 1.1  2006/04/12 19:22:41  trey
 * initial check-in
 *
 *
 ***************************************************************************/
