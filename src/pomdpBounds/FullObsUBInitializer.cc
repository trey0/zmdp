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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>

#include "FullObsUBInitializer.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

void FullObsUBInitializer::nextAlphaAction(dvector &result, int a) {
#if 0
  alpha_vector x(numStates), y(numStates);
  x = matrix_column<bmatrix>( pomdp->R, a );
  y = pomdp->discount * prod( pomdp->T[a], alpha );
  cout << "x = " << maxRep(x) << endl;
  cout << "y = " << maxRep(y) << endl;
#endif
#if 0
  alpha_vector sum(numStates);
  sum = matrix_column<bmatrix>( pomdp->R, a )
    + pomdp->discount * prod( pomdp->T[a], alpha );
  cout << "a = " << a << endl
       << "sum = " << maxRep(sum) << endl;
  return sum;
#endif

  dvector R_xa;

  mult(result, alpha, pomdp->Ttr[a]);
  result *= pomdp->discount;
  copy_from_column(R_xa, pomdp->R, a);
  result += R_xa;

#if 0
  return matrix_column<bmatrix>( pomdp->R, a )
    + pomdp->discount * prod( pomdp->T[a], alpha );
#endif
}

double FullObsUBInitializer::valueIterationOneStep(void) {
  dvector nextAlpha(pomdp->getBeliefSize()), naa(pomdp->getBeliefSize());
  dvector tmp;
  double maxResidual;

  nextAlphaAction(nextAlpha, 0);
  FOR(a, pomdp->numActions) {
    nextAlphaAction(naa, a);
    FOR(s, pomdp->getBeliefSize()) {
      if (naa(s) > nextAlpha(s))
        nextAlpha(s) = naa(s);
    }
  }

  tmp.resize(alpha.size());
  tmp = alpha;
  tmp -= nextAlpha;
  maxResidual = norm_inf(tmp);

#if 0
  // norm_inf(v) = max_i |v_i|
  maxResidual = norm_inf(nextAlpha - alpha);
#endif

#if 0
  cout << "maxResidual = " << maxResidual << endl
       << "alpha = " << maxRep(alpha) << endl;
#endif
  alpha = nextAlpha;

  return maxResidual;
}

void FullObsUBInitializer::valueIteration(const Pomdp *_pomdp, double eps) {
  pomdp = _pomdp;

  alpha.resize(pomdp->getBeliefSize());
  set_to_zero(alpha);

  double residual;
  if (zmdpDebugLevelG >= 1) {
    cout << "using mdp value iteration to generate initial upper bound" << endl
         << "mdp";
    cout.flush();
  }
  FOR(i, MDP_MAX_ITERS) {
    residual = valueIterationOneStep();
    if (zmdpDebugLevelG >= 1) {
      cout << ".";
      cout.flush();
    }
    if (residual < eps) {
      if (zmdpDebugLevelG >= 1) {
        cout << endl;
      }
      return;
    }
  }
  cout << endl
       << "failed to reach desired eps of " << eps << " after " << MDP_MAX_ITERS
       << " iterations" << endl;
  cout << "residual = " << residual << endl;
}

}; // namespace zmdp
