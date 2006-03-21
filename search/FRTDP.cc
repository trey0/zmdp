/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2006-03-21 21:09:25 $
   
 @file    FRTDP.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#define FRTDP_ALT_PRIO_MARGIN (log(100))
#define FRTDP_UNDEFINED (-999)
#define FRTDP_INIT_MAX_DEPTH (10)
#define FRTDP_MAX_DEPTH_ADJUST_RATIO (1.1)

namespace zmdp {

FRTDP::FRTDP(AbstractBound* _initUpperBound) :
  RTDPCore(_initUpperBound)
{
  maxDepth = FRTDP_INIT_MAX_DEPTH;
}

void FRTDP::getMaxPrioOutcome(MDPNode& cn, int a, FRTDPUpdateResult& r) const
{
  r.maxPrio = -99e+20;
  r.secondBestPrio = -99e+20;
  r.maxPrioOutcome = -1;
  double prio;
  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      prio = log(problem->getDiscount() * e->obsProb) + e->nextState->prio;
      if (prio > r.maxPrio) {
	r.secondBestPrio = r.maxPrio;
	r.maxPrio = prio;
	r.maxPrioOutcome = o;
      } else if (prio > r.secondBestPrio) {
	r.secondBestPrio = prio;
      }
#if 0
      printf("    a=%d o=%d obsProb=%g nsprio=%g prio=%g\n",
	     a, o, e->obsProb, e->nextState->prio, prio);
      if (e->nextState->prio < -99e+20) {
	MDPNode& sn = *e->nextState;
	printf("ns: s=[%s] [%g .. %g] prio=%g\n",
	       denseRep(sn.s).c_str(), sn.lbVal, sn.ubVal, sn.prio);
      }
#endif
    }
  }
}

void FRTDP::update2(MDPNode& cn, FRTDPUpdateResult& r)
{
  double maxLBVal = -99e+20;
  r.maxUBVal = -99e+20;
  r.secondBestUBVal = -99e+20;
  r.maxUBAction = -1;
  double lbVal, ubVal;
  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    lbVal = 0;
    ubVal = 0;
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	double oprob = e->obsProb;
	lbVal += oprob * sn.lbVal;
	ubVal += oprob * sn.ubVal;
      }
    }
    Qa.lbVal = lbVal = Qa.immediateReward + problem->getDiscount() * lbVal;
    Qa.ubVal = ubVal = Qa.immediateReward + problem->getDiscount() * ubVal;

    maxLBVal = std::max(maxLBVal, lbVal);
    if (ubVal > r.maxUBVal) {
      r.secondBestUBVal = r.maxUBVal;
      r.maxUBVal = ubVal;
      r.maxUBAction = a;
    } else if (ubVal > r.secondBestUBVal) {
      r.secondBestUBVal = ubVal;
    }
  }

#if 1
  // min and max calls here only necessary if bounds are not uniformly improvable
  maxLBVal = std::max(cn.lbVal, maxLBVal);
  r.maxUBVal = std::min(cn.ubVal, r.maxUBVal);
#endif

  r.ubResidual = cn.ubVal - r.maxUBVal;

  cn.lbVal = maxLBVal;
  cn.ubVal = r.maxUBVal;

  getMaxPrioOutcome(cn, r.maxUBAction, r);
  cn.prio = r.maxPrio;

  numBackups++;
}

void FRTDP::trialRecurse(MDPNode& cn, double actionDelta, double altPrio, double logOcc, int depth)
{
  if (cn.isFringe()) {
    expand(cn);
  }

  FRTDPUpdateResult r;
  update2(cn, r);

  double excessWidth = cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  double occ = (logOcc < -50) ? 0 : exp(logOcc);
  double updateQuality = r.ubResidual * occ;

  // is there a better way to enforce this?
  cn.prio = std::min(cn.prio, (excessWidth <= 0) ? RT_PRIO_MINUS_INFINITY : log(excessWidth));

#if USE_DEBUG_PRINT
  printf("  trialRecurse: depth=%d [%g .. %g] actionDelta=%g altPrio=%g a=%d o=%d\n",
	 depth, cn.lbVal, cn.ubVal, actionDelta, altPrio, r.maxUBAction, r.maxPrioOutcome);
  printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

#if 0
  printf("  tr: maxUBAction=%d maxUBVal=%g secondBestUBVal=%g ubResidual=%g\n",
	 r.maxUBAction, r.maxUBVal, r.secondBestUBVal, r.ubResidual);
  printf("  tr: maxPrioOutcome=%d maxPrio=%g secondBestPrio=%g\n",
	 r.maxPrioOutcome, r.maxPrio, r.secondBestPrio);
#endif

  if (depth > oldMaxDepth) {
    oldQualitySum += updateQuality;
    oldNumUpdates++;
  } else {
    newQualitySum += updateQuality;
    newNumUpdates++;
  }

  if (excessWidth < 0
      || depth > maxDepth
#if USE_FRTDP_ALT_PRIO
      || (altPrio - r.maxPrio) > -0.9 * logOcc // FRTDP_ALT_PRIO_MARGIN
#endif
      //|| actionDelta < -targetPrecision
      ) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: depth=%d actionDelta=%g altPrio=%g excessWidth=%g (terminating)\n",
	   depth, actionDelta, altPrio, excessWidth);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

    return;
  }

  // recurse to successor
  double obsProb = cn.Q[r.maxUBAction].outcomes[r.maxPrioOutcome]->obsProb;
  double weight = problem->getDiscount() * obsProb;
  double nextActionDelta = std::min(r.maxUBVal - r.secondBestUBVal,
				    (actionDelta - r.ubResidual) / weight);
  double nextAltPrio = std::max(r.secondBestPrio,
				altPrio - log(weight));
  double nextLogOcc = logOcc + log(weight);
  trialRecurse(cn.getNextState(r.maxUBAction, r.maxPrioOutcome),
	       nextActionDelta, nextAltPrio, nextLogOcc, depth+1);

  update2(cn, r);
}

bool FRTDP::doTrial(MDPNode& cn)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  oldQualitySum = 0;
  oldNumUpdates = 0;
  newQualitySum = 0;
  newNumUpdates = 0;

  trialRecurse(cn,
	       /* actionDelta = */ 99e+20,
	       /* altPrio = */ RT_PRIO_MINUS_INFINITY,
	       /* logOcc = */ log(1.0),
	       /* depth = */ 0);

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
    maxDepth *= FRTDP_MAX_DEPTH_ADJUST_RATIO;
#if USE_DEBUG_PRINT
    printf("endTrial: updateQualityRatio=%g oldMaxDepth=%g maxDepth=%g\n",
	   updateQualityRatio, oldMaxDepth, maxDepth);
#endif
  } else {
#if USE_DEBUG_PRINT
  printf("endTrial: updateQualityRatio=%g maxDepth=%g (no change)\n",
	 updateQualityRatio, maxDepth);
#endif
  }

  numTrials++;

  return (cn.ubVal - cn.lbVal < targetPrecision);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
