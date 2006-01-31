/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-01-31 19:18:24 $
   
 @file    PomdpLowerBound.cc
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

#include "pomdpCommonDefs.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "PomdpLowerBound.h"
#include "MDPValueFunction.h"
#if USE_RS_NATIVE
#  include "RSDynamics.h"
#endif

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define PRUNE_EPS (1e-10)
#define INIT1_RESIDUAL (1e-10)
#define INIT2_RESIDUAL (1e-3)

namespace pomdp {

void PomdpLowerBound::initialize(const MDP* problem)
{
  initBlind(problem);
}

double PomdpLowerBound::getValue(const belief_vector& b) const
{
  double bestVal = -99e+20;
  FOR_EACH (alp, alphas) {
    double val = inner_prod(*alp, b);
    if (val > bestVal) {
      bestVal = val;
    }
  }
  return bestVal;
}

void PomdpLowerBound::initBlindWorstCase(const MDP* problem)
{
  Pomdp* pomdp = (Pomdp*) problem;

  // set alpha to be a lower bound on the value of the best blind policy

  double worstStateVal;
  int safestAction = -1;
  double worstCaseReward = -99e+20;
  // calculate worstCaseReward = max_a min_s R(s,a)
  // safestAction = argmax_a min_s R(s,a)
  FOR (a, problem->getNumActions()) {
    worstStateVal = 99e+20;
    FOR (s, pomdp->numStates) {
      worstStateVal = std::min(worstStateVal, pomdp->R(s,a));
    }
    if (worstStateVal > worstCaseReward) {
      safestAction = a;
      worstCaseReward = worstStateVal;
    }
  }
  dvector worstCaseDVector(problem->getNumStateDimensions());
  assert(problem->getDiscount() < 1);
  double worstCaseLongTerm = worstCaseReward / (1 - problem->getDiscount());
  FOR (i, pomdp->numStates) {
    worstCaseDVector(i) = worstCaseLongTerm;
  }
  cvector calpha;
  copy(calpha, worstCaseDVector);
  alphas.push_back(calpha);
  cout << "initLowerBoundBlindWorstCase: alpha=" << sparseRep(calpha) << endl;
}

void PomdpLowerBound::initBlind(const MDP* problem)
{
  Pomdp* pomdp = (Pomdp*) problem;

  alpha_vector al(pomdp->numStates);
  alpha_vector nextAl, tmp, diff;
  alpha_vector weakAl;
  belief_vector dummy; // ignored
  double maxResidual;

  initBlindWorstCase(problem);
  weakAl = alphas[0];
  alphas.clear();

  // produce one alpha vector for each fixed policy "always take action a"
  FOR (a, pomdp->numActions) {
    al = weakAl;

    do {
      // calculate nextAl
      //mult( nextAl, al, pomdp->Ttr[a] );
      mult( nextAl, pomdp->T[a], al );
      nextAl *= pomdp->discount;
      copy_from_column( tmp, pomdp->R, a );
      nextAl += tmp;

      // calculate residual
      diff = nextAl;
      diff -= al;
      maxResidual = norm_inf(diff);

      al = nextAl;
    } while (maxResidual > INIT2_RESIDUAL);

#if 1
    cout << "initLowerBoundBlind: a=" << a << " al=" << sparseRep(al) << endl;
#endif

    alphas.push_back(al);
  }
}

}; // namespace pomdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
