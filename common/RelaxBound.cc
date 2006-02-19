/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-02-19 18:30:50 $
   
 @file    RelaxBound.cc
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "MDP.h"
#include "RelaxBound.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define OBS_IS_ZERO_EPS (1e-10)

namespace zmdp {

RelaxBound::RelaxBound(const MDP* _problem) :
  problem(_problem),
  root(NULL)
{}

void RelaxBound::setup(double targetPrecision)
{
  initLowerBound = problem->newLowerBound();
  initLowerBound->initialize(targetPrecision);
  initUpperBound = problem->newUpperBound();
  initUpperBound->initialize(targetPrecision);

  lookup = new MDPHash();
  root = getNode(problem->getInitialState());
}

MDPNode* RelaxBound::getNode(const state_vector& s)
{
  string hs = hashable(s);
  MDPHash::iterator pr = lookup->find(hs);
  if (lookup->end() == pr) {
    // create a new fringe node
    MDPNode& cn = *(new MDPNode);
    cn.s = s;
    cn.lbVal = initLowerBound->getValue(s);
    cn.ubVal = initUpperBound->getValue(s);
    (*lookup)[hs] = &cn;
    return &cn;
  } else {
    // return existing node
    return pr->second;
  }
}

void RelaxBound::expand(MDPNode& cn)
{
  // set up successors for this fringe node (possibly creating new fringe nodes)
  outcome_prob_vector opv;
  state_vector sp;
  cn.Q.resize(problem->getNumActions());
  FOR (a, problem->getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    Qa.immediateReward = problem->getReward(cn.s, a);
    problem->getOutcomeProbVector(opv, cn.s, a);
    Qa.outcomes.resize(opv.size());
    FOR (o, opv.size()) {
      double oprob = opv(o);
      if (oprob > OBS_IS_ZERO_EPS) {
	MDPEdge* e = new MDPEdge();
        Qa.outcomes[o] = e;
        e->obsProb = oprob;
        e->nextState = getNode(problem->getNextState(sp, cn.s, a, o));
      } else {
        Qa.outcomes[o] = NULL;
      }
    }
  }
}

void RelaxBound::updateInternal(MDPNode& cn)
{
  cn.lbVal = -99e+20;
  cn.ubVal = -99e+20;
  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    Qa.lbVal = -99e+20;
    Qa.ubVal = -99e+20;
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	Qa.lbVal = std::max(Qa.lbVal, sn.lbVal);
	Qa.ubVal = std::max(Qa.ubVal, sn.ubVal);
      }
    }
    Qa.lbVal = Qa.immediateReward + problem->getDiscount() * Qa.lbVal;
    Qa.ubVal = Qa.immediateReward + problem->getDiscount() * Qa.ubVal;

    cn.lbVal = std::max(cn.lbVal, Qa.lbVal);
    cn.ubVal = std::max(cn.ubVal, Qa.ubVal);
  }
}

void RelaxBound::update(MDPNode& cn)
{
  if (cn.isFringe()) {
    expand(cn);
  }
  updateInternal(cn);
}

int RelaxBound::getMaxUBAction(MDPNode& cn, double* maxUBValP,
			       double* secondBestUBValP) const
{
  double maxUBVal = -99e+20;
  double secondBestUBVal = -99e+20;
  int maxUBAction = -1;

  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];

    if (Qa.ubVal > maxUBVal) {
      secondBestUBVal = maxUBVal;
      maxUBVal = Qa.ubVal;
      maxUBAction = a;
    } else if (Qa.ubVal > secondBestUBVal) {
      secondBestUBVal = Qa.ubVal;
    }
  }

  if (NULL != maxUBValP) *maxUBValP = maxUBVal;
  if (NULL != secondBestUBValP) *secondBestUBValP = secondBestUBVal;
  return maxUBAction;
}

void RelaxBound::trialRecurse(MDPNode& cn, double costSoFar, double altActionPrio, int depth)
{
  // update to ensure cached values in cn.Q are correct
  update(cn);
  double maxUBVal, secondBestUBVal;
  int maxUBAction = getMaxUBAction(cn, &maxUBVal, &secondBestUBVal);

  // check for termination
  double actionPrio = costSoFar + cn.ubVal;
  if (altActionPrio > actionPrio) {
#if USE_DEBUG_PRINT
    printf("  RB trialRecurse: depth=%d [%g .. %g] costSoFar=%g altActionPrio=%g actionPrio=%g (terminating)\n",
	   depth, cn.lbVal, cn.ubVal, costSoFar, altActionPrio, actionPrio);
#endif
    return;
  }

  // select best possible outcome
  MDPQEntry& Qbest = cn.Q[maxUBAction];
  double bestVal = -99e+20;
  int bestOutcome = -1;
  FOR (o, Qbest.getNumOutcomes()) {
    MDPEdge* e = Qbest.outcomes[o];
    if (NULL != e) {
      MDPNode& sn = *e->nextState;
      if (sn.ubVal > bestVal) {
	bestVal = sn.ubVal;
	bestOutcome = o;
      }
    }
  }

#if USE_DEBUG_PRINT
  printf("  RB trialRecurse: depth=%d a=%d o=%d [%g .. %g] altActionPrio=%g actionPrio=%g \n",
	 depth, maxUBAction, bestOutcome, cn.lbVal, cn.ubVal, altActionPrio, actionPrio);
  printf("  RB trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
#endif

  // recurse to successor
  double nextCostSoFar = costSoFar + cn.Q[maxUBAction].immediateReward;
  double nextAltActionPrio = std::max(altActionPrio, secondBestUBVal + costSoFar);
  trialRecurse(cn.getNextState(maxUBAction, bestOutcome),
	       nextCostSoFar, nextAltActionPrio, depth+1);

  update(cn);
}

void RelaxBound::doTrial(MDPNode& cn, double pTarget)
{
  trialRecurse(cn,
	       /* costSoFar = */ 0,
	       /* altActionPrio = */ -99e+20,
	       /* depth = */ 0);
}

void RelaxBound::initialize(double targetPrecision)
{
#if USE_DEBUG_PRINT
  timeval startTime = getTime();
#endif

  setup(targetPrecision);

  int trialNum = 1;
  double width;
  while (1) {
    width = root->ubVal - root->lbVal;
#if USE_DEBUG_PRINT
    printf("-*- RB initialize trial=%d width=%g target=%g\n",
	   trialNum, width, targetPrecision);
#endif
    if (width < targetPrecision) {
#if USE_DEBUG_PRINT
    printf("RB initialize width=%g target=%g terminating\n",
	   width, targetPrecision);
#endif
      break;
    }
    doTrial(*root, width * problem->getDiscount());
    trialNum++;
  }

#if USE_DEBUG_PRINT
  double elapsedTime = timevalToSeconds(getTime() - startTime);
  printf("--> RB initialization completed after %g seconds\n",
	 elapsedTime);
#endif
}

double RelaxBound::getValue(const state_vector& s) const
{
  MDPHash::iterator pr = lookup->find(hashable(s));
  if (lookup->end() == pr) {
    return initUpperBound->getValue(s);
  } else {
    return pr->second->ubVal;
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/02/17 21:09:08  trey
 * fixed stack overflow on undiscounted problems (unfortunately, initialization is slower now)
 *
 * Revision 1.4  2006/02/14 19:30:28  trey
 * added targetPrecision argument to initialize()
 *
 * Revision 1.3  2006/02/10 19:31:44  trey
 * tweaked debug output
 *
 * Revision 1.2  2006/02/09 21:56:27  trey
 * added minor efficiency enhancement to calculate maxUBAction during update
 *
 * Revision 1.1  2006/02/08 19:21:44  trey
 * initial check-in
 *
 *
 ***************************************************************************/
