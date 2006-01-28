/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.7 $  $Author: trey $  $Date: 2006-01-28 03:04:39 $
   
 @file    PomdpSim.cc
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

#include <iostream>
#include <sstream>

#include "PomdpSim.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

namespace pomdp {

PomdpSim::PomdpSim(Pomdp* _pomdp) :
  pomdp(_pomdp)
{
  simOutFile = NULL;
  restart();
}

// sets us back to the initial belief (choosing state randomly according to
// initial belief) and zeros elapsed time and reward
void PomdpSim::restart(void) {
  elapsedTime = 0;
  rewardSoFar = 0;
  terminated = false;
  lastState = -1;
  currentBelief.resize(pomdp->initialBelief.size());
  currentBelief = pomdp->initialBelief;
  state = chooseFromDistribution(currentBelief);
  if (simOutFile) {
    (*simOutFile) << ">>> begin" << endl;
  }
}

// when BELIEF_SIM is non-zero, the sim tracks a belief state instead of
// a discrete physical state
#define BELIEF_SIM 0

void PomdpSim::performAction(int a) {
  double r;
  int sp, o;
  cvector Ta_times_b, tau;
  double imm_reward;

  if (terminated) {
    cerr << "ERROR: trying to perform action when simulation is terminated"
	 << " (elapsedTime=" << elapsedTime << ", lastState=" << lastState << ")"
	 << endl;
    exit(EXIT_FAILURE);
  }

  // increment reward
#if BELIEF_SIM
  imm_reward = inner_prod( matrix_column<bmatrix>(pomdp->R,a),
				  currentBelief );
#else
  imm_reward = pomdp->R(state, a);
#endif

  rewardSoFar += pow(pomdp->discount, elapsedTime) * imm_reward;

#if 0
  rval = pomdp->R(state, a);
  if (rval > 0) {
    cout << "got non-zero reward rval=" << rval << ","
	 << " (elapsedTime=" << elapsedTime << ", state=" << state << ")"
	 << endl;
  }
#endif

  // figure out actual next state and observation
#if BELIEF_SIM
  // in belief-sim mode, regenerate state each time from current belief distribution
  state = chooseFromDistribution(currentBelief);
#endif
#define CHOOSE_EPS (1e-10)

  // draw next state sp based on current state and T
  r = unit_rand();
  sp = 0;
  FOR (i, pomdp->numStates) {
    r -= pomdp->Ttr[a](i,state);
    if (r < 0) {
      sp = i;
      break;
    }
  }
  assert( r < CHOOSE_EPS );

  // draw observation o based on sp ond O
  r = unit_rand();
  o = 0;
  FOR (i, pomdp->numOutcomes) {
    r -= pomdp->O[a](sp,i);
    if (r < 0) {
      o = i;
      break;
    }
  }
  assert( r < CHOOSE_EPS );

#if 0
  cout << "sim: s=" << state << ", a=" << a << ", sp=" << sp << ", o=" << o << endl;
  cout << "  " << sparseRep(currentBelief) << endl;
#endif
  if (simOutFile) {
    (*simOutFile) << "sim: " << state << " " << a << " " << sp << " " << o << endl;
  }

  // change state and elapsed time
  state = sp;
  elapsedTime++;

  // we used to check termination *before* the transition to take into account
  // weirdness in the hallway and hallway2 problems; but that's resolved now.
  if (pomdp->isPomdpTerminalState[state]) {
    terminated = true;
    lastState = state;
    if (simOutFile) {
      (*simOutFile) << "terminated" << endl;
    }
  }

  // update the belief according to Bayes rule, using only knowledge of the
  //   observation.  technically, belief tracking could be considered part
  //   of the solution algorithm, but it seems ok to do it here for convenience.

  // Ta_times_b = pomdp->Ttr[a] * currentBelief
  mult( Ta_times_b, pomdp->Ttr[a], currentBelief );

  // tau = O(:,o) .* Ta_times_b
  emult_column( tau, pomdp->O[a], o, Ta_times_b );

  tau *= (1.0 / norm_1(tau));
  currentBelief = tau;

#if 0
  currentBelief = eprod( belief_vector(matrix_column<bmatrix>(pomdp->O[a], o)),
			 belief_vector(prod( currentBelief, pomdp->T[a] )) );
  currentBelief /= norm_1(currentBelief);
#endif

  if (simOutFile) {
    (*simOutFile) << "belief: " << sparseRep(currentBelief) << endl;
    simOutFile->flush();
  }
}

}; // namespace pomdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.5  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.4  2005/10/21 20:08:53  trey
 * added namespace pomdp
 *
 * Revision 1.3  2005/01/28 03:19:38  trey
 * switched to use emult_column for efficiency
 *
 * Revision 1.2  2005/01/27 05:33:20  trey
 * modified for sla compatibility
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.8  2003/09/22 18:48:13  trey
 * made several algorithm configurations depend on makefile settings, added extra timing output
 *
 * Revision 1.7  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.6  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.5  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.4  2003/07/24 15:33:44  trey
 * working test configuration for self-update
 *
 * Revision 1.3  2003/07/23 20:49:17  trey
 * published/fixed sparseRep
 *
 * Revision 1.2  2003/07/22 19:59:38  trey
 * several minor fixes; still observing discrepancy between sim and solver lower bound
 *
 * Revision 1.1  2003/07/16 16:09:35  trey
 * initial check-in
 *
 *
 ***************************************************************************/
