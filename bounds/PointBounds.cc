/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-10-18 18:05:02 $
   
 @file    PointBounds.cc
 @brief   No brief

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
#include "AbstractBound.h"
#include "PointBounds.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

PointBounds::PointBounds(void)
{}

void PointBounds::updateValuesUB(MDPNode& cn, int* maxUBActionP)
{
  double ubVal;
  double maxUBVal = -99e+20;
  int maxUBAction = -1;

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

    if (ubVal > maxUBVal) {
      maxUBVal = ubVal;
      maxUBAction = a;
    }
  }
#if 1
  cn.ubVal = std::min(cn.ubVal, maxUBVal);
#else
  cn.ubVal = maxUBVal;
#endif

  if (NULL != maxUBActionP) *maxUBActionP = maxUBAction;
}

void PointBounds::updateValuesBoth(MDPNode& cn, int* maxUBActionP)
{
  double lbVal, ubVal;
  double maxLBVal = -99e+20;
  double maxUBVal = -99e+20;
  int maxUBAction = -1;

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
    lbVal = Qa.immediateReward + problem->getDiscount() * lbVal;
    ubVal = Qa.immediateReward + problem->getDiscount() * ubVal;
    Qa.lbVal = lbVal;
    Qa.ubVal = ubVal;

    maxLBVal = std::max(maxLBVal, lbVal);
    if (ubVal > maxUBVal) {
      maxUBVal = ubVal;
      maxUBAction = a;
    }
  }
#if 1
  cn.lbVal = std::max(cn.lbVal, maxLBVal);
  cn.ubVal = std::min(cn.ubVal, maxUBVal);
#else
  cn.lbVal = maxLBVal;
  cn.ubVal = maxUBVal;
#endif

  if (NULL != maxUBActionP) *maxUBActionP = maxUBAction;
}

void PointBounds::setBounds(AbstractBound* _initLowerBound,
			    AbstractBound* _initUpperBound,
			    bool _forceUpperBoundActionSelection)
{
  initLowerBound = _initLowerBound;
  initUpperBound = _initUpperBound;
  forceUpperBoundActionSelection = _forceUpperBoundActionSelection;
}

void PointBounds::initialize(const MDP* _problem,
			     const ZMDPConfig& config)
{
  problem = _problem;
  targetPrecision = config.getDouble("terminateRegretBound");

  if (NULL != initLowerBound) {
    initLowerBound->initialize(targetPrecision);
  }
  initUpperBound->initialize(targetPrecision);

  lookup = new MDPHash();
  root = NULL;

  numStatesTouched = 0;
  numStatesExpanded = 0;
  numBackups = 0;

  getNodeHandler = NULL;
  handlerData = NULL;
}

MDPNode* PointBounds::getRootNode(void)
{
  if (NULL == root) {
    root = getNode(problem->getInitialState());
  }
  return root;
}

MDPNode* PointBounds::getNode(const state_vector& s)
{
  string hs = hashable(s);
  MDPHash::iterator pr = lookup->find(hs);
  if (lookup->end() == pr) {
    // create a new fringe node
    MDPNode& cn = *(new MDPNode);
    cn.s = s;
    cn.isTerminal = problem->getIsTerminalState(s);
    if (cn.isTerminal) {
      cn.ubVal = 0;
    } else {
      cn.ubVal = initUpperBound->getValue(s);
    }
    if (NULL == initLowerBound) {
      cn.lbVal = -1; // n/a
    } else {
      if (cn.isTerminal) {
	cn.lbVal = 0;
      } else {
	cn.lbVal = initLowerBound->getValue(s);
      }
    }
    cn.searchData = NULL;
    cn.boundsData = NULL;
    (*lookup)[hs] = &cn;

    if (NULL != getNodeHandler) {
      (*getNodeHandler)(cn, handlerData);
    }

    numStatesTouched++;
    return &cn;
  } else {
    // return existing node
    return pr->second;
  }
}

void PointBounds::expand(MDPNode& cn)
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
  numStatesExpanded++;
}

void PointBounds::update(MDPNode& cn, int* maxUBActionP)
{
  if (cn.isFringe()) {
    expand(cn);
  }
  if (NULL == initLowerBound) {
    updateValuesUB(cn, maxUBActionP);
  } else {
    updateValuesBoth(cn, maxUBActionP);
  }

  numBackups++;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int PointBounds::chooseAction(const state_vector& s) const
{
  outcome_prob_vector opv;
  state_vector sp;
  int bestAction = -1;

  // if LB is available, use it to choose the action
  if (!forceUpperBoundActionSelection && NULL != initLowerBound) {
    double lbVal;
    double maxVal = -99e+20;
    double minVal = 99e+20;
    FOR (a, problem->getNumActions()) {
      problem->getOutcomeProbVector(opv, s, a);
      lbVal = 0;
      FOR (o, opv.size()) {
	if (opv(o) > OBS_IS_ZERO_EPS) {
	  ValueInterval intv = getValueAt(problem->getNextState(sp, s, a, o));
	  lbVal += opv(o) * intv.l;
	}
      }
      lbVal = problem->getReward(s,a) + problem->getDiscount() * lbVal;
      if (lbVal > maxVal) {
	maxVal = lbVal;
	bestAction = a;
      }
      if (lbVal < minVal) {
	minVal = lbVal;
      }
    }

    // but only return best LB action if all choices are not tied
    if (maxVal - minVal > 1e-10) {
      return bestAction;
    }
  }

  // fall back to UB
  double ubVal;
  double maxVal = -99e+20;
  FOR (a, problem->getNumActions()) {
    problem->getOutcomeProbVector(opv, s, a);
    ubVal = 0;
    FOR (o, opv.size()) {
      if (opv(o) > OBS_IS_ZERO_EPS) {
	ValueInterval intv = getValueAt(problem->getNextState(sp, s, a, o));
	ubVal += opv(o) * intv.u;
      }
    }
    ubVal = problem->getReward(s,a) + problem->getDiscount() * ubVal;
    if (ubVal > maxVal) {
      maxVal = ubVal;
      bestAction = a;
    }
  }
  return bestAction;
}

ValueInterval PointBounds::getValueAt(const state_vector& s) const
{
  typeof(lookup->begin()) pr = lookup->find(hashable(s));
  if (lookup->end() == pr) {
    return ValueInterval((NULL == initLowerBound) ? -1 : initLowerBound->getValue(s),
			 initUpperBound->getValue(s));
  } else {
    const MDPNode& cn = *pr->second;
    return ValueInterval(cn.lbVal, cn.ubVal);
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.4  2006/04/10 20:25:08  trey
 * added forceUpperBoundActionSelection
 *
 * Revision 1.3  2006/04/06 20:33:51  trey
 * moved setGetNodeHandler() implementation from PointBounds -> IncrementalBounds
 *
 * Revision 1.2  2006/04/05 21:34:40  trey
 * changed initialization to match new IncrementalBounds API
 *
 * Revision 1.1  2006/04/04 17:22:51  trey
 * initial check-in
 *
 *
 ***************************************************************************/
