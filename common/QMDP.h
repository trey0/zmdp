/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2005-10-28 02:51:40 $
   
 @file    QMDP.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * This code and derivatives may not be incorporated into commercial
   products without specific prior written permission.
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * The name of Trey Smith may not be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ***************************************************************************/

#ifndef INCQMDP_h
#define INCQMDP_h

#include "MDPValueFunction.h"

namespace pomdp {

struct QMDP : public MDPValueFunction, public Solver {

  PomdpP pomdp;
  std::vector< dvector > Q;

  void planInit(PomdpP _pomdp)
  {
    pomdp = _pomdp;
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
 * Revision 1.2  2005/10/21 20:09:11  trey
 * added namespace pomdp
 *
 * Revision 1.1  2005/03/25 16:09:31  trey
 * initial check-in
 *
 *
 ***************************************************************************/
