/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-13 23:29:44 $
 *  
 * PROJECT: FIRE Architecture Project
 *
 * @file    Solver.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCSolver_h
#define INCSolver_h

#include "PomdpSim.h"

class Solver {
public:
  // sets up the problem
  virtual void planInit(PomdpP pomdp) = 0;
  // plan for a fixed amount of time.  if maxTimeSeconds < 0,
  //   the amount of time is chosen by the solver to optimize
  //   time performance.  returns true if minPrecision has been
  //   reached.
  virtual bool planFixedTime(const belief_vector& currentBelief,
			     double maxTimeSeconds,
			     double minPrecision) = 0;
  // choose the best action given the current belief.
  virtual int chooseAction(const belief_vector& currentBelief) = 0;
  virtual void setBoundsFile(std::ostream* boundsFile) = 0;
  virtual ValueInterval getValueAt(const belief_vector& currentBelief) const = 0;
};

#endif // INCSolver_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.1  2003/09/23 21:11:51  trey
 * initial check-in
 *
 * Revision 1.1  2001/08/27 17:49:16  trey
 * initial check-in
 *
 *
 ***************************************************************************/
