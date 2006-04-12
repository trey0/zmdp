/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-12 19:48:47 $
   
 @file    WRTDP.cc
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
#include "WRTDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define WRTDP_INIT_MAX_DEPTH (10)
#define WRTDP_MAX_DEPTH_ADJUST_RATIO (1.1)
#define WRTDP_IMPROVEMENT_CONSTANT (0.95)

namespace zmdp {

WRTDP::WRTDP(void)
{
#if USE_WRTDP_ADAPTIVE_DEPTH
  maxDepth = WRTDP_INIT_MAX_DEPTH;
#endif
}

void WRTDP::getMaxExcessUncOutcome(MDPNode& cn, int depth, WRTDPUpdateResult& r) const
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

void WRTDP::update(MDPNode& cn, int depth, WRTDPUpdateResult& r)
{
  double oldUBVal = cn.ubVal;
  bounds->update(cn, &r.maxUBAction);
  
  r.ubResidual = oldUBVal - r.maxUBVal;

  getMaxExcessUncOutcome(cn, depth, r);
}

void WRTDP::trialRecurse(MDPNode& cn, double logOcc, int depth)
{
  double excessUnc = cn.ubVal - cn.lbVal - trialTargetPrecision
    * pow(problem->getDiscount(), -depth);

  if (excessUnc <= 0
#if USE_WRTDP_ADAPTIVE_DEPTH      
      || depth > maxDepth
#endif
      ) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: depth=%d excessUnc=%g (terminating)\n",
	   depth, excessUnc);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

    return;
  }

  WRTDPUpdateResult r;
  update(cn, depth, r);

#if USE_WRTDP_ADAPTIVE_DEPTH
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

#if USE_DEBUG_PRINT
  printf("  trialRecurse: depth=%d [%g .. %g] a=%d o=%d\n",
	 depth, cn.lbVal, cn.ubVal, r.maxUBAction, r.maxExcessUncOutcome);
  printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

  // recurse to successor
  assert(-1 != r.maxExcessUncOutcome);
  double obsProb = cn.Q[r.maxUBAction].outcomes[r.maxExcessUncOutcome]->obsProb;
  double weight = problem->getDiscount() * obsProb;
  double nextLogOcc = logOcc + log(weight);
  trialRecurse(cn.getNextState(r.maxUBAction, r.maxExcessUncOutcome),
	       nextLogOcc, depth+1);

  update(cn, depth, r);
}

bool WRTDP::doTrial(MDPNode& cn)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

#if USE_WRTDP_ADAPTIVE_DEPTH
  oldQualitySum = 0;
  oldNumUpdates = 0;
  newQualitySum = 0;
  newNumUpdates = 0;
#endif

  trialTargetPrecision = (cn.ubVal - cn.lbVal) * WRTDP_IMPROVEMENT_CONSTANT;

  trialRecurse(cn,
	       /* logOcc = */ log(1.0),
	       /* depth = */ 0);

#if USE_WRTDP_ADAPTIVE_DEPTH
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
    maxDepth *= WRTDP_MAX_DEPTH_ADJUST_RATIO;
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
#endif // if USE_WRTDP_ADAPTIVE_DEPTH

  numTrials++;

  return (cn.ubVal - cn.lbVal < targetPrecision);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/12 19:22:41  trey
 * initial check-in
 *
 *
 ***************************************************************************/
