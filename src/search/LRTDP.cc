/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.14 $  $Author: trey $  $Date: 2006-10-30 20:00:15 $
   
 @file    LRTDP.cc
 @brief   Implementation of Bonet and Geffner's LRTDP algorithm.

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

/**********************************************************************
  This is my implementation of the LRTDP algorithm, based on the paper

  "Labeled RTDP: Improving the Convergence of Real Time Dynamic Programming."
    B. Bonet and H. Geffner. In Proc. of ICAPS, 2003.

  Inevitably they could not include all the details of the algorithm in
  their paper, so it is possible that my implementation differs from
  theirs in important ways.  They have not signed off on this
  implementation: use at your own risk.  (And please inform me if you
  find any errors!)

  -Trey Smith, Feb. 2006
 **********************************************************************/

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

LRTDP::LRTDP(void)
{}

void LRTDP::getNodeHandler(MDPNode& cn)
{
  LRTDPExtraNodeData* searchData = new LRTDPExtraNodeData;
  cn.searchData = searchData;
  searchData->isSolved = cn.isTerminal;
}

void LRTDP::staticGetNodeHandler(MDPNode& s, void* handlerData)
{
  LRTDP* x = (LRTDP *) handlerData;
  x->getNodeHandler(s);
}

bool& LRTDP::getIsSolved(const MDPNode& cn)
{
  return ((LRTDPExtraNodeData *) cn.searchData)->isSolved;
}

void LRTDP::cacheQ(MDPNode& cn)
{
  double oldUBVal = cn.ubVal;
  // bounds->update() changes both Q values and cn.ubVal
  bounds->update(cn, NULL);
  trackBackup(cn);
  // keep the changes to Q but undo the change to cn.ubVal
  cn.ubVal = oldUBVal;
}

// assumes correct Q values are already cached (using cacheQ)
double LRTDP::residual(MDPNode& cn)
{
  int maxUBAction = bounds->getMaxUBAction(cn);
  return fabs(cn.ubVal - cn.Q[maxUBAction].ubVal);
}

bool LRTDP::checkSolved(MDPNode& cn)
{
  bool rv = true;
  NodeStack open, closed;
  int a;
  
  if (!getIsSolved(cn)) open.push(&cn);
  while (!open.empty()) {
    MDPNode& n = *open.pop();
    closed.push(&n);

    cacheQ(n);
    if (residual(n) > targetPrecision) {
      rv = false;
      continue;
    }
    a = bounds->getMaxUBAction(n);
    MDPQEntry& Qa = n.Q[a];
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	if (!getIsSolved(sn)
	    && !open.contains(&sn)
	    && !closed.contains(&sn)) {
	  open.push(&sn);
	}
      }
    }
  }

  int numClosedStates = -1;
  if (zmdpDebugLevelG >= 1) {
    numClosedStates = closed.size();
  }
  if (rv) {
    // label relevant states
    while (!closed.empty()) {
      MDPNode& n = *closed.pop();
      getIsSolved(n) = true;
    }
  } else {
    // update states with residuals and ancestors
    while (!closed.empty()) {
      MDPNode& n = *closed.pop();
      updateInternal(n);
    }
  }

  if (zmdpDebugLevelG >= 1) {
    printf("  checkSolved: s=[%s] numClosedStates=%d rv=%d\n",
	   sparseRep(cn.s).c_str(), numClosedStates, rv);
  }
  return rv;
}

void LRTDP::updateInternal(MDPNode& cn)
{
  cacheQ(cn);
  int maxUBAction = bounds->getMaxUBAction(cn);
  cn.ubVal = cn.Q[maxUBAction].ubVal;
}

bool LRTDP::trialRecurse(MDPNode& cn, int depth)
{
  // check for termination
  if (getIsSolved(cn)) {
    if (zmdpDebugLevelG >= 1) {
      printf("  trialRecurse: depth=%d ubVal=%g solved node (terminating)\n",
	     depth, cn.ubVal);
    }
    return true;
  }

  // cached Q values must be up to date for subsequent calls
  int maxUBAction;
  bounds->update(cn, &maxUBAction);
  trackBackup(cn);

  int simulatedOutcome = bounds->getSimulatedOutcome(cn, maxUBAction);

  if (zmdpDebugLevelG >= 1) {
    printf("  trialRecurse: depth=%d a=%d o=%d ubVal=%g\n",
	   depth, maxUBAction, simulatedOutcome, cn.ubVal);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
  }

  // recurse to successor
  bool solvedAtNextDepth =
    trialRecurse(cn.getNextState(maxUBAction, simulatedOutcome), depth+1);

  if (solvedAtNextDepth) {
    return checkSolved(cn);
  } else {
    return false;
  }
}

bool LRTDP::doTrial(MDPNode& cn)
{
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials+1));
  }

  trialRecurse(cn, 0);
  numTrials++;

  return getIsSolved(cn);
}

void LRTDP::derivedClassInit(void)
{
  bounds->addGetNodeHandler(&LRTDP::staticGetNodeHandler, this);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.13  2006/10/24 02:37:05  trey
 * updated for modified bounds interfaces
 *
 * Revision 1.12  2006/10/19 19:31:16  trey
 * added support for backup logging
 *
 * Revision 1.11  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.10  2006/04/07 19:41:45  trey
 * removed initLowerBound, initUpperBound arguments to constructor
 *
 * Revision 1.9  2006/04/06 04:14:50  trey
 * changed how bounds are initialized
 *
 * Revision 1.8  2006/04/04 17:24:52  trey
 * modified to use IncrementalBounds methods
 *
 * Revision 1.7  2006/02/27 20:12:36  trey
 * cleaned up meta-information in header
 *
 * Revision 1.6  2006/02/19 18:35:09  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.5  2006/02/17 18:34:34  trey
 * renamed LStack -> NodeStack and moved it from LRTDP to RTDPCore so it could be shared with HDP
 *
 * Revision 1.4  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
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
