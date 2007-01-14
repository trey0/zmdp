/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-01-14 00:53:30 $
   
 @file    PointUpperBound.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith.

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

//#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "PointUpperBound.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

PointUpperBound::PointUpperBound(const MDP* _problem,
				 const ZMDPConfig* _config,
				 BoundPairCore* _core) :
  problem(_problem),
  config(_config),
  core(_core),
  initBound(NULL)
{}

PointUpperBound::~PointUpperBound(void)
{}

void PointUpperBound::initialize(double _targetPrecision)
{
  targetPrecision = _targetPrecision;
  initBound->initialize(targetPrecision);
}

double PointUpperBound::getValue(const state_vector& s,
				 const MDPNode* cn) const
{
  if (NULL == cn) {
    return initBound->getValue(s, NULL);
  } else {
    return cn->ubVal;
  }
}

double PointUpperBound::getNewUBValueQ(MDPNode& cn, int a)
{
  double ubVal;

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

  return ubVal;
}

void PointUpperBound::initNodeBound(MDPNode& cn)
{
  if (cn.isTerminal) {
    cn.ubVal = 0;
  } else {
    cn.ubVal = initBound->getValue(cn.s, NULL);
  }
}

void PointUpperBound::updateSimple(MDPNode& cn, int* maxUBActionP)
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
  // this check may be helpful when there is round-off error or if the
  // UB is not uniformly improvable, but normally maxUBVal will always
  // be smaller
  cn.ubVal = std::min(cn.ubVal, maxUBVal);
#else
  cn.ubVal = maxUBVal;
#endif

  if (NULL != maxUBActionP) *maxUBActionP = maxUBAction;
}

void PointUpperBound::updateUseCache(MDPNode& cn, int* maxUBActionP)
{
  // cache upper bound for each action
  dvector cachedUpperBound(problem->getNumActions());
  FOR (a, problem->getNumActions()) {
    cachedUpperBound(a) = cn.Q[a].ubVal;
  }

  // remember which Q functions we have updated on this call
  std::vector<bool> updatedAction(problem->getNumActions());
  FOR (a, problem->getNumActions()) {
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

  double maxUBVal = cachedUpperBound(maxUBAction);
  
#if 1
  // this check may be helpful when there is round-off error or if the
  // UB is not uniformly improvable, but normally maxUBVal will always
  // be smaller
  cn.ubVal = std::min(cn.ubVal, maxUBVal);
#else
  cn.ubVal = maxUBVal;
#endif

  if (NULL != maxUBActionP) *maxUBActionP = maxUBAction;
}

void PointUpperBound::update(MDPNode& cn, int* maxUBActionP)
{
  if (BP_QVAL_UNDEFINED == cn.Q[0].ubVal) {
    updateSimple(cn, maxUBActionP);
  } else {
    updateUseCache(cn, maxUBActionP);
  }
}

int PointUpperBound::getStorage(int whichMetric) const
{
  switch (whichMetric) {
  case ZMDP_S_NUM_ELTS:
  case ZMDP_S_NUM_ENTRIES:
    return initBound->getStorage(whichMetric);

  case ZMDP_S_NUM_ELTS_TABULAR:
  case ZMDP_S_NUM_ENTRIES_TABULAR:
    return getNodeCacheStorage(core->lookup, whichMetric);

  default:
    assert(0); // never reach this point
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/01/13 00:42:02  trey
 * added upper bound caching like sawtooth
 *
 * Revision 1.1  2006/10/24 02:06:16  trey
 * initial check-in
 *
 *
 ***************************************************************************/
