/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-02-13 19:09:24 $
   
 @file    LRTDP.cc
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
#include <stack>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "LRTDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define OBS_IS_ZERO_EPS (1e-10)

namespace zmdp {

struct LStack {
  std::stack<MDPNode*> data;
  EXT_NAMESPACE::hash_map<MDPNode*, bool> lookup;
  
  void push(MDPNode* n) {
    data.push(n);
    lookup[n] = true;
  }
  MDPNode* pop(void) {
    MDPNode* n = data.top();
    data.pop();
    lookup.erase(n);
    return n;
  }
  bool empty(void) const {
    return data.empty();
  }
  size_t size(void) const {
    return data.size();
  }
  bool contains(MDPNode* n) const {
    return (lookup.end() != lookup.find(n));
  }
};

LRTDP::LRTDP(AbstractBound* _initUpperBound) :
  RTDPCore(_initUpperBound)
{}

void LRTDP::cacheQ(MDPNode& cn)
{
  double ubVal;
  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    ubVal = 0;
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	double oprob = e->obsProb;
	ubVal += oprob * sn.ubVal;
      }
    }
    ubVal = Qa.immediateReward + problem->getDiscount() * ubVal;
    Qa.ubVal = ubVal;
  }

  numBackups++;
}

// assumes correct Q values are already cached (using cacheQ)
int LRTDP::greedyAction(MDPNode& cn)
{
  double bestVal = -99e+20;
  int bestAction = -1;
  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    if (Qa.ubVal > bestVal) {
      bestVal = Qa.ubVal;
      bestAction = a;
    }
  }
  return bestAction;
}

// assumes correct Q values are already cached (using cacheQ)
double LRTDP::residual(MDPNode& cn)
{
  int maxUBAction = greedyAction(cn);
  return fabs(cn.ubVal - cn.Q[maxUBAction].ubVal);
}

bool LRTDP::checkSolved(MDPNode& cn, double pTarget)
{
#if USE_DEBUG_PRINT
  printf("checkSolved: s=[%s] pTarget=%g\n",
	 denseRep(cn.s).c_str(), pTarget);
#endif

  bool rv = true;
  LStack open, closed;
  int a;
  
  if (!cn.isSolved) open.push(&cn);
  while (!open.empty()) {
    MDPNode& n = *open.pop();
    closed.push(&n);

    if (n.isFringe()) {
      expand(n);
    }
    cacheQ(n);
    if (residual(n) > pTarget) {
      rv = false;
      continue;
    }
    a = greedyAction(n);
    MDPQEntry& Qa = n.Q[a];
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	if (!sn.isSolved
	    && !open.contains(&sn)
	    && !closed.contains(&sn)) {
	  open.push(&sn);
	}
      }
    }
  }

  int numClosedStates = closed.size();
  if (rv) {
    // label relevant states
    while (!closed.empty()) {
      MDPNode& n = *closed.pop();
      n.isSolved = true;
    }
  } else {
    // update states with residuals and ancestors
    while (!closed.empty()) {
      MDPNode& n = *closed.pop();
      //n.ubVal = n.Q[greedyAction(n)].ubVal;
      updateInternal(n, NULL);
    }
  }

#if USE_DEBUG_PRINT
  printf("  checkSolved: numClosedStates=%d rv=%d\n",
	 numClosedStates, rv);
#endif
  return rv;
}

void LRTDP::updateInternal(MDPNode& cn, int* maxUBActionP)
{
  cacheQ(cn);
  if (NULL != maxUBActionP) *maxUBActionP = greedyAction(cn);
}

bool LRTDP::trialRecurse(MDPNode& cn, double pTarget, int depth)
{
  // check for termination
  if (cn.isSolved) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: depth=%d ubVal=%g solved node (terminating)\n",
	   depth, cn.ubVal);
#endif
    return true;
  }

  int maxUBAction;
  update(cn, &maxUBAction);

  // simulate outcome
  double r = unit_rand();
  int simulatedOutcome = 0;
  MDPQEntry& Qbest = cn.Q[maxUBAction];
  FOR (o, Qbest.getNumOutcomes()) {
    MDPEdge* e = Qbest.outcomes[o];
    if (NULL != e) {
      r -= e->obsProb;
      if (r <= 0) {
	simulatedOutcome = o;
	break;
      }
    }
  }

#if USE_DEBUG_PRINT
  printf("  trialRecurse: depth=%d a=%d o=%d ubVal=%g\n",
	 depth, maxUBAction, simulatedOutcome, cn.ubVal);
  printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

  // recurse to successor
  bool solvedAtNextDepth =
    trialRecurse(cn.getNextState(maxUBAction, simulatedOutcome), pTarget, depth+1);

  if (solvedAtNextDepth) {
    return checkSolved(cn, pTarget);
  } else {
    return false;
  }
}

void LRTDP::doTrial(MDPNode& cn, double pTarget)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  trialRecurse(cn, pTarget, 0);
  numTrials++;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
