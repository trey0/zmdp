/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-02-13 20:20:33 $
   
 @file    RTDPCore.cc
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
#include "RTDPCore.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define OBS_IS_ZERO_EPS (1e-10)

namespace zmdp {

RTDPCore::RTDPCore(AbstractBound* _initUpperBound) :
  problem(NULL),
  initUpperBound(_initUpperBound),
  boundsFile(NULL),
  initialized(false)
{}

void RTDPCore::init(void)
{
  if (getUseLowerBound()) {
    initLowerBound = problem->newLowerBound();
    initLowerBound->initialize();
  }

  initUpperBound->initialize();

  lookup = new MDPHash();
  root = getNode(problem->getInitialState());

  previousElapsedTime = secondsToTimeval(0.0);

  numStatesTouched = 0;
  numStatesExpanded = 0;
  numTrials = 0;
  numBackups = 0;

  if (NULL != boundsFile) {
    (*boundsFile) << "# wallclock time"
		  << ", lower bound"
		  << ", upper bound"
		  << ", # states touched"
		  << ", # states expanded"
		  << ", # trials"
		  << ", # backups"
		  << endl;
    boundsFile->flush();
  }

  initialized = true;
}

void RTDPCore::planInit(const MDP* _problem)
{
  problem = _problem;
  initialized = false;
}

MDPNode* RTDPCore::getNode(const state_vector& s)
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
      cn.isSolved = true;
    } else {
      cn.ubVal = initUpperBound->getValue(s);
      cn.isSolved = false;
    }
    if (getUseLowerBound()) {
      if (cn.isTerminal) {
	cn.lbVal = 0;
      } else {
	cn.lbVal = initLowerBound->getValue(s);
      }
    } else {
      cn.lbVal = -1; // n/a
    }
    (*lookup)[hs] = &cn;
    numStatesTouched++;
    return &cn;
  } else {
    // return existing node
    return pr->second;
  }
}

void RTDPCore::expand(MDPNode& cn)
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

// relies on correct Q values!
int RTDPCore::getMaxUBAction(MDPNode& cn) const
{
  double bestVal = -99e+20;
  int bestAction = -1;
  FOR (a, cn.getNumActions()) {
    const MDPQEntry& Qa = cn.Q[a];
    if (Qa.ubVal > bestVal) {
      bestVal = Qa.ubVal;
      bestAction = a;
    }
  }
  return bestAction;
}

int RTDPCore::getSimulatedOutcome(MDPNode& cn, int a) const
{
  double r = unit_rand();
  int result = 0;
  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      r -= e->obsProb;
      if (r <= 0) {
	result = o;
	break;
      }
    }
  }

  return result;
}

void RTDPCore::update(MDPNode& cn)
{
  if (cn.isFringe()) {
    expand(cn);
  }
  updateInternal(cn);
}

bool RTDPCore::planFixedTime(const state_vector& s,
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
    double elapsed = timevalToSeconds(getTime() - boundsStartTime);
    if (elapsed / lastPrintTime >= 1.01) {
      (*boundsFile) << timevalToSeconds(getTime() - boundsStartTime)
		    << " " << root->lbVal
		    << " " << root->ubVal
		    << " " << numStatesTouched
		    << " " << numStatesExpanded
		    << " " << numTrials
		    << " " << numBackups
		    << endl;
      boundsFile->flush();
      lastPrintTime = elapsed;
    }
  }

  return false;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int RTDPCore::chooseAction(const state_vector& s)
{
  outcome_prob_vector opv;
  state_vector sp;
  double bestVal = -99e+20;
  int bestAction = -1;
  FOR (a, problem->getNumActions()) {
    problem->getOutcomeProbVector(opv, s, a);
    double val = 0;
    FOR (o, opv.size()) {
      if (opv(o) > OBS_IS_ZERO_EPS) {
	ValueInterval intv = getValueAt(problem->getNextState(sp, s, a, o));
	val += opv(o) * (getUseLowerBound() ? intv.l : intv.u);
      }
    }
    val = problem->getReward(s,a) + problem->getDiscount() * val;
    if (val > bestVal) {
      bestVal = val;
      bestAction = a;
    }
  }

  return bestAction;
}

void RTDPCore::setBoundsFile(std::ostream* _boundsFile)
{
  boundsFile = _boundsFile;
}

ValueInterval RTDPCore::getValueAt(const state_vector& s) const
{
  typeof(lookup->begin()) pr = lookup->find(hashable(s));
  if (lookup->end() == pr) {
    return ValueInterval(getUseLowerBound() ? initLowerBound->getValue(s) : -1,
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
 * Revision 1.3  2006/02/13 19:53:11  trey
 * reduced amount of debug output
 *
 * Revision 1.2  2006/02/13 19:08:49  trey
 * moved numBackups tracking code for better flexibility
 *
 * Revision 1.1  2006/02/11 22:38:10  trey
 * moved much of the RTDP implementation into RTDPCore, where it can be shared by many RTDP variants
 *
 *
 ***************************************************************************/
