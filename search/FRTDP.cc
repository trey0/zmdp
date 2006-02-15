/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-02-15 16:24:28 $
   
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

namespace zmdp {

FRTDP::FRTDP(AbstractBound* _initUpperBound) :
  RTDPCore(_initUpperBound)
{}

int FRTDP::getMaxPrioOutcome(MDPNode& cn, int a,
			     double* maxPrioP,
			     double* secondBestPrioP) const
{
  double maxPrio = -99e+20;
  double secondBestPrio = -99e+20;
  int maxPrioOutcome = -1;
  double prio;
  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      prio = log(e->obsProb) + e->nextState->prio;
      if (prio > maxPrio) {
	secondBestPrio = maxPrio;
	maxPrio = prio;
	maxPrioOutcome = o;
      }
    }
  }

  if (NULL != maxPrioP) *maxPrioP = maxPrio;
  if (NULL != secondBestPrioP) *secondBestPrioP = secondBestPrio;

  return maxPrioOutcome;
}

void FRTDP::updateInternal(MDPNode& cn)
{
  double lbVal, ubVal;
  double maxLBVal = -99e+20;
  double maxUBVal = -99e+20;
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
    maxUBVal = std::max(maxUBVal, ubVal);
  }
  cn.lbVal = maxLBVal;
  cn.ubVal = maxUBVal;
  //cn.ubVal = std::min(cn.ubVal, maxUBVal); (might be better if UB not uniformly improvable)

  int maxUBAction = getMaxUBAction(cn);
  double maxPrio;
  getMaxPrioOutcome(cn, maxUBAction, &maxPrio);
  cn.prio = maxPrio;

  numBackups++;
}

void FRTDP::trialRecurse(MDPNode& cn, double occ, double altPrio, int depth)
{
  // cached Q values must be up to date for subsequent calls
  update(cn);

  int maxUBAction = getMaxUBAction(cn);
  double maxPrio, secondBestPrio;
  int maxPrioOutcome = getMaxPrioOutcome(cn, maxUBAction,
					 &maxPrio, &secondBestPrio);

  // check for termination
  if (occ+maxPrio < std::max(altPrio - 1e-10, -1000.0)) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: depth=%d occ=%g maxPrio=%g altPrio=%g occ*maxPrio=%g (terminating)\n",
	   depth, occ, maxPrio, altPrio, maxPrio*occ);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif
    return;
  }

#if USE_DEBUG_PRINT
  printf("  trialRecurse: depth=%d a=%d o=%d ubVal=%g occ=%g altPrio=%g maxPrio=%g\n",
	 depth, maxUBAction, maxPrioOutcome, cn.ubVal, occ, altPrio, maxPrio);
  printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

  // recurse to successor
  double obsProb = cn.Q[maxUBAction].outcomes[maxPrioOutcome]->obsProb;
  double nextOcc = occ + log(obsProb);
  double nextAltPrio = std::max(altPrio, occ + secondBestPrio);
  trialRecurse(cn.getNextState(maxUBAction, maxPrioOutcome), nextOcc, nextAltPrio, depth+1);

  update(cn);
}

bool FRTDP::doTrial(MDPNode& cn, double pTarget)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  trialRecurse(cn, /* occ = */ 0, /* altPrio = */ -1000, 0);
  numTrials++;

  return (cn.ubVal - cn.lbVal < pTarget);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.1  2006/02/13 21:46:46  trey
 * initial check-in
 *
 *
 ***************************************************************************/
