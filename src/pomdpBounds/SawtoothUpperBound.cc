/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.16 $  $Author: trey $  $Date: 2007-01-15 17:22:32 $
   
 @file    SawtoothUpperBound.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>
#include <list>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "SawtoothUpperBound.h"
#include "FastInfUBInitializer.h"

#define MIN_RATIO_EPS (1e-10)
#define PRUNE_PTS_INCREMENT (10)
#define PRUNE_PTS_FACTOR (2.0)
#define CORNER_EPS (1e-6)

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

SawtoothUpperBound::SawtoothUpperBound(const MDP* _pomdp,
				       const ZMDPConfig* _config) :
  pomdp((const Pomdp*) _pomdp),
  config(_config),
  core(NULL)
{
  numStates = pomdp->getBeliefSize();
  lastPruneNumPts = 0;
  lastPruneNumBackups = -1;
  useSawtoothSupportList = config->getBool("useSawtoothSupportList");

  if (useSawtoothSupportList) {
    supportList.resize(pomdp->getBeliefSize());
  }
}

SawtoothUpperBound::~SawtoothUpperBound(void)
{
  FOR_EACH (entryP, pts) {
    delete *entryP;
  }
}

void SawtoothUpperBound::initialize(double targetPrecision)
{
  FastInfUBInitializer fib(pomdp, this);
  fib.initialize(targetPrecision);
}

void SawtoothUpperBound::initNodeBound(MDPNode& cn)
{
  if (cn.isTerminal) {
    setUBForNode(cn, 0, true);
  } else {
    setUBForNode(cn, getValue(cn.s, NULL), false);
  }
}

void SawtoothUpperBound::update(MDPNode& cn, int* maxUBActionP)
{
  double newUBVal = getNewUBValue(cn, maxUBActionP);
  setUBForNode(cn, newUBVal, true);
}

// returns the upper bound that the (belief,value) pair c induces on b.
double SawtoothUpperBound::getBVValue(const belief_vector& b,
				      const BVPair* cPair,
				      double innerCornerPtsB,
				      double innerCornerPtsC)
{
  const belief_vector& c = cPair->b;
  double cVal = cPair->v;
  
  if (innerCornerPtsC <= cVal) {
    // c does not provide a useful bound because it actually lies above
    // the plane defined by cornerPts.  ideally, we would prune c here.
    return 99e+20;
  }
  
  double minRatio = 99e+20;
  typeof(b.data.begin()) bi = b.data.begin();
  typeof(b.data.begin())  bend = b.data.end();
  typeof(b.data.begin()) ci = c.data.begin();
  typeof(b.data.begin()) cend = c.data.end();
  bool bdone = false;
  
  for (; ci != cend; ci++) {
    if (0.0 == ci->value) continue;
    
    // advance until bi->index >= ci->index
    while (1) {
      if (bi == bend) {
	bdone = true;
	goto breakCiLoop;
      }
      if (bi->index >= ci->index) break;
      bi++;
    }
    
    if (bi->index > ci->index) {
      // we found a j such that b(j) = 0 and c(j) != 0, which implies
      // minRatio = 0, so c does not provide a useful bound
      return 99e+20;
    }
    
    minRatio = std::min(minRatio, bi->value / ci->value);
  }
 breakCiLoop:
  if (bdone) {
    for (; ci != cend; ci++) {
      if (0.0 != ci->value) return 99e+20;
    }
  }
  
#if 1
  if (minRatio > 1) {
    if (minRatio < 1 + MIN_RATIO_EPS) {
      // round-off error, correct it down to 1
      minRatio = 1;
    } else {
      cout << "ERROR: minRatio > 1 in upperBoundInternal!" << endl;
      cout << "  (minRatio-1)=" << (minRatio-1) << endl;
      cout << "  normb=" << norm_1(b) << endl;
      cout << "  b=" << sparseRep(b) << endl;
      cout << "  normc=" << norm_1(c) << endl;
      cout << "  c=" << sparseRep(c) << endl;
      exit(EXIT_FAILURE);
    }
  }
#endif
  
  return innerCornerPtsB + minRatio * (cVal - innerCornerPtsC);
}

// return true iff xPair dominates yPair.  NOTE: innerCornerCache fields
// must be set properly before dominates() is called!
bool SawtoothUpperBound::dominates(const BVPair* xPair,
				   const BVPair* yPair)
{
  double xValueAtY = getBVValue(yPair->b, xPair,
				yPair->innerCornerCache,
				xPair->innerCornerCache);
  double yValueAtY = yPair->v;
  return (xValueAtY < yValueAtY + ZMDP_BOUNDS_PRUNE_EPS);
}

double SawtoothUpperBound::getValue(const belief_vector& b, const MDPNode* cn) const
{
  const BVList* ptsToCheck;
  if (useSawtoothSupportList) {
    ptsToCheck = &supportList[b.data[0].index];
  } else {
    ptsToCheck = &pts;
  }

  double innerCornerPtsB = inner_prod(cornerPts, b);
  double minValue = innerCornerPtsB;
  FOR_EACH (cPairP, *ptsToCheck) {
    double innerCornerPtsC = inner_prod(cornerPts, (*cPairP)->b);
    minValue = std::min(minValue, getBVValue(b, *cPairP, innerCornerPtsB, innerCornerPtsC));
  }
  return minValue;
}

void SawtoothUpperBound::deleteAndForward(BVPair* victim,
					  BVPair* dominator)
{
  if (useSawtoothSupportList) {
    // remove victim from supportList
    FOR_EACH (bi, victim->b.data) {
      BVList& bvl = supportList[bi->index];
      FOR_EACH (eltP, bvl) {
	if (victim == *eltP) {
	  eraseElement(bvl, eltP);
	  break;
	}
      }
    }
  }

  delete victim;
}

void SawtoothUpperBound::prune(int numBackups) {
  int oldNum = -1;
  if (zmdpDebugLevelG >= 1) {
    oldNum = pts.size();
  }

  FOR_EACH (ptP, pts) {
    BVPair* pt = *ptP;
    pt->innerCornerCache = inner_prod(cornerPts, pt->b);
  }

  typeof(pts.begin()) candidateP = pts.begin();
  while (candidateP != pts.end()) {
    BVPair* candidate = *candidateP;
    const BVList* ptsToCheck;
    if (useSawtoothSupportList) {
      ptsToCheck = &supportList[candidate->b.data[0].index];
    } else {
      ptsToCheck = &pts;
    }
    typeof(ptsToCheck->begin()) opponentP = ptsToCheck->begin();
    while (opponentP != ptsToCheck->end()) {
      BVPair* opponent = *opponentP;
      if (candidate == opponent) {
	// duh, can't dominate yourself
      } else if (candidate->numBackupsAtCreation <= lastPruneNumBackups
	  && opponent->numBackupsAtCreation <= lastPruneNumBackups) {
	// candidate and opponent were compared the last time we pruned
	// and neither dominates the other; leave them both in
      } else if (dominates(opponent, candidate)) {
	// candidate is pruned
	deleteAndForward(candidate, opponent);
	candidateP = eraseElement(pts, candidateP);
	goto nextCandidate;
      }
      opponentP++;
    }
    candidateP++;
  nextCandidate: ;
  }

  if (zmdpDebugLevelG >= 1) {
    cout << "... pruned # pts from " << oldNum << " down to " << pts.size() << endl;
  }
  lastPruneNumPts = pts.size();
  lastPruneNumBackups = numBackups;
}

void SawtoothUpperBound::maybePrune(int numBackups)
{
  unsigned int nextPruneNumPts = max(lastPruneNumPts + PRUNE_PTS_INCREMENT,
				     (int) (lastPruneNumPts * PRUNE_PTS_FACTOR));
  if (pts.size() > nextPruneNumPts) {
    prune(numBackups);
  }
}

// if b is a corner point e_i, return i.  else return -1
int SawtoothUpperBound::whichCornerPoint(const belief_vector& b) const {
  int non_zero_count = 0;
  int non_zero_index = -1;
  FOR (i, numStates) {
    if (fabs(b(i)) > CORNER_EPS) {
      if (non_zero_count == 0) {
	non_zero_index = i;
	non_zero_count++;
      } else {
	return -1;
      }
    }
  }
  if (non_zero_count == 0) {
    return -1;
  } else {
    return non_zero_index;
  }
}

void SawtoothUpperBound::addPoint(BVPair* bv)
{
  int wc = whichCornerPoint(bv->b);
  if (-1 == wc) {
    if (useSawtoothSupportList) {
      // add new point to supportList
      FOR_EACH (bi, bv->b.data) {
	supportList[bi->index].push_back(bv);
      }
    }
    pts.push_back(bv);
  } else {
    cornerPts(wc) = bv->v;
    delete bv;
  }
}

void SawtoothUpperBound::addPoint(const belief_vector& b, double val)
{
  int wc = whichCornerPoint(b);
  if (-1 == wc) {
    BVPair* bv = new BVPair(b,val);

    if (useSawtoothSupportList) {
      // add new point to supportList
      FOR_EACH (bi, b.data) {
	supportList[bi->index].push_back(bv);
      }
    }

    pts.push_back(bv);
  } else {
    cornerPts(wc) = val;
  }
}

void SawtoothUpperBound::printToStream(ostream& out) const
{
  out << "{" << endl;
  out << "  cornerPts = " << sparseRep(cornerPts) << endl;
  out << "  " << pts.size() << " points:" << endl;
  FOR_EACH (pt, pts) {
    const BVPair* pr = *pt;
    out << "    " << sparseRep(pr->b)
	<< " => " << pr->v
	<< " (" << getValue(pr->b, NULL) << ")" << endl;
  }
  out << "}" << endl;
}

// upper bound on long-term reward for taking action a
double SawtoothUpperBound::getNewUBValueQ(MDPNode& cn, int a)
{
  double val = 0;

  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      val += e->obsProb * getValue(e->nextState->s, NULL);
    }
  }
  val = Qa.immediateReward + pomdp->getDiscount() * val;
  Qa.ubVal = val;

  return val;
}

double SawtoothUpperBound::getNewUBValueSimple(MDPNode& cn, int* maxUBActionP)
{
  timeval startTime;
  if (zmdpDebugLevelG >= 1) {
    startTime = getTime();
  }

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
  
  if (zmdpDebugLevelG >= 1) {
    cout << "** newUpperBound: elapsed time = "
	 << timevalToSeconds(getTime() - startTime)
	 << endl;
  }
  
  return maxVal;
}

double SawtoothUpperBound::getNewUBValueUseCache(MDPNode& cn, int* maxUBActionP)
{
  timeval startTime;
  if (zmdpDebugLevelG >= 1) {
    startTime = getTime();
  }

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

  if (zmdpDebugLevelG >= 1) {
    cout << "** newUpperBound: elapsed time = "
	 << timevalToSeconds(getTime() - startTime)
	 << endl;
  }

  return maxVal;
}

double SawtoothUpperBound::getNewUBValue(MDPNode& cn, int* maxUBActionP)
{
  if (BP_QVAL_UNDEFINED == cn.Q[0].ubVal) {
    return getNewUBValueSimple(cn, maxUBActionP);
  } else {
    return getNewUBValueUseCache(cn, maxUBActionP);
  }
}

void SawtoothUpperBound::setUBForNode(MDPNode& cn, double newUB, bool addBV)
{
  BVPair* newBV = new BVPair();
  newBV->b = cn.s;
  newBV->v = newUB;
  newBV->numBackupsAtCreation = core->numBackups;

  cn.ubVal = newUB;

  if (addBV) {
    addPoint(newBV);
    maybePrune(core->numBackups);
  }
}

int SawtoothUpperBound::getStorage(int whichMetric) const
{
  switch (whichMetric) {
  case ZMDP_S_NUM_ELTS:
    // return number of belief/value pairs -- each corner point counts as 1
    return pts.size() + cornerPts.size();

  case ZMDP_S_NUM_ENTRIES: {
    // return total number of entries in all belief/value pairs
    int entryCount = 0;
    FOR_EACH (ptP, pts) {
      const BVPair& p = **ptP;
      entryCount += p.b.filled()+1; // (add one for the value)
    }
    // each corner point counts as one entry
    return entryCount + cornerPts.size();
  }

  default:
    /* N/A */
    return 0;
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2007/01/14 00:54:10  trey
 * added hooks for logging storage space during a run
 *
 * Revision 1.14  2006/10/30 20:00:15  trey
 * USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
 *
 * Revision 1.13  2006/10/24 19:12:32  trey
 * replaced useConvexSupportList with useSawtoothSupportList
 *
 * Revision 1.12  2006/10/24 02:13:04  trey
 * distributed update code from ConvexBounds to SawtoothUpperBound, allows more flexibility
 *
 * Revision 1.11  2006/10/18 18:07:13  trey
 * USE_TIME_WITHOUT_HEURISTIC is now a run-time config parameter
 *
 * Revision 1.10  2006/10/17 19:14:02  trey
 * fixed bad interaction between pruning and support lists; new pruning code is simpler, probably not quite as fast, hopefully bug-free
 *
 * Revision 1.9  2006/07/26 20:22:10  trey
 * new implementation of USE_CONVEX_CACHE; during pruning, now skip comparison of points if they were compared during last pruning cycle
 *
 * Revision 1.8  2006/07/26 16:32:24  trey
 * removed dependence of pruning on USE_CONVEX_SUPPORT_LIST
 *
 * Revision 1.7  2006/07/25 19:41:14  trey
 * pulled out constants to #defines
 *
 * Revision 1.6  2006/07/24 17:08:02  trey
 * added USE_CONVEX_SUPPORT_LIST
 *
 * Revision 1.5  2006/07/24 14:38:24  trey
 * fixed inaccurate comment
 *
 * Revision 1.4  2006/07/14 15:09:48  trey
 * cleaned up pruning
 *
 * Revision 1.3  2006/07/12 19:45:55  trey
 * cleaned out copyFrom() cruft
 *
 * Revision 1.2  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.16  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.15  2005/10/28 03:52:15  trey
 * simplified license
 *
 * Revision 1.14  2005/10/28 02:55:36  trey
 * added copyright header
 *
 * Revision 1.13  2005/10/27 22:03:40  trey
 * cleaned out some cruft
 *
 * Revision 1.12  2005/10/21 20:20:09  trey
 * added namespace zmdp
 *
 * Revision 1.11  2005/03/28 18:14:46  trey
 * renamed updateAsSafety to useSafetyUpdate
 *
 * Revision 1.10  2005/03/25 21:43:26  trey
 * added updateAsSafety flag in BoundFunction and AlphaList, made some FocusedPomdp functions take a bound as an argument
 *
 * Revision 1.9  2005/03/25 19:23:39  trey
 * made lowerBoundV and upperBoundV explicit in FocusedPomdp
 *
 * Revision 1.8  2005/03/11 19:24:35  trey
 * switched from hash_map to list representation
 *
 * Revision 1.7  2005/02/08 23:55:47  trey
 * updated to work when alpha_vector = cvector
 *
 * Revision 1.6  2005/02/06 16:41:30  trey
 * removed broken upperBoundInternal2 and added #if USE_CPLEX around upperBoundInternal1
 *
 * Revision 1.5  2005/02/04 21:03:40  trey
 * added upperBoundInternal3 and timing code for lp optimization
 *
 * Revision 1.4  2005/01/28 03:25:53  trey
 * improved sla compatibility
 *
 * Revision 1.3  2005/01/26 04:18:39  trey
 * fixed for changed rand_vector() API, fixed some spurious warnings
 *
 * Revision 1.2  2005/01/17 19:34:30  trey
 * added some test code for comparing upper bound mechanisms
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.9  2003/09/18 21:00:28  trey
 * upper bound self-update seems to be working
 *
 * Revision 1.8  2003/09/17 20:54:24  trey
 * seeing good performance on tagAvoid (but some mods since run started...)
 *
 * Revision 1.7  2003/09/16 00:57:01  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.6  2003/09/11 01:46:41  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.5  2003/07/22 19:59:36  trey
 * several minor fixes; still observing discrepancy between sim and solver lower bound
 *
 * Revision 1.4  2003/07/18 16:07:58  trey
 * finished implementing Q caching
 *
 * Revision 1.3  2003/07/17 18:50:40  trey
 * did prep for caching upper bound Q values
 *
 * Revision 1.2  2003/07/16 16:06:41  trey
 * alpha vectors now tagged with action, tweaked interface to LP solver
 *
 * Revision 1.1  2003/06/26 15:41:18  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
