/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-01-31 18:31:51 $
   
 @file    QMDP.h
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

#ifndef INCQMDP_h
#define INCQMDP_h

#include "MDPValueFunction.h"

namespace pomdp {

struct QMDP : public MDPValueFunction, public Solver {

  const Pomdp* pomdp;
  std::vector< dvector > Q;

  void planInit(const MDP* _pomdp)
  {
    pomdp = (const Pomdp*) _pomdp;
  }

  bool planFixedTime(const belief_vector& currentBelief,
		     double maxTimeSeconds,
		     double minPrecision) 
  {
    std::vector< dvector > nextQ;
    dvector tmp, tmp2, diff;
    timeval tBegin, tEnd;
    double residual, maxResidual;

    gettimeofday( &tBegin, NULL );

    std::cout << "QMDP iteration";
    std::cout.flush();

    Q.resize( pomdp->numActions );
    nextQ.resize( pomdp->numActions );

    FOR (a, pomdp->numActions) {
      Q[a].resize( pomdp->numStates );
      nextQ[a].resize( pomdp->numStates );
    }
    do {
      maxResidual = -99e+20;

      FOR (a, pomdp->numActions) {
	FOR (s, pomdp->numStates) {
	  nextQ[a](s) = -99e+20;
	}
	FOR (ap, pomdp->numActions) {
	  mult( tmp, Q[ap], pomdp->Ttr[a] );
	  tmp *= pomdp->discount;
	  copy_from_column( tmp2, pomdp->R, a );
	  tmp += tmp2;

	  max_assign( nextQ[a], tmp );
	}

	diff = nextQ[a];
	diff -= Q[a];
	residual = norm_inf( diff );

	if (residual > maxResidual) {
	  maxResidual = residual;
	}
      }
      FOR (a, pomdp->numActions) {
	Q[a] = nextQ[a];
      }

      std::cout << ".";
      std::cout.flush();

    } while (maxResidual > 1e-10);

    gettimeofday( &tEnd, NULL );

    std::cout << std::endl;
    std::cout << "QMDP solution time: "
	      << ((tEnd.tv_sec - tBegin.tv_sec)
		  + 1e-6*(tEnd.tv_usec - tBegin.tv_usec))
	      << std::endl;

    return true;
  }

  int chooseAction(const belief_vector& currentBelief)
  {
    double val, maxval = -99e+20;
    int best_action = -1;
    FOR (a, pomdp->numActions) {
      val = inner_prod( Q[a], currentBelief );
      if (val > maxval) {
	maxval = val;
	best_action = a;
      }
    }
    return best_action;
  }

  void setBoundsFile(std::ostream* boundsFile) 
  {
    // do nothing, no bounds
  }

  ValueInterval getValueAt(const belief_vector& currentBelief) const
  {
    // not sure what i wanted to use this for, ignore it for now
    return ValueInterval();
  }
};


void testMDP(void);

}; // namespace pomdp

#endif // INCQMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/01/28 22:01:51  trey
 * fixed PomdpP -> Pomdp* and MDP*
 *
 * Revision 1.4  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.3  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.2  2005/10/21 20:09:11  trey
 * added namespace pomdp
 *
 * Revision 1.1  2005/03/25 16:09:31  trey
 * initial check-in
 *
 *
 ***************************************************************************/
