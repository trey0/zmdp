/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-04-04 17:23:58 $
   
 @file    HDP.cc
 @brief   Implementation of Bonet and Geffner's HDP algorithm.

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

/**********************************************************************
  This is my implementation of the HDP algorithm, based on the paper

    "Faster heuristic Search Algorithms for Planning with
       Uncertainty and Full Feedback."
    B. Bonet and H. Geffner. In Proc. of IJCAI, 2003.

  Inevitably they could not include all the details of the algorithm in
  their paper, so it is possible that my implementation differs from
  theirs in important ways.  They have not signed off on this
  implementation: use at your own risk.  (And please inform me if you
  find any errors!)

  This code also implements my variant HDP+L algorithm [not yet
  published] when the compile-time flag '-DUSE_HDP_LOWER_BOUND=1' is
  set.  In addition to the usual upper bound, HDP+L keeps a lower bound
  and uses that to generate the output policy.  Empirically, this
  improves anytime performance when the upper and lower bounds have not
  yet converged.

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
#include "HDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

HDP::HDP(AbstractBound* _initUpperBound) :
  RTDPCore(_initUpperBound)
{}

void HDP::getNodeHandler(MDPNode& cn)
{
  HDPExtraNodeData* searchData = new HDPExtraNodeData;
  cn.searchData = searchData;
  searchData->isSolved = cn.isTerminal;
  searchData->idx = RT_IDX_PLUS_INFINITY;
}

void HDP::staticGetNodeHandler(MDPNode& s, void* handlerData)
{
  HDP* x = (HDP *) handlerData;
  x->getNodeHandler(s);
}

bool& HDP::getIsSolved(const MDPNode& cn)
{
  return ((HDPExtraNodeData *) cn.searchData)->isSolved;
}

int& HDP::getLow(const MDPNode& cn)
{
  return ((HDPExtraNodeData *) cn.searchData)->low;
}

int& HDP::getIdx(const MDPNode& cn)
{
  return ((HDPExtraNodeData *) cn.searchData)->idx;
}

void HDP::cacheQ(MDPNode& cn)
{
  double oldUBVal = cn.ubVal;
  // bounds->update() changes both Q values and cn.ubVal
  bounds->update(cn, NULL);
  // keep the changes to Q but undo the change to cn.ubVal
  cn.ubVal = oldUBVal;
}

// assumes correct Q values are already cached (using cacheQ)
double HDP::residual(MDPNode& cn)
{
  int maxUBAction = bounds->getMaxUBAction(cn);
  return fabs(cn.ubVal - cn.Q[maxUBAction].ubVal);
}

void HDP::updateInternal(MDPNode& cn)
{
  cacheQ(cn);
  int maxUBAction = bounds->getMaxUBAction(cn);
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
  if (getIsSolved(cn)) {
#if USE_DEBUG_PRINT
    printf("  trialRecurse: solved node (terminating)\n");
#endif
    return false;
  }

  // check residual
  cacheQ(cn);
  int maxUBAction = bounds->getMaxUBAction(cn);
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
  getIdx(cn) = getLow(cn) = index;
  index++;

  // recursive call
  bool flag = false;
  MDPQEntry& Qa = cn.Q[maxUBAction];
  //printf("    pre low=%d idx=%d\n", getLow(cn), getIdx(cn));
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      MDPNode& sn = *e->nextState;
      //printf("      a=%d o=%d sn=[%s] sn.idx=%d\n", maxUBAction, o, denseRep(sn.s).c_str(), getIdx(sn));
      if (RT_IDX_PLUS_INFINITY == getIdx(sn)) {
	if (trialRecurse(sn, depth+1)) {
	  flag = true;
	}
	getLow(cn) = std::min(getLow(cn), getLow(sn));
      } else if (nodeStack.contains(&sn)) {
	getLow(cn) = std::min(getLow(cn), getLow(sn));
      }
    }
  }
  //printf("    post low=%d idx=%d\n", getLow(cn), getIdx(cn));

  // update if necessary
  if (flag) {
    bounds->update(cn, NULL);
    return true;
  }

  // try to label
  else if (getIdx(cn) == getLow(cn)) {
    printf("  marking %u nodes solved\n", nodeStack.size());
    while (nodeStack.top() != &cn) {
      MDPNode& sn = *nodeStack.pop();
      getIsSolved(sn) = true;
    }
    nodeStack.pop();
    getIsSolved(cn) = true;
  }

  return flag;
}

bool HDP::doTrial(MDPNode& cn)
{
  if (getIsSolved(cn)) {
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
    getIdx(*visited.top()) = RT_IDX_PLUS_INFINITY;
    visited.pop();
  }
  nodeStack.clear();
  RT_CLEAR_STD_STACK(visited);

  numTrials++;

  return false;
}

void HDP::derivedClassInit(void)
{
  bounds->setGetNodeHandler(&HDP::staticGetNodeHandler, this);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/03/17 20:06:13  trey
 * fixed compile warning
 *
 * Revision 1.4  2006/02/27 20:12:36  trey
 * cleaned up meta-information in header
 *
 * Revision 1.3  2006/02/20 00:04:49  trey
 * added optional lower bound use
 *
 * Revision 1.2  2006/02/19 18:33:36  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.1  2006/02/17 18:20:55  trey
 * initial check-in
 *
 *
 ***************************************************************************/
