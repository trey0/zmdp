/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-13 23:29:44 $
 *  
 * @file    PomdpSim.cc
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

#include <iostream>
#include <sstream>

#include "PomdpSim.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

PomdpSim::PomdpSim(PomdpP _pomdp) :
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
  if (terminated) {
    cerr << "ERROR: trying to perform action when simulation is terminated"
	 << " (elapsedTime=" << elapsedTime << ", lastState=" << lastState << ")"
	 << endl;
    exit(EXIT_FAILURE);
  }

  // increment reward
#if BELIEF_SIM
  double imm_reward = inner_prod( matrix_column<bmatrix>(pomdp->R,a),
				  currentBelief );
#else
  double imm_reward = pomdp->R(state, a);
#endif
  rewardSoFar += pow(pomdp->discount, elapsedTime) * imm_reward;
#if 0
  double rval = pomdp->R(state, a);
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
  // calls to belief_vector and obs_prob_vector remove type ambiguity in call to
  //   chooseFromDistribution()
  int sp = chooseFromDistribution
    ( belief_vector(matrix_row<bmatrix>(pomdp->T[a], state)) );
  int o = chooseFromDistribution
    ( obs_prob_vector(matrix_row<bmatrix>(pomdp->O[a], sp)) );

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
  if (pomdp->isTerminalState[state]) {
    terminated = true;
    lastState = state;
    if (simOutFile) {
      (*simOutFile) << "terminated" << endl;
    }
  }

  // update the belief according to Bayes rule, using only knowledge of the
  //   observation.  technically, belief tracking could be considered part
  //   of the solution algorithm, but it seems ok to do it here for convenience.
  currentBelief = eprod( belief_vector(matrix_column<bmatrix>(pomdp->O[a], o)),
			 belief_vector(prod( currentBelief, pomdp->T[a] )) );
  currentBelief /= norm_1(currentBelief);

  if (simOutFile) {
    (*simOutFile) << "belief: " << sparseRep(currentBelief) << endl;
    simOutFile->flush();
  }
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
