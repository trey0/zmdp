/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-27 23:08:40 $
   
 @file    BlindLBInitializer.cc
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "BlindLBInitializer.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define PRUNE_EPS (1e-10)

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
  assert(pomdp->getDiscount() < 1);
  double worstCaseLongTerm = worstCaseReward / (1 - pomdp->getDiscount());
  FOR (i, pomdp->numStates) {
    worstCaseDVector(i) = worstCaseLongTerm;
  }
  cvector calpha;

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
  belief_vector dummy; // ignored
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
    bound->addLBPlane(dummy, al, a, full_mask);
#else 
    bound->addLBPlane(dummy, al, a);
#endif
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
