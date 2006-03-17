/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-03-17 20:05:57 $
   
 @file    ARTDP.cc
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
#include "ARTDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define ARTDP_ALT_PRIO_MARGIN (log(100))
#define USE_ARTDP_MIN_TERM (1)

namespace zmdp {

ARTDPParamInfo::ARTDPParamInfo(const string& _name, double initVal)
{
  name = _name;
  init(initVal);
}

void ARTDPParamInfo::init(double initVal)
{
  val[0] = ARTDP_UNDEFINED;
  val[1] = initVal;
  val[2] = ARTDP_UNDEFINED;

  currentValIndex = 1;
  lastBatchQualityAtVal[0] = ARTDP_UNDEFINED;
  lastBatchQualityAtVal[1] = ARTDP_UNDEFINED;
  batchesPerAdvance = 1;
  batchesSoFarInAdvance = 0;
  batchQualitySum = 0;
  trialsSoFarInBatch = 0;
#if USE_ARTDP_MIN_TERM
  minTermThreshold = 99e+20;
#else
  thresholdsSoFarInTrial = 0;
#endif

  currentVal = val[currentValIndex];
}

void ARTDPParamInfo::recordTermination(double termThreshold)
{
  // record termThreshold for val[2] calculation if necessary
  if (1 == currentValIndex && ARTDP_UNDEFINED == val[2]) {
#if USE_ARTDP_MIN_TERM
    minTermThreshold = std::min(minTermThreshold, termThreshold);
#else
    terminateThresholds[thresholdsSoFarInTrial % ARTDP_TERMINATE_THRESHOLD_MAX_VALUES]
      = termThreshold;
#endif
    thresholdsSoFarInTrial++;
  }
}

void ARTDPParamInfo::endTrial(double trialQuality)
{
#if USE_DEBUG_PRINT
  printf("entering endTrial (%s): trialQuality=%g vals=[%g %g %g] index=%d\n",
	 name.c_str(), trialQuality, val[0], val[1], val[2], currentValIndex);
#endif

  // calculate val[2] if necessary
  if (1 == currentValIndex && ARTDP_UNDEFINED == val[2] && thresholdsSoFarInTrial > 0) {
#if USE_ARTDP_MIN_TERM
    val[2] = minTermThreshold;
    printf("endTrial: thresholdsSoFarInTrial=%d minTermThreshold=%g\n",
	   thresholdsSoFarInTrial, minTermThreshold);

    thresholdsSoFarInTrial = 0;
    minTermThreshold = 99e+20;
#else
    int numThresholds = std::min(thresholdsSoFarInTrial, ARTDP_TERMINATE_THRESHOLD_MAX_VALUES);
    std::sort(terminateThresholds, &terminateThresholds[numThresholds]);
    int selectIndex = (int) (numThresholds * ARTDP_QUANTILE);

    val[2] = terminateThresholds[selectIndex];
    printf("endTrial: numThresholds=%d selectIndex=%d val2=%g\n",
	   numThresholds, selectIndex, val[2]);

    thresholdsSoFarInTrial = 0;
#endif
  }

  // record trial end and quality data
  batchQualitySum += trialQuality;
  trialsSoFarInBatch++;
  
  // check for the end of the batch
  if (ARTDP_BATCH_SIZE == trialsSoFarInBatch) {
    lastBatchQualityAtVal[currentValIndex] = batchQualitySum / ARTDP_BATCH_SIZE;
    printf("endTrial (%s): batch complete with quality=%g\n",
	   name.c_str(), lastBatchQualityAtVal[currentValIndex]);

    if (0 == currentValIndex) {
      batchesSoFarInAdvance++;
      if (batchesSoFarInAdvance >= batchesPerAdvance) {
	// advance to next value
	printf("endTrial: advancing\n");
	currentValIndex = 1;
      } else {
	printf("endTrial: waiting before next advance (%d/%g)\n",
	       batchesSoFarInAdvance, batchesPerAdvance);
      }
    } else {
      if (ARTDP_UNDEFINED == val[2]) {
	// no change, wait until val[2] is defined
	printf("endTrial: no terminations; no change\n");
      } else if (ARTDP_UNDEFINED == val[0]
		 || lastBatchQualityAtVal[1] >= lastBatchQualityAtVal[0]) {
	// accept val[1] and advance to val[2]
	printf("endTrial (%s): accepting v=%g\n", name.c_str(), val[currentValIndex]);
	val[0] = val[1];
	val[1] = val[2];
	val[2] = ARTDP_UNDEFINED;
	// keep currentValIndex = 1 -- the new val[1] is the old val[2]
	lastBatchQualityAtVal[0] = lastBatchQualityAtVal[1];
	lastBatchQualityAtVal[1] = ARTDP_UNDEFINED;
	batchesPerAdvance = 1;
      } else {
	// revert to val[0]
	printf("endTrial: reverting (qual0=%g qual1=%g)\n",
	       lastBatchQualityAtVal[0], lastBatchQualityAtVal[1]);
	currentValIndex = 0;
	batchesPerAdvance *= ARTDP_ADVANCE_GROWTH_RATE;
	batchesSoFarInAdvance = 0;
      }
    }

    // clear data for next batch
    batchQualitySum = 0;
    trialsSoFarInBatch = 0;
  }

  currentVal = val[currentValIndex];

#if USE_DEBUG_PRINT
  printf("exiting endTrial (%s): vals=[%g %g %g] index=%d\n",
	 name.c_str(), val[0], val[1], val[2], currentValIndex);
#endif
}

ARTDP::ARTDP(AbstractBound* _initUpperBound) :
  RTDPCore(_initUpperBound),
  minF("minF", 99e+20),
  minLogRelevance("minLogRelevance", 99e+20)
{}

void ARTDP::update2(MDPNode& cn, ARTDPUpdateResult& r)
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

  numBackups++;
}

void ARTDP::trialRecurse(MDPNode& cn, double g, double logOcc, int depth)
{
  if (cn.isFringe()) {
    expand(cn);
  }

  ARTDPUpdateResult r;
  update2(cn, r);

  double excessWidth = cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  double logRelevance = logOcc + log(excessWidth);

  double occ = (logOcc < -50) ? 0 : exp(logOcc);
  double relevance = occ * excessWidth;
  double updateQuality = r.ubResidual * relevance;
  trialQualitySum += updateQuality;
  updatesSoFarInTrial++;

#if USE_DEBUG_PRINT
  printf("  trialRecurse: depth=%d [%g .. %g] a=%d o=%d\n",
	 depth, cn.lbVal, cn.ubVal, r.maxUBAction, r.maxPrioOutcome);
  printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

#if 0
  printf("  tr: maxUBAction=%d maxUBVal=%g secondBestUBVal=%g ubResidual=%g\n",
	 r.maxUBAction, r.maxUBVal, r.secondBestUBVal, r.ubResidual);
  printf("  tr: maxPrioOutcome=%d maxPrio=%g secondBestPrio=%g\n",
	 r.maxPrioOutcome, r.maxPrio, r.secondBestPrio);
#endif

  bool terminateTrial =false;

  if (excessWidth < 0) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: terminating because excessWidth=%g\n",
	   excessWidth);
#endif
    terminateTrial = true;
  }

  if (!terminateTrial) {
    double f = g + cn.ubVal; // 'f = g + h'
    if (f < minF.currentVal) {
#if USE_DEBUG_PRINT
      printf("  trialRecurse: terminating because f=%g minF=%g\n",
	     f, minF.currentVal);
#endif
      minF.recordTermination(f);
      terminateTrial = true;
    }
  }

  if (!terminateTrial) {
    if (logRelevance < minLogRelevance.currentVal) {
#if USE_DEBUG_PRINT
      printf("  trialRecurse: terminating because logRel=%g minLogRel=%g\n",
	     logRelevance, minLogRelevance.currentVal);
#endif
      minLogRelevance.recordTermination(logRelevance);
      terminateTrial = true;
    }
  }

  if (terminateTrial) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: depth=%d excessWidth=%g (terminating)\n",
	   depth, excessWidth);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif
    return;
  }

  // recurse to successor
  MDPQEntry& Qa = cn.Q[r.maxUBAction];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      double weight = problem->getDiscount() * e->obsProb;
      double nextLogOcc = logOcc + log(weight);
      double nextG = g + Qa.immediateReward;
      trialRecurse(*e->nextState, nextG, nextLogOcc, depth+1);
    }
  }

  update2(cn, r);

  updateQuality = r.ubResidual * relevance;
  trialQualitySum += updateQuality;
  updatesSoFarInTrial++;
}

bool ARTDP::doTrial(MDPNode& cn)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  trialQualitySum = 0;
  updatesSoFarInTrial = 0;

  trialRecurse(cn,
	       /* g = */ 0,
	       /* logOcc = */ log(1.0),
	       /* depth = */ 0);

  double trialQuality = trialQualitySum / updatesSoFarInTrial;
  printf("calc trialQuality: trialQualitySum=%g updatesSoFarInTrial=%d\n",
	 trialQualitySum, updatesSoFarInTrial);
  minF.endTrial(trialQuality);
  minLogRelevance.endTrial(trialQuality);

  numTrials++;

  return (cn.ubVal - cn.lbVal < targetPrecision);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
