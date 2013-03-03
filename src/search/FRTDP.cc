/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.21 $  $Author: trey $  $Date: 2007-04-08 22:45:39 $
   
 @file    FRTDP.cc
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
#include "FRTDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define FRTDP_INIT_MAX_DEPTH (10)
#define FRTDP_MAX_DEPTH_ADJUST_RATIO (1.1)
#define FRTDP_QUALITY_MARGIN (1e-5)

namespace zmdp {

FRTDP::FRTDP(void)
{
  oldMaxDepth = 0;
  maxDepth = FRTDP_INIT_MAX_DEPTH;
}

void FRTDP::getNodeHandler(MDPNode& cn)
{
  FRTDPExtraNodeData* searchData = new FRTDPExtraNodeData;
  cn.searchData = searchData;
  double excessWidth = cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  searchData->prio = (excessWidth <= 0) ? RT_PRIO_MINUS_INFINITY : log(excessWidth);
}

void FRTDP::staticGetNodeHandler(MDPNode& s, void* handlerData)
{
  FRTDP* x = (FRTDP *) handlerData;
  x->getNodeHandler(s);
}

double& FRTDP::getPrio(const MDPNode& cn)
{
  return ((FRTDPExtraNodeData*) cn.searchData)->prio;
}

void FRTDP::getMaxPrioOutcome(MDPNode& cn, int a, FRTDPUpdateResult& r) const
{
  r.maxPrio = -99e+20;
  r.maxPrioOutcome = -1;
  double prio;
  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
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

void FRTDP::update(MDPNode& cn, FRTDPUpdateResult& r)
{
  double oldUBVal = cn.ubVal;
  bounds->update(cn, &r.maxUBAction);
  trackBackup(cn);
  
  r.ubResidual = oldUBVal - cn.ubVal;

  getMaxPrioOutcome(cn, r.maxUBAction, r);

  double excessWidth = cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  getPrio(cn) = std::min(r.maxPrio, (excessWidth <= 0)
			 ? RT_PRIO_MINUS_INFINITY : log(excessWidth));

  //getPrio(cn) = r.maxPrio;
}

void FRTDP::trialRecurse(MDPNode& cn, double logOcc, int depth)
{
  FRTDPUpdateResult r;
  update(cn, r);

  double excessWidth = cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  double occ = (logOcc < -50) ? 0 : exp(logOcc);
  double updateQuality = r.ubResidual * occ;

#if 0
  // now done in update() itself
  getPrio(cn) = std::min(getPrio(cn), (excessWidth <= 0)
			 ? RT_PRIO_MINUS_INFINITY : log(excessWidth));
#endif

  if (zmdpDebugLevelG >= 1) {
    printf("  trialRecurse: depth=%d [%g .. %g] a=%d o=%d\n",
	   depth, cn.lbVal, cn.ubVal, r.maxUBAction, r.maxPrioOutcome);
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
      printf("  trialRecurse: depth=%d excessWidth=%g (terminating)\n",
	     depth, excessWidth);
      printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
    }

    return;
  }

  // recurse to successor
  assert(-1 != r.maxPrioOutcome);
  double obsProb = cn.Q[r.maxUBAction].outcomes[r.maxPrioOutcome]->obsProb;
  double weight = problem->getDiscount() * obsProb;
  double nextLogOcc = logOcc + log(weight);
  trialRecurse(cn.getNextState(r.maxUBAction, r.maxPrioOutcome),
	       nextLogOcc, depth+1);

  update(cn, r);
}

bool FRTDP::doTrial(MDPNode& cn)
{
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials+1));
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
  printf("endTrial: oldQualitySum=%g oldNumUpdates=%d newQualitySum=%g newNumUpdates=%d\n",
	 oldQualitySum, oldNumUpdates, newQualitySum, newNumUpdates);
#endif

  numTrials++;

  return (cn.ubVal - cn.lbVal < targetPrecision);
}

void FRTDP::derivedClassInit(void)
{
  bounds->addGetNodeHandler(&FRTDP::staticGetNodeHandler, this);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.20  2006/10/30 20:00:15  trey
 * USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
 *
 * Revision 1.19  2006/10/24 02:37:05  trey
 * updated for modified bounds interfaces
 *
 * Revision 1.18  2006/10/19 19:31:16  trey
 * added support for backup logging
 *
 * Revision 1.17  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.16  2006/04/26 18:45:28  trey
 * corrected nasty bug with an uninitialized variable, not sure why this worked before
 *
 * Revision 1.15  2006/04/12 19:22:22  trey
 * removed #define of no longer used macros
 *
 * Revision 1.14  2006/04/08 22:23:12  trey
 * added some debug code
 *
 * Revision 1.13  2006/04/07 19:41:17  trey
 * removed initLowerBound, initUpperBound arguments to constructor
 *
 * Revision 1.12  2006/04/06 04:14:11  trey
 * changed how bounds are initialized
 *
 * Revision 1.11  2006/04/04 17:23:28  trey
 * made getPrio() static
 *
 * Revision 1.10  2006/04/03 21:39:03  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.9  2006/03/21 21:24:43  trey
 * removed code that was obsoleted by new termination condition
 *
 * Revision 1.8  2006/03/21 21:09:25  trey
 * added adaptive maxDepth termination criterion
 *
 * Revision 1.7  2006/02/20 02:04:56  trey
 * changed altPrio margin to be based on occupancy
 *
 * Revision 1.6  2006/02/20 00:05:35  trey
 * added FRTDP_ALT_PRIO_MARGIN
 *
 * Revision 1.5  2006/02/19 18:34:35  trey
 * lots of changes, trying out different termination approaches
 *
 * Revision 1.4  2006/02/17 21:09:50  trey
 * made updates robust to a non-uniformly-improvable lower bound
 *
 * Revision 1.3  2006/02/15 16:24:28  trey
 * switched to a better termination criterion
 *
 * Revision 1.2  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.1  2006/02/13 21:46:46  trey
 * initial check-in
 *
 *
 ***************************************************************************/
