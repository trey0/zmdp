/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.3 $  $Author: trey $  $Date: 2005-02-08 23:53:04 $
 *  
 * @file    MDPValueFunction.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCMDPValueFunction_h
#define INCMDPValueFunction_h

#include "PomdpM.h"
#include "ValueFunction.h"

#define MDP_MAX_ITERS (1000)

class MDPValueFunction : public ValueFunction {
public:
  dvector alpha;
  PomdpP pomdp;

  ValueInterval getValueAt(const belief_vector& b) const {
    return ValueInterval(0, inner_prod( alpha, b ));
  }

  void nextAlphaAction(dvector& result, int a);
  double valueIterationOneStep(void);
  void valueIteration(PomdpP _pomdp, double eps);
};


void testMDP(void);


#endif // INCMDPValueFunction_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
