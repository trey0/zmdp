/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-10-03 03:18:36 $
   
 @file    BlindLBInitializer.cc
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "BlindLBInitializer.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define PRUNE_EPS (1e-10)
#define POMDP_LONG_TERM_UNBOUNDED (99e+20)

namespace zmdp {

BlindLBInitializer::BlindLBInitializer(const MDP* _pomdp, MaxPlanesLowerBound* _bound) {
  pomdp = (const Pomdp*) _pomdp;
  bound = _bound;
}

void BlindLBInitializer::initialize(double targetPrecision)
{
  initBlind(targetPrecision);
}

void BlindLBInitializer::initBlindWorstCase(alpha_vector& weakAlpha)
{
  // set alpha to be a lower bound on the value of the best blind policy

  double worstStateVal;
  int safestAction = -1;
  double worstCaseReward = -99e+20;
  // calculate worstCaseReward = max_a min_s R(s,a)
  // safestAction = argmax_a min_s R(s,a)
  FOR (a, pomdp->getNumActions()) {
    worstStateVal = 99e+20;
    FOR (s, pomdp->numStates) {
      worstStateVal = std::min(worstStateVal, pomdp->R(s,a));
    }
    if (worstStateVal > worstCaseReward) {
      safestAction = a;
      worstCaseReward = worstStateVal;
    }
  }
  dvector worstCaseDVector(pomdp->getNumStateDimensions());

  double longTermFactor = POMDP_LONG_TERM_UNBOUNDED;
  if (pomdp->getDiscount() < 1.0) {
    longTermFactor = std::min(longTermFactor, 1.0 / (1.0 - pomdp->getDiscount()));
  }
  if (pomdp->maxHorizon != -1) {
    longTermFactor = std::min(longTermFactor, (double) pomdp->maxHorizon);
  }
  assert(longTermFactor != POMDP_LONG_TERM_UNBOUNDED);
  
  double worstCaseLongTerm = worstCaseReward * longTermFactor;
  FOR (i, pomdp->numStates) {
    worstCaseDVector(i) = worstCaseLongTerm;
  }

  // post-process: make sure the value for all terminal states
  // is exactly 0, since that is how the lbVal field of terminal
  // nodes is initialized.
  FOR (i, pomdp->numStates) {
    if (pomdp->isPomdpTerminalState[i]) {
      worstCaseDVector(i) = 0.0;
    }
  }

  copy(weakAlpha, worstCaseDVector);
#if USE_DEBUG_PRINT
  cout << "initLowerBoundBlindWorstCase: alpha=" << sparseRep(weakAlpha) << endl;
#endif
}

void BlindLBInitializer::initBlind(double targetPrecision)
{
  alpha_vector al(pomdp->numStates);
  alpha_vector nextAl, tmp, diff;
  alpha_vector weakAl;
  double maxResidual;
#if USE_MASKED_ALPHA
  alpha_vector full_mask;

  mask_set_all( full_mask, pomdp->numStates );
#endif

  initBlindWorstCase(weakAl);
  bound->planes.clear();

  // produce one alpha vector for each fixed policy "always take action a"
  FOR (a, pomdp->numActions) {
    al = weakAl;

    do {
      // calculate nextAl
      mult(nextAl, pomdp->T[a], al);
      nextAl *= pomdp->discount;
      copy_from_column(tmp, pomdp->R, a);
      nextAl += tmp;

      // calculate residual
      diff = nextAl;
      diff -= al;
      maxResidual = norm_inf(diff);

      al = nextAl;
    } while (maxResidual > targetPrecision);

#if USE_DEBUG_PRINT
    cout << "initLowerBoundBlind: a=" << a << " al=" << sparseRep(al) << endl;
#endif

#if USE_MASKED_ALPHA
    bound->addLBPlane(new LBPlane(al, a, full_mask));
#else 
    bound->addLBPlane(new LBPlane(al, a));
#endif
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/07/14 15:08:34  trey
 * removed belief argument to addLBPlane()
 *
 * Revision 1.4  2006/06/03 10:58:45  trey
 * added exact initialization rule for terminal states
 *
 * Revision 1.3  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.2  2006/04/27 23:08:40  trey
 * put some output in USE_DEBUG_PRINT
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.4  2006/02/14 19:33:55  trey
 * added targetPrecision argument for bounds initialization
 *
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:13:45  trey
 * changed when MDP* arguments are passed into bounds initialization
 *
 * Revision 1.1  2006/01/31 19:18:24  trey
 * initial check-in
 *
 *
 ***************************************************************************/
