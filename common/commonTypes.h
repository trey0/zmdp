/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.3 $  $Author: trey $  $Date: 2005-01-21 18:07:02 $
 *  
 * @file    commonTypes.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCcommonTypes_h
#define INCcommonTypes_h

#if USE_UBLAS
# include "ublasMatrixTypes.h"
#else
# include "sla.h"
#endif

// select storage types of the data structures we use
typedef cvector belief_vector;
typedef dvector alpha_vector;
typedef dvector obs_prob_vector;

struct ValueInterval {
  double l, u;

  ValueInterval(void) {}
  ValueInterval(double _l, double _u) : l(_l), u(_u) {}
  bool overlapsWith(const ValueInterval& rhs) const {
    return (l <= rhs.u) && (rhs.l <= u);
  }
  double width(void) const {
    return u - l;
  }
};
std::ostream& operator<<(std::ostream& out, const ValueInterval& v);

#endif // INCcommonTypes_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2005/01/21 15:22:02  trey
 * added include of ublas/operation.hpp, allowing use of axpy_prod()
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.3  2003/09/22 18:48:14  trey
 * made several algorithm configurations depend on makefile settings, added extra timing output
 *
 * Revision 1.2  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.1  2003/09/11 01:45:50  trey
 * initial check-in
 *
 *
 ***************************************************************************/
