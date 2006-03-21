/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-03-21 16:46:21 $
   
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

#define USE_ARTDP_MIN_TERM (1)
#define USE_ARTDP_ADVANCE_ALL (0)
#define USE_ARTDP_APPLY_TERM_UPDATE (1)
#define USE_ARTDP_DISCREP_ZERO (1)
#define USE_ARTDP_WITH_PRIO (1)
#define ARTDP_MIN_DELTA (1.0)

namespace zmdp {

ARTDPParamInfo::ARTDPParamInfo(const string& _name, double initVal)
{
  init(_name, initVal);
}

void ARTDPParamInfo::init(const string& _name, double initVal)
{
  name = _name;
  controlVal = 99e+20;
  val = initVal;

#if USE_ARTDP_MIN_TERM
  minTermNodeValue = 99e+20;
#endif

  currNodeQualitySum = 0;
  numCurrNodes = 0;
  controlNodeQualitySum = 0;
  numControlNodes = 0;
  numTermNodes = 0;
}

bool ARTDPParamInfo::recordNode(double nodeVal, double updateQuality)
{
#if 0
  printf("recordNode (%s): nodeVal=%g updateQuality=%g\n",
	 name.c_str(), nodeVal, updateQuality);
#endif

  if (nodeVal < controlVal) {
    currNodeQualitySum += updateQuality;
    numCurrNodes++;
  } else {
    controlNodeQualitySum += updateQuality;
    numControlNodes++;
  }

  if (nodeVal < val) {
#if USE_ARTDP_MIN_TERM
    minTermNodeValue = std::min(minTermNodeValue, nodeVal);
#else
    if (numTermNodes < ARTDP_TERM_NODE_ARR_SIZE) {
      termNodeValues[numTermNodes] = nodeVal;
    } else {
      double insertProb = ((double) ARTDP_TERM_NODE_ARR_SIZE) / (numTermNodes+1);
      if (unit_rand() < insertProb) {
	int insertIndex = (int) (unit_rand() * ARTDP_TERM_NODE_ARR_SIZE);
	termNodeValues[insertIndex] = nodeVal;
      }
    }
#endif
    numTermNodes++;
    return true;
  } else {
    return false;
  }
}

double ARTDPParamInfo::calcDelta(void)
{
#if USE_DEBUG_PRINT
    printf("calcDelta: numCurrNodes=%d numControlNodes=%d numTermNodes=%d\n",
	   numCurrNodes, numControlNodes, numTermNodes);
#endif
  if (0 == numTermNodes) {
    return -99e+20;
  } else if (0 == numControlNodes) {
    return ARTDP_MIN_DELTA;
  } else {
    double currNodeQualityMean = currNodeQualitySum / numCurrNodes;
    double controlNodeQualityMean = controlNodeQualitySum / numControlNodes;
    if (0 == controlNodeQualityMean) {
      return 1000;
    } else {
      return currNodeQualityMean / controlNodeQualityMean;
    }
  }
}

ARTDP::ARTDP(AbstractBound* _initUpperBound) :
  RTDPCore(_initUpperBound)
{}

void ARTDP::endTrial(void)
{
#if USE_ARTDP_ADVANCE_ALL

  double delta;
  FOR (i, ARTDP_NUM_PARAMS) {
    ARTDPParamInfo& p = params[i];

    delta = p.calcDelta();
#if USE_DEBUG_PRINT
    printf("endTrial: i=%d val=%g controlVal=%g delta=%g\n",
	   i, p.val, p.controlVal, delta);
#endif

    if (delta >= ARTDP_MIN_DELTA) {
#if USE_ARTDP_MIN_TERM
      double newVal = p.minTermNodeValue;
#else
      int numValues = std::min(p.numTermNodes, ARTDP_TERM_NODE_ARR_SIZE);
      std::sort(p.termNodeValues, &p.termNodeValues[numValues]);
      int selectIndex = (int) (numValues * ARTDP_QUANTILE);
      double newVal = p.termNodeValues[selectIndex];
#endif

#if USE_DEBUG_PRINT
      printf("endTrial: update i=%d val=%g newVal=%g diff=%g\n",
	     i, p.val, newVal, newVal-p.val);
#endif
      p.controlVal = p.val;
      p.val = newVal;
    }
  }

#else // if USE_ARTDP_ADVANCE_ALL / else

  double bestDelta = ARTDP_MIN_DELTA - 1e-10;
  int bestDeltaIndex = -1;
  double delta;
  FOR (i, ARTDP_NUM_PARAMS) {
    delta = params[i].calcDelta();
#if USE_DEBUG_PRINT
    printf("endTrial: i=%d val=%g delta=%g\n", i, params[i].val, delta);
#endif
    if (delta > bestDelta) {
      bestDelta = delta;
      bestDeltaIndex = i;
    }
  }
  if (-1 != bestDeltaIndex) {
    ARTDPParamInfo& p = params[bestDeltaIndex];

#if USE_ARTDP_MIN_TERM
    double newVal = p.minTermNodeValue;
#else
    int numValues = std::min(p.numTermNodes, ARTDP_TERM_NODE_ARR_SIZE);
    std::sort(p.termNodeValues, &p.termNodeValues[numValues]);
    int selectIndex = (int) (numValues * ARTDP_QUANTILE);
    double newVal = p.termNodeValues[selectIndex];
#endif

#if USE_DEBUG_PRINT
    printf("endTrial: bestDeltaIndex=%d val=%g newVal=%g\n",
	   bestDeltaIndex, p.val, newVal);
#endif
    p.controlVal = p.val;
    p.val = newVal;
  }

#endif // if USE_ARTDP_ADVANCE_ALL / else

  // cleanup
  FOR (i, ARTDP_NUM_PARAMS) {
    ARTDPParamInfo& p = params[i];

    // FIX may want to add this back in some other way?
    //p.minTermNodeValue = 99e+20;

    p.currNodeQualitySum = 0;
    p.numCurrNodes = 0;
    p.controlNodeQualitySum = 0;
    p.numControlNodes = 0;
    p.numTermNodes = 0;
  }
}

void ARTDP::getMaxPrioOutcome(MDPNode& cn, int a, ARTDPUpdateResult& r) const
{
  r.maxPrio = -99e+20;
  r.maxPrioOutcome = -1;
  double prio;
  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      prio = log(problem->getDiscount() * e->obsProb) + e->nextState->prio;
      if (prio > r.maxPrio) {
	r.maxPrio = prio;
	r.maxPrioOutcome = o;
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

void ARTDP::update2(MDPNode& cn, ARTDPUpdateResult& r)
{
  r.maxLBVal = -99e+20;
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

    r.maxLBVal = std::max(r.maxLBVal, lbVal);
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
  r.maxLBVal = std::max(cn.lbVal, r.maxLBVal);
  r.maxUBVal = std::min(cn.ubVal, r.maxUBVal);
#endif

  r.ubResidual = cn.ubVal - r.maxUBVal;

#if USE_ARTDP_APPLY_TERM_UPDATE
  cn.lbVal = r.maxLBVal;
  cn.ubVal = r.maxUBVal;
#endif

#if USE_ARTDP_WITH_PRIO
  getMaxPrioOutcome(cn, r.maxUBAction, r);
  cn.prio = r.maxPrio;
#endif

  numBackups++;
}

double ARTDP::calcOutcomePrio(MDPNode &cn, double obsProb)
{
  double excessWidth = cn.ubVal - cn.lbVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  double prio0 = excessWidth * obsProb;
  if (prio0 > 1e-10) {
    return log(prio0);
  } else {
    return -99e+20;
  }
}

void ARTDP::trialRecurse(MDPNode& cn, double g, double logOcc, double discrep, int depth)
{
  if (cn.isFringe()) {
    expand(cn);
  }

  ARTDPUpdateResult r;
  update2(cn, r);

  double excessWidth = r.maxUBVal - r.maxLBVal - RT_PRIO_IMPROVEMENT_CONSTANT * targetPrecision;
  double logRelevance = logOcc + log(excessWidth);

  double occ = (logOcc < -50) ? 0 : exp(logOcc);
  double relevance = occ * excessWidth;
  double updateQuality = r.ubResidual * relevance;

  // is there a better way to enforce this?
  cn.prio = std::min(cn.prio, (excessWidth <= 0) ? RT_PRIO_MINUS_INFINITY : log(excessWidth));

#if USE_DEBUG_PRINT
  printf("  trialRecurse: depth=%d [%g .. %g] a=%d o=%d\n",
	 depth, r.maxLBVal, r.maxUBVal, r.maxUBAction, r.maxPrioOutcome);
  printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

#if 0
  printf("  tr: maxUBAction=%d maxUBVal=%g secondBestUBVal=%g ubResidual=%g\n",
	 r.maxUBAction, r.maxUBVal, r.secondBestUBVal, r.ubResidual);
  printf("  tr: maxPrioOutcome=%d maxPrio=%g secondBestPrio=%g\n",
	 r.maxPrioOutcome, r.maxPrio, r.secondBestPrio);
#endif

  bool terminateTrial = false;
  bool termTriggered;

  if (excessWidth < 0) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: width triggers termination excessWidth=%g\n",
	   excessWidth);
#endif
    terminateTrial = true;
  }

#if 1
  double f = g + r.maxUBVal; // 'f = g + h'
  termTriggered = getMinF().recordNode(f, updateQuality);
  if (termTriggered) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: f triggers termination f=%g minF=%g\n",
	   f, getMinF().val);
#endif
    terminateTrial = true;
  }
#endif

#if 0
  termTriggered = getMinLogRelevance().recordNode(logRelevance, updateQuality);
  if (termTriggered) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: rel triggers termination logRel=%g minLogRel=%g\n",
	   logRelevance, getMinLogRelevance().val);
#endif
    terminateTrial = true;
  }
#endif

#if !USE_ARTDP_DISCREP_ZERO
  termTriggered = getMinDiscrep().recordNode(discrep, updateQuality);
  if (termTriggered) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: discrep triggers termination discrep=%g minDiscrep=%g\n",
	   discrep, getMinDiscrep().val);
#endif
    terminateTrial = true;
  }
#endif

  termTriggered = getMinNegDepth().recordNode(-depth, updateQuality);
  if (termTriggered) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: depth triggers termination negDepth=%d minNegDepth=%g\n",
	   -depth, getMinNegDepth().val);
#endif
    terminateTrial = true;
  }

  if (terminateTrial) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: depth=%d excessWidth=%g (terminating)\n",
	   depth, excessWidth);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif
    return;
  }

#if !USE_ARTDP_APPLY_TERM_UPDATE
  cn.lbVal = r.maxLBVal;
  cn.ubVal = r.maxUBVal;
#endif

#if USE_ARTDP_WITH_PRIO
  MDPQEntry& Qa = cn.Q[r.maxUBAction];
  MDPEdge* e = Qa.outcomes[r.maxPrioOutcome];
  MDPNode& sn = *e->nextState;
  double weight = problem->getDiscount() * e->obsProb;
  double nextG = g + Qa.immediateReward;
  double nextLogOcc = logOcc + log(weight);
  double nextDiscrep = 0;
  trialRecurse(*e->nextState, nextG, nextLogOcc, nextDiscrep, depth+1);
#else // if USE_ARTDP_WITH_PRIO / else
  // recurse to successors
  MDPQEntry& Qa = cn.Q[r.maxUBAction];
  double bestOutcomePrio = -99e+20;
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      MDPNode& sn = *e->nextState;
      double outcomePrio = calcOutcomePrio(sn, e->obsProb);
      bestOutcomePrio = std::max(bestOutcomePrio, outcomePrio);
    }
  }
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      MDPNode& sn = *e->nextState;
      double outcomePrio = calcOutcomePrio(sn, e->obsProb);
      if (outcomePrio > -1000) {
	double weight = problem->getDiscount() * e->obsProb;
	double nextG = g + Qa.immediateReward;
	double nextLogOcc = logOcc + log(weight);
	double nextDiscrep = discrep + (outcomePrio - bestOutcomePrio);
#if USE_ARTDP_DISCREP_ZERO
	if (0 == nextDiscrep) {
	  trialRecurse(*e->nextState, nextG, nextLogOcc, nextDiscrep, depth+1);
	}
#else
	trialRecurse(*e->nextState, nextG, nextLogOcc, nextDiscrep, depth+1);
#endif
      }
    }
  }
#endif // if USE_ARTDP_WITH_PRIO / else

  update2(cn, r);
#if !USE_ARTDP_APPLY_TERM_UPDATE
  cn.lbVal = r.maxLBVal;
  cn.ubVal = r.maxUBVal;
#endif
}

bool ARTDP::doTrial(MDPNode& cn)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  trialRecurse(cn,
	       /* g = */ 0,
	       /* logOcc = */ log(1.0),
	       /* discrep = */ 0,
	       /* depth = */ 0);
  endTrial();

  numTrials++;

  return (cn.ubVal - cn.lbVal < targetPrecision);
}

void ARTDP::derivedClassInit(void)
{
  params[0].init("minF", 99e+20);
  params[1].init("minLogRelevance", 99e+20);
  params[2].init("minDiscrep", 99e+20);
  params[3].init("minNegDepth", -10);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/03/20 19:21:01  trey
 * quantile approximation is now stochastic, no longer biased toward values at the end of the trial
 *
 * Revision 1.2  2006/03/20 18:54:36  trey
 * adaptive params no longer advance in lock step
 *
 * Revision 1.1  2006/03/17 20:05:57  trey
 * initial check-in
 *
 *
 ***************************************************************************/
