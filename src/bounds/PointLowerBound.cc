/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-01-14 00:53:30 $
   
 @file    PointLowerBound.cc
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
#include "PointLowerBound.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

PointLowerBound::PointLowerBound(const MDP* _problem,
				 const ZMDPConfig* _config,
				 BoundPairCore* _core) :
  problem(_problem),
  config(_config),
  core(_core),
  initBound(NULL)
{}

PointLowerBound::~PointLowerBound(void)
{}

void PointLowerBound::initialize(double _targetPrecision)
{
  targetPrecision = _targetPrecision;
  initBound->initialize(targetPrecision);
}

double PointLowerBound::getValue(const state_vector& s,
				 const MDPNode* cn) const
{
  if (NULL == cn) {
    return initBound->getValue(s, NULL);
  } else {
    return cn->lbVal;
  }
}

void PointLowerBound::initNodeBound(MDPNode& cn)
{
  if (cn.isTerminal) {
    cn.lbVal = 0;
  } else {
    cn.lbVal = initBound->getValue(cn.s, NULL);
  }
}

void PointLowerBound::update(MDPNode& cn)
{
  double lbVal;
  double maxLBVal = -99e+20;

  FOR (a, cn.getNumActions()) {
    MDPQEntry& Qa = cn.Q[a];
    lbVal = 0;
    FOR (o, Qa.getNumOutcomes()) {
      MDPEdge* e = Qa.outcomes[o];
      if (NULL != e) {
	MDPNode& sn = *e->nextState;
	double oprob = e->obsProb;
	lbVal += oprob * sn.lbVal;
      }
    }
    lbVal = Qa.immediateReward + problem->getDiscount() * lbVal;
    Qa.lbVal = lbVal;

    maxLBVal = std::max(maxLBVal, lbVal);
  }
#if 1
  // this check may be helpful when there is round-off error or if the
  // LB is not uniformly improvable, but normally maxLBVal will always
  // be larger
  cn.lbVal = std::max(cn.lbVal, maxLBVal);
#else
  cn.lbVal = maxLBVal;
#endif
}

int PointLowerBound::getStorage(int whichMetric) const
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
 * Revision 1.1  2006/10/24 02:06:16  trey
 * initial check-in
 *
 *
 ***************************************************************************/
