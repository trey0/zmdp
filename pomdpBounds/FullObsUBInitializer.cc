/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2006-10-30 20:00:15 $
   
 @file    FullObsUBInitializer.cc
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
#include <math.h>

#include <algorithm>
#include <iostream>

#include "FullObsUBInitializer.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

void FullObsUBInitializer::nextAlphaAction(dvector& result, int a) {
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

  mult( result, alpha, pomdp->Ttr[a] );
  result *= pomdp->discount;
  copy_from_column( R_xa, pomdp->R, a );
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

  nextAlphaAction(nextAlpha,0);
  FOR (a, pomdp->numActions) {
    nextAlphaAction(naa,a);
    FOR (s, pomdp->getBeliefSize()) {
      if (naa(s) > nextAlpha(s)) nextAlpha(s) = naa(s);
    }
  }

  tmp.resize( alpha.size() );
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

void FullObsUBInitializer::valueIteration(const Pomdp* _pomdp, double eps) {
  pomdp = _pomdp;

  alpha.resize(pomdp->getBeliefSize());
  set_to_zero(alpha);

  double residual;
  if (zmdpDebugLevelG >= 1) {
    cout << "using mdp value iteration to generate initial upper bound" << endl
	 << "mdp";
    cout.flush();
  }
  FOR (i, MDP_MAX_ITERS) {
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
       << "failed to reach desired eps of " << eps << " after "
       << MDP_MAX_ITERS << " iterations" << endl;
  cout << "residual = " << residual << endl;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.3  2006/04/27 23:10:17  trey
 * put some output in USE_DEBUG_PRINT
 *
 * Revision 1.2  2006/04/06 04:10:14  trey
 * removed obsolete testMDP() function
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.2  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2006/01/31 18:31:50  trey
 * moved many files from common to pomdpCore
 *
 * Revision 1.11  2006/01/28 22:02:48  trey
 * switched Pomdp* -> const Pomdp* in valueIteration()
 *
 * Revision 1.10  2006/01/28 03:02:45  trey
 * replaced PomdpP -> Pomdp*
 *
 * Revision 1.9  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.8  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.7  2005/10/27 22:06:19  trey
 * renamed PomdpM to Pomdp
 *
 * Revision 1.6  2005/10/21 20:07:17  trey
 * added namespace zmdp
 *
 * Revision 1.5  2005/03/28 18:11:40  trey
 * changed definition of residual
 *
 * Revision 1.4  2005/02/08 23:53:04  trey
 * updated to work for alpha_vector = cvector
 *
 * Revision 1.3  2005/01/28 03:17:07  trey
 * fixed bug by zeroing tmp, switched to use -= operator
 *
 * Revision 1.2  2005/01/27 05:30:10  trey
 * modified for sla compatibility
 *
 * Revision 1.1  2004/11/24 20:48:04  trey
 * moved to common from hsvi
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.3  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.2  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.1  2003/06/26 15:41:20  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
