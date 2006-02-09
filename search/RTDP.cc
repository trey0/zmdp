/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-02-09 21:59:04 $
   
 @file    RTDP.cc
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
#include "RTDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define OBS_IS_ZERO_EPS (1e-10)

namespace zmdp {

RTDP::RTDP(AbstractBound* _initUpperBound) :
  problem(NULL),
  initUpperBound(_initUpperBound),
  boundsFile(NULL),
  initialized(false)
{}

void RTDP::init(void)
{
  // initUpperBound set in constructor
  initUpperBound->initialize();

  lookup = new MDPHash();
  root = getNode(problem->getInitialState());

  previousElapsedTime = secondsToTimeval(0.0);

  numBoundUpdates = 0;
  numNodes = 0;
  numInternalNodes = 0;
  numWeightPropUpdates = 0;

  if (NULL != boundsFile) {
    (*boundsFile) << "# wallclock time"
		  << ", lower bound"
		  << ", upper bound"
		  << ", # lower bound pts"
		  << ", # upper bound pts"
		  << ", # top level iterations"
		  << ", # backups"
		  << ", # value queries"
		  << endl;
    boundsFile->flush();
  }

  initialized = true;
}

void RTDP::planInit(const MDP* _problem)
{
  problem = _problem;
  initialized = false;
}

MDPNode* RTDP::getNode(const state_vector& s)
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
    (*lookup)[hs] = &cn;
    numNodes++;
    return &cn;
  } else {
    // return existing node
    return pr->second;
  }
}

void RTDP::expand(MDPNode& cn)
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

  numInternalNodes++;
}

void RTDP::updateInternal(MDPNode& cn, int* maxUBActionP)
{
  int maxUBAction = -1;

  // FIX: this initialization relies on uniform improvability -- is that what we want?
  cn.ubVal = -99e+20;
  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    Qa.ubVal = 0;
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	double oprob = e->obsProb;
	Qa.ubVal += oprob * sn.ubVal;
      }
    }
    Qa.ubVal = Qa.immediateReward + problem->getDiscount() * Qa.ubVal;

    if (Qa.ubVal > cn.ubVal) {
      cn.ubVal = Qa.ubVal;
      maxUBAction = a;
    }
  }

  if (NULL != maxUBActionP) *maxUBActionP = maxUBAction;
}

void RTDP::update(MDPNode& cn, int* maxUBActionP)
{
  if (cn.isFringe()) {
    expand(cn);
  }
  updateInternal(cn, maxUBActionP);

  numBoundUpdates++;
}

void RTDP::trialRecurse(MDPNode& cn, double pTarget, int depth)
{
  // check for termination
  if (cn.isTerminal) {
#if USE_DEBUG_PRINT
    printf("trialRecurse: depth=%d ubVal=%g terminal node (terminating)\n",
	   depth, cn.ubVal);
#endif
    return;
  }

  // update to ensure cached values in cn.Q are correct
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
  MDPNode& bestSuccessor = *cn.Q[maxUBAction].outcomes[simulatedOutcome]->nextState;
  double pNextTarget = pTarget / problem->getDiscount();
  trialRecurse(bestSuccessor, pNextTarget, depth+1);

  update(cn, NULL);
}

void RTDP::doTrial(MDPNode& cn, double pTarget)
{
  trialRecurse(cn, pTarget, 0);
}

bool RTDP::planFixedTime(const state_vector& currentBelief,
			 double maxTimeSeconds,
			 double minPrecision)
{
  boundsStartTime = getTime() - previousElapsedTime;

  if (!initialized) {
    boundsStartTime = getTime();
    init();
  }

  // disable this termination check for now
  //if (root->ubVal - root->lbVal < minPrecision) return true;

  doTrial(*root, minPrecision);

  previousElapsedTime = getTime() - boundsStartTime;

  if (NULL != boundsFile) {
    (*boundsFile) << timevalToSeconds(getTime() - boundsStartTime)
		  << " " << -1 // lower bound, n/a
		  << " " << root->ubVal
		  << " " << numNodes // lower bound # 'alpha vectors'
		  << " " << numNodes // upper bound # pts
		  << " " << -1 // num top-level iterations, n/a
		  << " " << numBoundUpdates
		  << " " << -1 // num value queries, n/a
		  << endl;
    boundsFile->flush();
  }

  return false;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int RTDP::chooseAction(const state_vector& s)
{
  outcome_prob_vector opv;
  state_vector sp;
  double bestLB = -99e+20;
  int bestLBAction = -1;
  FOR (a, problem->getNumActions()) {
    problem->getOutcomeProbVector(opv, s, a);
    double sumLB = 0;
    FOR (o, opv.size()) {
      if (opv(o) > OBS_IS_ZERO_EPS) {
	ValueInterval intv = getValueAt(problem->getNextState(sp, s, a, o));
	sumLB += opv(o) * intv.l;
      }
    }
    sumLB = problem->getReward(s,a) + problem->getDiscount() * sumLB;
    if (sumLB > bestLB) {
      bestLB = sumLB;
      bestLBAction = a;
    }
  }

  return bestLBAction;
}

void RTDP::setBoundsFile(std::ostream* _boundsFile)
{
  boundsFile = _boundsFile;
}

ValueInterval RTDP::getValueAt(const state_vector& s) const
{
  typeof(lookup->begin()) pr = lookup->find(hashable(s));
  if (lookup->end() == pr) {
    return ValueInterval(-1, // bogus value, n/a
			 initUpperBound->getValue(s));
  } else {
    const MDPNode& cn = *pr->second;
    return ValueInterval(-1, // bogus value n/a
			 cn.ubVal);
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
