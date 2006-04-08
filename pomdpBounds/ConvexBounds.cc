/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-04-08 22:21:25 $
   
 @file    ConvexBounds.cc
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
#include "AbstractBound.h"
#include "ConvexBounds.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define CB_QVAL_UNDEFINED (-99e+20)

namespace zmdp {

ConvexBounds::ConvexBounds(bool _useLowerBound) :
  useLowerBound(_useLowerBound)
{}

// lower bound on long-term reward for taking action a (alpha vector)
void ConvexBounds::getNewLBPlaneQ(LBPlane& result, const MDPNode& cn, int a)
{
  alpha_vector betaA(pomdp->getBeliefSize()), betaAO(pomdp->getBeliefSize());
  alpha_vector tmp, tmp2;
  obs_prob_vector opv;
#if USE_MASKED_ALPHA
  alpha_vector tmp3;
#endif

  set_to_zero(betaA);
  
  bool defaultIsSet = false;
  alpha_vector defaultBetaAO;
  const MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      betaAO = lowerBound->getBestLBPlane(e->nextState->s).alpha;
    } else {
      // impossible to see this observation, so it doesn't make sense to
      // pick the alpha that optimizes for the next belief.  plug in a
      // default alpha (arbitrarily chosen to optimize for cn.b).
      if (!defaultIsSet) {
	defaultBetaAO = lowerBound->getBestLBPlane(cn.s).alpha;
	defaultIsSet = true;
      }
      betaAO = defaultBetaAO;
    }

    emult_column( tmp, pomdp->O[a], o, betaAO );
#if USE_MASKED_ALPHA
    mult( tmp3, pomdp->T[a], tmp );
    mask_copy( tmp2, tmp3, b );
#else
    mult( tmp2, tmp, pomdp->Ttr[a] );
#endif
    betaA += tmp2;
  }

  alpha_vector Rxa;
#if USE_MASKED_ALPHA
  copy_from_column( tmp, pomdp->R, a );
  mask_copy( Rxa, tmp, b );
#else
  copy_from_column( Rxa, pomdp->R, a );
#endif
  betaA *= pomdp->getDiscount();
  betaA += Rxa;
  
#if USE_MASKED_ALPHA
  result.copyFrom(betaA, a, b);
#else
  result.copyFrom(betaA, a);
#endif
}

void ConvexBounds::getNewLBPlane(LBPlane& result, MDPNode& cn)
{
#if USE_DEBUG_PRINT
  timeval startTime = getTime();
#endif

  double val, maxVal = -99e+20;
  LBPlane betaA;
 
  FOR (a, cn.getNumActions()) {
    getNewLBPlaneQ(betaA, cn, a);
    val = inner_prod(betaA.alpha, cn.s);
    cn.Q[a].lbVal = val;
    if (val > maxVal) {
      maxVal = val;
      result = betaA;
    }
  }
#if USE_DEBUG_PRINT
  cout << "** newLowerBound: elapsed time = "
       << timevalToSeconds(getTime() - startTime)
       << endl;
#endif
}

void ConvexBounds::updateLowerBound(MDPNode& cn)
{
  LBPlane newPlane;
  getNewLBPlane(newPlane, cn);
  lowerBound->addLBPlane(cn.s, newPlane);
  cn.lbVal = inner_prod(cn.s, newPlane.alpha);
}

// upper bound on long-term reward for taking action a
double ConvexBounds::getNewUBValueQ(MDPNode& cn, int a)
{
  double val = 0;

  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      val += e->obsProb * upperBound->getValue(e->nextState->s);
    }
  }
  val = Qa.immediateReward + pomdp->getDiscount() * val;
  Qa.ubVal = val;

  return val;
}

double ConvexBounds::getNewUBValueSimple(MDPNode& cn, int* maxUBActionP)
{
#if USE_DEBUG_PRINT
  timeval startTime = getTime();
#endif

  double val, maxVal = -99e+20;
  int maxUBAction = -1;
  FOR (a, pomdp->getNumActions()) {
    val = getNewUBValueQ(cn,a);
    if (val > maxVal) {
      maxVal = val;
      maxUBAction = a;
    }
  }

  if (NULL != maxUBActionP) *maxUBActionP = maxUBAction;
  
#if USE_DEBUG_PRINT
  cout << "** newUpperBound: elapsed time = "
       << timevalToSeconds(getTime() - startTime)
       << endl;
#endif
  
  return maxVal;
}

double ConvexBounds::getNewUBValueUseCache(MDPNode& cn, int* maxUBActionP)
{
#if USE_DEBUG_PRINT
  timeval startTime = getTime();
#endif

  // cache upper bound for each action
  dvector cachedUpperBound(pomdp->getNumActions());
  FOR (a, pomdp->numActions) {
    cachedUpperBound(a) = cn.Q[a].ubVal;
  }

  // remember which Q functions we have updated on this call
  std::vector<bool> updatedAction(pomdp->getNumActions());
  FOR (a, pomdp->getNumActions()) {
    updatedAction[a] = false;
  }

  double val;
  int maxUBAction = argmax_elt(cachedUpperBound);
  while (1) {
    // do the backup for the best Q
    val = getNewUBValueQ(cn,maxUBAction);
    cachedUpperBound(maxUBAction) = val;
    updatedAction[maxUBAction] = true;
      
    // the best action may have changed after updating Q
    maxUBAction = argmax_elt(cachedUpperBound);

    // if the best action after the update is one that we have already
    //    updated, we're done
    if (updatedAction[maxUBAction]) break;
  }

  double maxVal = cachedUpperBound(maxUBAction);
  
  if (NULL != maxUBActionP) *maxUBActionP = maxUBAction;

#if USE_DEBUG_PRINT
  cout << "** newUpperBound: elapsed time = "
       << timevalToSeconds(getTime() - startTime)
       << endl;
#endif

  return maxVal;
}

double ConvexBounds::getNewUBValue(MDPNode& cn, int* maxUBActionP)
{
  if (CB_QVAL_UNDEFINED == cn.Q[0].ubVal) {
    return getNewUBValueSimple(cn, maxUBActionP);
  } else {
    return getNewUBValueUseCache(cn, maxUBActionP);
  }
}

void ConvexBounds::updateUpperBound(MDPNode& cn, int* maxUBActionP)
{
  double newUBVal = getNewUBValue(cn, maxUBActionP);
  upperBound->addPoint(cn.s, newUBVal);
  cn.ubVal = newUBVal;
}

void ConvexBounds::initialize(const MDP* _pomdp,
			      double _targetPrecision)
{
  pomdp = (const Pomdp*) _pomdp;
  targetPrecision = _targetPrecision;

  if (useLowerBound) {
    lowerBound = new MaxPlanesLowerBound(pomdp);
    lowerBound->initialize(targetPrecision);
  }

  upperBound = new SawtoothUpperBound(pomdp);
  upperBound->initialize(targetPrecision);

  lookup = new MDPHash();
  root = NULL;

  numStatesTouched = 0;
  numStatesExpanded = 0;
  numBackups = 0;

  getNodeHandler = NULL;
  handlerData = NULL;
}

MDPNode* ConvexBounds::getRootNode(void)
{
  if (NULL == root) {
    root = getNode(pomdp->getInitialState());
  }
  return root;
}

MDPNode* ConvexBounds::getNode(const state_vector& s)
{
  string hs = hashable(s);
  MDPHash::iterator pr = lookup->find(hs);
  if (lookup->end() == pr) {
    // create a new fringe node
    MDPNode& cn = *(new MDPNode);
    cn.s = s;
    cn.isTerminal = pomdp->getIsTerminalState(s);
    if (cn.isTerminal) {
      cn.ubVal = 0;
    } else {
      cn.ubVal = upperBound->getValue(s);
    }
    if (useLowerBound) {
      if (cn.isTerminal) {
	cn.lbVal = 0;
      } else {
	cn.lbVal = lowerBound->getValue(s);
      }
    } else {
      cn.lbVal = -1; // n/a
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

void ConvexBounds::expand(MDPNode& cn)
{
  // set up successors for this fringe node (possibly creating new fringe nodes)
  outcome_prob_vector opv;
  state_vector sp;
  cn.Q.resize(pomdp->getNumActions());
  FOR (a, pomdp->getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    Qa.immediateReward = pomdp->getReward(cn.s, a);
    pomdp->getOutcomeProbVector(opv, cn.s, a);
    Qa.outcomes.resize(opv.size());
    FOR (o, opv.size()) {
      double oprob = opv(o);
      if (oprob > OBS_IS_ZERO_EPS) {
	MDPEdge* e = new MDPEdge();
        Qa.outcomes[o] = e;
        e->obsProb = oprob;
        e->nextState = getNode(pomdp->getNextState(sp, cn.s, a, o));
      } else {
        Qa.outcomes[o] = NULL;
      }
    }
    Qa.ubVal = CB_QVAL_UNDEFINED;
  }
  numStatesExpanded++;
}

void ConvexBounds::update(MDPNode& cn, int* maxUBActionP)
{
  if (cn.isFringe()) {
    expand(cn);
  }
  if (useLowerBound) {
    updateLowerBound(cn);
  }
  updateUpperBound(cn, maxUBActionP);

  numBackups++;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int ConvexBounds::chooseAction(const state_vector& s) const
{
  if (useLowerBound) {
    // 'direct' policy as opposed to 'lookahead' policy
    return lowerBound->getBestLBPlane(s).action;
  }

  // if not maintaining LB, fall back to UB

  outcome_prob_vector opv;
  state_vector sp;
  int bestAction = -1;

  double ubVal, ubValO;
  double maxVal = -99e+20;
  FOR (a, pomdp->getNumActions()) {
    pomdp->getOutcomeProbVector(opv, s, a);
    ubVal = 0;
    FOR (o, opv.size()) {
      if (opv(o) > OBS_IS_ZERO_EPS) {
	ubValO = upperBound->getValue(pomdp->getNextState(sp, s, a, o));
	ubVal += opv(o) * ubValO;
      }
    }
    ubVal = pomdp->getReward(s,a) + pomdp->getDiscount() * ubVal;
    if (ubVal > maxVal) {
      maxVal = ubVal;
      bestAction = a;
    }
  }
  return bestAction;
}

ValueInterval ConvexBounds::getValueAt(const state_vector& s) const
{
  return ValueInterval(useLowerBound ? lowerBound->getValue(s) : -1,
		       upperBound->getValue(s));
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/06 20:34:47  trey
 * filled out most of ConvexBounds implementation
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 *
 ***************************************************************************/
