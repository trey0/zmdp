/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-02-14 19:34:34 $
   
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
double LRTDP::residual(MDPNode& cn)
{
  int maxUBAction = getMaxUBAction(cn);
  return fabs(cn.ubVal - cn.Q[maxUBAction].ubVal);
}

bool LRTDP::checkSolved(MDPNode& cn, double pTarget)
{
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
    a = getMaxUBAction(n);
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
      updateInternal(n);
    }
  }

#if USE_DEBUG_PRINT
  printf("  checkSolved: s=[%s] numClosedStates=%d rv=%d\n",
	 sparseRep(cn.s).c_str(), numClosedStates, rv);
#endif
  return rv;
}

void LRTDP::updateInternal(MDPNode& cn)
{
  cacheQ(cn);
  int maxUBAction = getMaxUBAction(cn);
  cn.ubVal = cn.Q[maxUBAction].ubVal;
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

  // cached Q values must be up to date for subsequent calls
  update(cn);

  int maxUBAction = getMaxUBAction(cn);
  int simulatedOutcome = getSimulatedOutcome(cn, maxUBAction);

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

bool LRTDP::doTrial(MDPNode& cn, double pTarget)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  trialRecurse(cn, pTarget, 0);
  numTrials++;

  return cn.isSolved;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/02/13 20:20:33  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.2  2006/02/13 19:52:44  trey
 * corrected a fatal bug in updateInternal()
 *
 * Revision 1.1  2006/02/13 19:09:24  trey
 * initial check-in
 *
 *
 ***************************************************************************/
