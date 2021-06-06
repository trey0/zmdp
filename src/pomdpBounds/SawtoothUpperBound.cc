/********** tell emacs we use -*- c++ -*- style comments *******************
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <list>

#include "FastInfUBInitializer.h"
#include "SawtoothUpperBound.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

#define MIN_RATIO_EPS (1e-10)
#define PRUNE_PTS_INCREMENT (10)
#define PRUNE_PTS_FACTOR (2.0)
#define CORNER_EPS (1e-6)

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

SawtoothUpperBound::SawtoothUpperBound(const MDP *_pomdp,
                                       const ZMDPConfig *_config)
    : pomdp((const Pomdp *)_pomdp), config(_config), core(NULL) {
  numStates = pomdp->getBeliefSize();
  lastPruneNumPts = 0;
  lastPruneNumBackups = -1;
  useSawtoothSupportList = config->getBool("useSawtoothSupportList");

  if (useSawtoothSupportList) {
    supportList.resize(pomdp->getBeliefSize());
  }
}

SawtoothUpperBound::~SawtoothUpperBound(void) {
  FOR_EACH(entryP, pts) { delete *entryP; }
}

void SawtoothUpperBound::initialize(double targetPrecision) {
  FastInfUBInitializer fib(pomdp, this);
  fib.initialize(targetPrecision);
}

void SawtoothUpperBound::initNodeBound(MDPNode &cn) {
  if (cn.isTerminal) {
    setUBForNode(cn, 0, true);
  } else {
    setUBForNode(cn, getValue(cn.s, NULL), false);
  }
}

void SawtoothUpperBound::update(MDPNode &cn, int *maxUBActionP) {
  double newUBVal = getNewUBValue(cn, maxUBActionP);
  setUBForNode(cn, newUBVal, true);
}

// returns the upper bound that the (belief,value) pair c induces on b.
double SawtoothUpperBound::getBVValue(const belief_vector &b,
                                      const BVPair *cPair,
                                      double innerCornerPtsB,
                                      double innerCornerPtsC) {
  const belief_vector &c = cPair->b;
  double cVal = cPair->v;

  if (innerCornerPtsC <= cVal) {
    // c does not provide a useful bound because it actually lies above
    // the plane defined by cornerPts.  ideally, we would prune c here.
    return 99e+20;
  }

  double minRatio = 99e+20;
  typeof(b.data.begin()) bi = b.data.begin();
  typeof(b.data.begin()) bend = b.data.end();
  typeof(b.data.begin()) ci = c.data.begin();
  typeof(b.data.begin()) cend = c.data.end();
  bool bdone = false;

  for (; ci != cend; ci++) {
    if (0.0 == ci->value)
      continue;

    // advance until bi->index >= ci->index
    while (1) {
      if (bi == bend) {
        bdone = true;
        goto breakCiLoop;
      }
      if (bi->index >= ci->index)
        break;
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
      if (0.0 != ci->value)
        return 99e+20;
    }
  }

#if 1
  if (minRatio > 1) {
    if (minRatio < 1 + MIN_RATIO_EPS) {
      // round-off error, correct it down to 1
      minRatio = 1;
    } else {
      cout << "ERROR: minRatio > 1 in upperBoundInternal!" << endl;
      cout << "  (minRatio-1)=" << (minRatio - 1) << endl;
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
bool SawtoothUpperBound::dominates(const BVPair *xPair, const BVPair *yPair) {
  double xValueAtY = getBVValue(yPair->b, xPair, yPair->innerCornerCache,
                                xPair->innerCornerCache);
  double yValueAtY = yPair->v;
  return (xValueAtY < yValueAtY + ZMDP_BOUNDS_PRUNE_EPS);
}

double SawtoothUpperBound::getValue(const belief_vector &b,
                                    const MDPNode *cn) const {
  const BVList *ptsToCheck;
  if (useSawtoothSupportList) {
    ptsToCheck = &supportList[b.data[0].index];
  } else {
    ptsToCheck = &pts;
  }

  double innerCornerPtsB = inner_prod(cornerPts, b);
  double minValue = innerCornerPtsB;
  FOR_EACH(cPairP, *ptsToCheck) {
    double innerCornerPtsC = inner_prod(cornerPts, (*cPairP)->b);
    minValue = std::min(
        minValue, getBVValue(b, *cPairP, innerCornerPtsB, innerCornerPtsC));
  }
  return minValue;
}

void SawtoothUpperBound::deleteAndForward(BVPair *victim, BVPair *dominator) {
  if (useSawtoothSupportList) {
    // remove victim from supportList
    FOR_EACH(bi, victim->b.data) {
      BVList &bvl = supportList[bi->index];
      FOR_EACH(eltP, bvl) {
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

  FOR_EACH(ptP, pts) {
    BVPair *pt = *ptP;
    pt->innerCornerCache = inner_prod(cornerPts, pt->b);
  }

  typeof(pts.begin()) candidateP = pts.begin();
  while (candidateP != pts.end()) {
    BVPair *candidate = *candidateP;
    const BVList *ptsToCheck;
    if (useSawtoothSupportList) {
      ptsToCheck = &supportList[candidate->b.data[0].index];
    } else {
      ptsToCheck = &pts;
    }
    typeof(ptsToCheck->begin()) opponentP = ptsToCheck->begin();
    while (opponentP != ptsToCheck->end()) {
      BVPair *opponent = *opponentP;
      if (candidate == opponent) {
        // duh, can't dominate yourself
      } else if (candidate->numBackupsAtCreation <= lastPruneNumBackups &&
                 opponent->numBackupsAtCreation <= lastPruneNumBackups) {
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
  nextCandidate:;
  }

  if (zmdpDebugLevelG >= 1) {
    cout << "... pruned # pts from " << oldNum << " down to " << pts.size()
         << endl;
  }
  lastPruneNumPts = pts.size();
  lastPruneNumBackups = numBackups;
}

void SawtoothUpperBound::maybePrune(int numBackups) {
  unsigned int nextPruneNumPts = max(lastPruneNumPts + PRUNE_PTS_INCREMENT,
                                     (int)(lastPruneNumPts * PRUNE_PTS_FACTOR));
  if (pts.size() > nextPruneNumPts) {
    prune(numBackups);
  }
}

// if b is a corner point e_i, return i.  else return -1
int SawtoothUpperBound::whichCornerPoint(const belief_vector &b) const {
  int non_zero_count = 0;
  int non_zero_index = -1;
  FOR(i, numStates) {
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

void SawtoothUpperBound::addPoint(BVPair *bv) {
  int wc = whichCornerPoint(bv->b);
  if (-1 == wc) {
    if (useSawtoothSupportList) {
      // add new point to supportList
      FOR_EACH(bi, bv->b.data) { supportList[bi->index].push_back(bv); }
    }
    pts.push_back(bv);
  } else {
    cornerPts(wc) = bv->v;
    delete bv;
  }
}

void SawtoothUpperBound::addPoint(const belief_vector &b, double val) {
  int wc = whichCornerPoint(b);
  if (-1 == wc) {
    BVPair *bv = new BVPair(b, val);

    if (useSawtoothSupportList) {
      // add new point to supportList
      FOR_EACH(bi, b.data) { supportList[bi->index].push_back(bv); }
    }

    pts.push_back(bv);
  } else {
    cornerPts(wc) = val;
  }
}

void SawtoothUpperBound::printToStream(ostream &out) const {
  out << "{" << endl;
  out << "  cornerPts = " << sparseRep(cornerPts) << endl;
  out << "  " << pts.size() << " points:" << endl;
  FOR_EACH(pt, pts) {
    const BVPair *pr = *pt;
    out << "    " << sparseRep(pr->b) << " => " << pr->v << " ("
        << getValue(pr->b, NULL) << ")" << endl;
  }
  out << "}" << endl;
}

// upper bound on long-term reward for taking action a
double SawtoothUpperBound::getNewUBValueQ(MDPNode &cn, int a) {
  double val = 0;

  MDPQEntry &Qa = cn.Q[a];
  FOR(o, Qa.getNumOutcomes()) {
    MDPEdge *e = Qa.outcomes[o];
    if (NULL != e) {
      val += e->obsProb * getValue(e->nextState->s, NULL);
    }
  }
  val = Qa.immediateReward + pomdp->getDiscount() * val;
  Qa.ubVal = val;

  return val;
}

double SawtoothUpperBound::getNewUBValueSimple(MDPNode &cn, int *maxUBActionP) {
  timeval startTime;
  if (zmdpDebugLevelG >= 1) {
    startTime = getTime();
  }

  double val, maxVal = -99e+20;
  int maxUBAction = -1;
  FOR(a, pomdp->getNumActions()) {
    val = getNewUBValueQ(cn, a);
    if (val > maxVal) {
      maxVal = val;
      maxUBAction = a;
    }
  }

  if (NULL != maxUBActionP)
    *maxUBActionP = maxUBAction;

  if (zmdpDebugLevelG >= 1) {
    cout << "** newUpperBound: elapsed time = "
         << timevalToSeconds(getTime() - startTime) << endl;
  }

  return maxVal;
}

double SawtoothUpperBound::getNewUBValueUseCache(MDPNode &cn,
                                                 int *maxUBActionP) {
  timeval startTime;
  if (zmdpDebugLevelG >= 1) {
    startTime = getTime();
  }

  // cache upper bound for each action
  dvector cachedUpperBound(pomdp->getNumActions());
  FOR(a, pomdp->numActions) { cachedUpperBound(a) = cn.Q[a].ubVal; }

  // remember which Q functions we have updated on this call
  std::vector<bool> updatedAction(pomdp->getNumActions());
  FOR(a, pomdp->getNumActions()) { updatedAction[a] = false; }

  double val;
  int maxUBAction = argmax_elt(cachedUpperBound);
  while (1) {
    // do the backup for the best Q
    val = getNewUBValueQ(cn, maxUBAction);
    cachedUpperBound(maxUBAction) = val;
    updatedAction[maxUBAction] = true;

    // the best action may have changed after updating Q
    maxUBAction = argmax_elt(cachedUpperBound);

    // if the best action after the update is one that we have already
    //    updated, we're done
    if (updatedAction[maxUBAction])
      break;
  }

  double maxVal = cachedUpperBound(maxUBAction);

  if (NULL != maxUBActionP)
    *maxUBActionP = maxUBAction;

  if (zmdpDebugLevelG >= 1) {
    cout << "** newUpperBound: elapsed time = "
         << timevalToSeconds(getTime() - startTime) << endl;
  }

  return maxVal;
}

double SawtoothUpperBound::getNewUBValue(MDPNode &cn, int *maxUBActionP) {
  if (BP_QVAL_UNDEFINED == cn.Q[0].ubVal) {
    return getNewUBValueSimple(cn, maxUBActionP);
  } else {
    return getNewUBValueUseCache(cn, maxUBActionP);
  }
}

void SawtoothUpperBound::setUBForNode(MDPNode &cn, double newUB, bool addBV) {
  BVPair *newBV = new BVPair();
  newBV->b = cn.s;
  newBV->v = newUB;
  newBV->numBackupsAtCreation = core->numBackups;

  cn.ubVal = newUB;

  if (addBV) {
    addPoint(newBV);
    maybePrune(core->numBackups);
  }
}

int SawtoothUpperBound::getStorage(int whichMetric) const {
  switch (whichMetric) {
  case ZMDP_S_NUM_ELTS:
    // return number of belief/value pairs -- each corner point counts as 1
    return pts.size() + cornerPts.size();

  case ZMDP_S_NUM_ENTRIES: {
    // return total number of entries in all belief/value pairs
    int entryCount = 0;
    FOR_EACH(ptP, pts) {
      const BVPair &p = **ptP;
      entryCount += p.b.filled() + 1; // (add one for the value)
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
