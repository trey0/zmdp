/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-02-20 00:04:49 $
   
 @file    HDP.cc
 @brief   Implementation of Bonet and Geffner's HDP algorithm
          (as presented at IJCAI in 2003).

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
#include <stack>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "HDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

HDP::HDP(AbstractBound* _initUpperBound) :
  RTDPCore(_initUpperBound)
{}

void HDP::cacheQ(MDPNode& cn)
{
#if USE_HDP_LOWER_BOUND
  double lbVal;
  double maxLBVal = -99e+20;
#endif
  double ubVal;
  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
#if USE_HDP_LOWER_BOUND
    lbVal = 0;
#endif
    ubVal = 0;
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	double oprob = e->obsProb;
#if USE_HDP_LOWER_BOUND
	lbVal += oprob * sn.lbVal;
#endif
	ubVal += oprob * sn.ubVal;
      }
    }
#if USE_HDP_LOWER_BOUND
    Qa.lbVal = lbVal = Qa.immediateReward + problem->getDiscount() * lbVal;
    maxLBVal = std::max(maxLBVal, lbVal);
#endif

    ubVal = Qa.immediateReward + problem->getDiscount() * ubVal;
    Qa.ubVal = ubVal;
  }

#if USE_HDP_LOWER_BOUND
  cn.lbVal = maxLBVal;
#endif

  numBackups++;
}

// assumes correct Q values are already cached (using cacheQ)
double HDP::residual(MDPNode& cn)
{
  int maxUBAction = getMaxUBAction(cn);
  return fabs(cn.ubVal - cn.Q[maxUBAction].ubVal);
}

void HDP::updateInternal(MDPNode& cn)
{
  cacheQ(cn);
  int maxUBAction = getMaxUBAction(cn);
  cn.ubVal = cn.Q[maxUBAction].ubVal;
}

bool HDP::trialRecurse(MDPNode& cn, int depth)
{
#if USE_DEBUG_PRINT
  printf("  trialRecurse: depth=%d ubVal=%g\n",
	 depth, cn.ubVal);
  printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

  // base case
  if (cn.isSolved) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: solved node (terminating)\n");
#endif
    return false;
  }

  // check residual
  if (cn.isFringe()) expand(cn);
  cacheQ(cn);
  int maxUBAction = getMaxUBAction(cn);
  // FIX do not recalculate maxUBAction in residual()
  if (residual(cn) > targetPrecision) {
    cn.ubVal = cn.Q[maxUBAction].ubVal;

#if USE_DEBUG_PRINT
    printf("  trialRecurse: big residual (terminating)\n");
#endif
    return true;
  }

  // mark state as active
  visited.push(&cn);
  nodeStack.push(&cn);
  cn.idx = cn.low = index;
  index++;

  // recursive call
  bool flag = false;
  MDPQEntry& Qa = cn.Q[maxUBAction];
  //printf("    pre low=%d idx=%d\n", cn.low, cn.idx);
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      MDPNode& sn = *e->nextState;
      //printf("      a=%d o=%d sn=[%s] sn.idx=%d\n", maxUBAction, o, denseRep(sn.s).c_str(), sn.idx);
      if (RT_IDX_PLUS_INFINITY == sn.idx) {
	if (trialRecurse(sn, depth+1)) {
	  flag = true;
	}
	cn.low = std::min(cn.low, sn.low);
      } else if (nodeStack.contains(&sn)) {
	cn.low = std::min(cn.low, sn.low);
      }
    }
  }
  //printf("    post low=%d idx=%d\n", cn.low, cn.idx);

  // update if necessary
  if (flag) {
    update(cn);
    return true;
  }

  // try to label
  else if (cn.idx == cn.low) {
    printf("  marking %lu nodes solved\n", nodeStack.size());
    while (nodeStack.top() != &cn) {
      MDPNode& sn = *nodeStack.pop();
      sn.isSolved = true;
    }
    nodeStack.pop();
    cn.isSolved = true;
  }

  return flag;
}

bool HDP::doTrial(MDPNode& cn)
{
  if (cn.isSolved) {
    printf("-*- doTrial: root node is solved, terminating\n");
    return true;
  }

#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  index = 0;
  trialRecurse(cn, 0);
  // reset idx to +infinity for visited states
  while (!visited.empty()) {
    visited.top()->idx = RT_IDX_PLUS_INFINITY;
    visited.pop();
  }
  nodeStack.clear();
  RT_CLEAR_STD_STACK(visited);

  numTrials++;

  return false;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/19 18:33:36  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.1  2006/02/17 18:20:55  trey
 * initial check-in
 *
 *
 ***************************************************************************/
