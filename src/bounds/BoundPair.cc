/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.9 $  $Author: trey $  $Date: 2007-04-03 06:07:33 $
   
 @file    BoundPair.cc
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
#include "BoundPair.h"
#include "MaxPlanesLowerBound.h"

#define BP_INITIALIZATION_PRECISION_FACTOR (1e-2)

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

BoundPair::BoundPair(bool _maintainLowerBound,
		     bool _maintainUpperBound,
		     bool _useUpperBoundRunTimeActionSelection,
		     bool _dualPointBounds) :
  problem(NULL),
  config(NULL),
  lowerBound(NULL),
  upperBound(NULL),
  maintainLowerBound(_maintainLowerBound),
  maintainUpperBound(_maintainUpperBound),
  useUpperBoundRunTimeActionSelection(_useUpperBoundRunTimeActionSelection),
  dualPointBounds(_dualPointBounds)
{}

void BoundPair::updateDualPointBounds(MDPNode& cn, int* maxUBActionP)
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

void BoundPair::initialize(MDP* _problem,
			   const ZMDPConfig* _config)
{
  problem = _problem;
  config = _config;
  targetPrecision = config->getDouble("terminateRegretBound");

  if (maintainLowerBound) {
    lowerBound->initialize(BP_INITIALIZATION_PRECISION_FACTOR * targetPrecision);
  }
  if (maintainUpperBound) {
    upperBound->initialize(BP_INITIALIZATION_PRECISION_FACTOR * targetPrecision);
  }

  lookup = new MDPHash();
  root = NULL;

  numStatesTouched = 0;
  numStatesExpanded = 0;
  numBackups = 0;
}

MDPNode* BoundPair::getRootNode(void)
{
  if (NULL == root) {
    root = getNode(problem->getInitialState());
  }
  return root;
}

MDPNode* BoundPair::getNode(const state_vector& s)
{
  string hs = hashable(s);
  MDPHash::iterator pr = lookup->find(hs);
  if (lookup->end() == pr) {
    // create a new fringe node
    MDPNode& cn = *(new MDPNode);
    cn.s = s;
    cn.isTerminal = problem->getIsTerminalState(s);
    cn.searchData = NULL;
    cn.boundsData = NULL;

    if (maintainUpperBound) {
      upperBound->initNodeBound(cn);
    } else {
      cn.ubVal = -1; // n/a
    }
    if (maintainLowerBound) {
      lowerBound->initNodeBound(cn);
    } else {
      cn.lbVal = -1; // n/a
    }
    (*lookup)[hs] = &cn;

    FOR_EACH (hstructP, getNodeHandlers) {
      (*hstructP->h)(cn, hstructP->hdata);
    }

    numStatesTouched++;
    return &cn;
  } else {
    // return existing node
    return pr->second;
  }
}

MDPNode* BoundPair::getNodeOrNull(const state_vector& s) const
{
  typeof(lookup->begin()) pr = lookup->find(hashable(s));
  if (lookup->end() == pr) {
    return NULL;
  } else {
    return pr->second;
  }
}

void BoundPair::expand(MDPNode& cn)
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
    Qa.ubVal = BP_QVAL_UNDEFINED;
  }
  numStatesExpanded++;
}

void BoundPair::update(MDPNode& cn, int* maxUBActionP)
{
  if (cn.isFringe()) {
    expand(cn);
  }
  if (dualPointBounds) {
    // updateDualPointBounds is an optimized procedure that only works if both lower
    // and upper bound are point bounds
    updateDualPointBounds(cn, maxUBActionP);
  } else {
    // otherwise fall back to whatever separate update procedures are
    // defined for the two bounds
    if (maintainLowerBound) {
      lowerBound->update(cn);
    }
    if (maintainUpperBound) {
      upperBound->update(cn, maxUBActionP);
    }
  }

  numBackups++;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int BoundPair::chooseAction(const state_vector& s) const
{
  outcome_prob_vector opv;
  state_vector sp;
  int bestAction = -1;

  if (!useUpperBoundRunTimeActionSelection) {
#if 0
    // direct policy performs significantly worse than lookahead-1 policy,
    // so let's not use it even though it chooses actions more quickly
    bestAction = lowerBound->chooseAction(s);
    if (-1 != bestAction) {
      return bestAction;
    }
#endif

    double lbVal;
    double maxVal = -99e+20;
    double minVal = 99e+20;
    FOR (a, problem->getNumActions()) {
      problem->getOutcomeProbVector(opv, s, a);
      lbVal = 0;
      FOR (o, opv.size()) {
	if (opv(o) > OBS_IS_ZERO_EPS) {
	  problem->getNextState(sp, s, a, o);
	  lbVal += opv(o) * lowerBound->getValue(sp, getNodeOrNull(sp));
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
    
    return bestAction;
  }

  // fall back to UB
  double ubVal;
  double maxVal = -99e+20;
  FOR (a, problem->getNumActions()) {
    problem->getOutcomeProbVector(opv, s, a);
    ubVal = 0;
    FOR (o, opv.size()) {
      if (opv(o) > OBS_IS_ZERO_EPS) {
	problem->getNextState(sp, s, a, o);
	ubVal += opv(o) * upperBound->getValue(sp, getNodeOrNull(sp));
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

ValueInterval BoundPair::getValueAt(const state_vector& s) const
{
  MDPNode* cn = getNodeOrNull(s);
  return ValueInterval(maintainLowerBound ? lowerBound->getValue(s,cn) : -1,
		       maintainUpperBound ? upperBound->getValue(s,cn) : -1);
}

ValueInterval BoundPair::getQValue(const state_vector& s, int a) const
{
  state_vector sp;
  MDPNode* spn;
  double lbVal, ubVal;
  outcome_prob_vector opv;

  lbVal = 0;
  ubVal = 0;
  problem->getOutcomeProbVector(opv, s, a);
  FOR (o, opv.size()) {
    if (opv(o) > OBS_IS_ZERO_EPS) {
      problem->getNextState(sp, s, a, o);
      spn = getNodeOrNull(sp);
      if (maintainLowerBound) {
	lbVal += opv(o) * lowerBound->getValue(sp, spn);
      }
      if (maintainUpperBound) {
	ubVal += opv(o) * upperBound->getValue(sp, spn);
      }
    }
  }
  lbVal = problem->getReward(s,a) + problem->getDiscount() * lbVal;
  ubVal = problem->getReward(s,a) + problem->getDiscount() * ubVal;

  return ValueInterval(maintainLowerBound ? lbVal : -1,
		       maintainUpperBound ? ubVal : -1);
}

void BoundPair::writePolicy(const std::string& outFileName, bool canModifyBounds)
{
  MaxPlanesLowerBound* mlb = (MaxPlanesLowerBound*) lowerBound;
  if (canModifyBounds) {
    mlb->prunePlanes(numBackups);
  }
  mlb->writeToFile(outFileName);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2007/03/25 15:14:52  trey
 * removed special case for actions tied according to LB; was causing a crash for some reason, not tracked down yet
 *
 * Revision 1.7  2007/03/24 22:39:34  trey
 * removed const-ness of problem argument
 *
 * Revision 1.6  2007/03/23 02:11:18  trey
 * switched back to using lookahead 1 policy during evaluation
 *
 * Revision 1.5  2007/03/22 23:57:28  trey
 * added ability for derived classes of IncrementalLowerBound to implement their own chooseAction() method that overrides the default in BoundPair
 *
 * Revision 1.4  2006/11/07 20:06:38  trey
 * added getQValue() function
 *
 * Revision 1.3  2006/10/26 21:26:19  trey
 * fixed problem of crashing when search strategies do not demand maintainLowerBound=1 or maintainUpperBound=1
 *
 * Revision 1.2  2006/10/24 19:11:01  trey
 * added writePolicy() from ConvexBounds
 *
 * Revision 1.1  2006/10/24 02:05:48  trey
 * initial check-in
 *
 *
 ***************************************************************************/
