/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.9 $  $Author: trey $  $Date: 2005-10-28 03:50:32 $
   
 @file    MDPValueIteration.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

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

#include "MDPValueFunction.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

namespace pomdp {

void MDPValueFunction::nextAlphaAction(dvector& result, int a) {
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

double MDPValueFunction::valueIterationOneStep(void) {
  dvector nextAlpha(numStates), naa(numStates);
  dvector tmp;
  double maxResidual;

  nextAlphaAction(nextAlpha,0);
  FOR (a, pomdp->numActions) {
    nextAlphaAction(naa,a);
    FOR (s, numStates) {
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

void MDPValueFunction::valueIteration(PomdpP _pomdp, double eps) {
  pomdp = _pomdp;
  numStates = pomdp->numStates;

  alpha.resize(numStates);
  set_to_zero(alpha);

  double residual;
  cout << "using mdp value iteration to generate initial upper bound" << endl
       << "mdp";
  cout.flush();
  FOR (i, MDP_MAX_ITERS) {
    residual = valueIterationOneStep();
    cout << ".";
    cout.flush();
    if (residual < eps) {
      cout << endl;
      return;
    }
  }
  cout << endl
       << "failed to reach desired eps of " << eps << " after "
       << MDP_MAX_ITERS << " iterations" << endl;
  cout << "residual = " << residual << endl;
}

void testMDP(void) {
  MDPValueFunction m;
  PomdpP p = new Pomdp();
  //p->readFromFile("examples/simple.pomdp");
  p->readFromFile("examples/hallway.pomdp");
  m.valueIteration(p, 1e-3);
  cout << "final alpha = " << sparseRep(m.alpha) << endl;
}

}; // namespace pomdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.7  2005/10/27 22:06:19  trey
 * renamed PomdpM to Pomdp
 *
 * Revision 1.6  2005/10/21 20:07:17  trey
 * added namespace pomdp
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
