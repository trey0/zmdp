/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-13 23:29:44 $
 *  
 * PROJECT: FIRE Architecture Project
 *
 * @file    template.cc
 * @brief   No brief
 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>

#include "commonDefs.h"
#include "pomdp.h"
#include "PomdpM.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

void PomdpM::readFromFile(const string& fileName) {
  cout << "reading problem from " << fileName << endl;
  Pomdp p;
  p.readFromFile(fileName);
  
  numStates = p.getNumStates();
  numActions = p.getNumActions();
  numObservations = p.getNumObservations();
  discount = p.getDiscount();

  // resize
  initialBelief.resize(numStates);
  isTerminalState.resize(numStates);
  R.resize(numStates, numActions);
  T.resize(numActions);
  Ttr.resize(numActions);
  O.resize(numActions);
  Otr.resize(numActions);
  FOR (a, numActions) {
    T[a].resize(numStates, numStates);
    Ttr[a].resize(numStates, numStates);
    O[a].resize(numStates, numObservations);
    Otr[a].resize(numStates, numObservations);
  }

  // zero
  set_to_zero(initialBelief);
  set_to_zero(R);
  FOR (a, numActions) {
    set_to_zero(T[a]);
    set_to_zero(Ttr[a]);
    set_to_zero(O[a]);
    set_to_zero(Otr[a]);
  }

  // copy
  FOR (s, numStates) {
    VEC_ASSIGN_CHECK_ZERO( initialBelief(s), p.getInitialBelief(s) );
    isTerminalState[s] = p.isTerminalState(s);
    FOR (a, numActions) {
      VEC_ASSIGN_CHECK_ZERO( R(s,a), p.R(s,a) );
      FOR (sp, numStates) {
	VEC_ASSIGN_CHECK_ZERO( T[a](s,sp), p.T(s,a,sp) );
      }
      FOR (o, numObservations) {
	VEC_ASSIGN_CHECK_ZERO( O[a](s,o), p.O(s,a,o) );
      }
    }
  }

  // set up transpose
  FOR (a, numActions) {
    Ttr[a] = trans(T[a]);
    Otr[a] = trans(O[a]);
  }

  // debug output: what's the density of T and O?
  int T_size = 0;
  int T_non_zeros = 0;
  int O_size = 0;
  int O_non_zeros = 0;
  FOR (a, numActions) {
    T_size += T[a].size1() * T[a].size2();
    O_size += O[a].size1() * O[a].size2();
#if !NO_COMPRESSED_MATRICES
    T_non_zeros += T[a].non_zeros();
    O_non_zeros += O[a].non_zeros();
#endif
  }
#if !NO_COMPRESSED_MATRICES
  cout << "T density = " << (((double) T_non_zeros) / T_size)
       << ", O density = " << (((double) O_non_zeros) / O_size)
       << endl;
#endif
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/11/09 21:31:59  trey
 * got pomdp source tree into a building state again
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.7  2003/09/22 21:42:28  trey
 * made some minor fixes so that algorithm variants to compile
 *
 * Revision 1.6  2003/09/20 02:26:10  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.5  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/07 02:28:07  trey
 * started to adapt for boost matrix library
 *
 * Revision 1.2  2003/07/16 16:07:37  trey
 * added isTerminalState
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
