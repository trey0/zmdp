/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-24 20:48:04 $
 *  
 * @file    MDPValueIteration.cc
 * @brief   No brief
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

alpha_vector MDPValueFunction::nextAlphaAction(int a) {
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
  return matrix_column<bmatrix>( pomdp->R, a )
    + pomdp->discount * prod( pomdp->T[a], alpha );
}

double MDPValueFunction::valueIterationOneStep(void) {
  alpha_vector nextAlpha(numStates), naa(numStates);
  nextAlpha = nextAlphaAction(0);
  FOR (a, pomdp->numActions) {
    naa = nextAlphaAction(a);
    FOR (s, numStates) {
      if (naa(s) > nextAlpha(s)) nextAlpha(s) = naa(s);
    }
  }

  // norm_inf(v) = max_i |v_i|
  double maxResidual = norm_inf(nextAlpha - alpha);
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
    if (residual / (1 - pomdp->discount) < eps) {
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
  PomdpM* p = new PomdpM();
  //p->readFromFile("examples/simple.pomdp");
  p->readFromFile("examples/hallway.pomdp");
  m.valueIteration(p, 1e-3);
  cout << "final alpha = " << m.alpha << endl;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
