/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-13 23:29:44 $
 *  
 * @file    commonTypes.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCcommonTypes_h
#define INCcommonTypes_h

// if VEC_OPTIM is set (check the makefile), set NDEBUG just for the vector/matrix headers
#if VEC_OPTIM
#  ifdef NDEBUG
#     define VEC_NDEBUG_WAS_DEFINED 1
#  else
#     define NDEBUG 1
#  endif
#endif

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>

// for some reason, this needs to be included after the others
#include <boost/numeric/ublas/io.hpp>

typedef boost::numeric::ublas::vector<double> dvector;
typedef boost::numeric::ublas::matrix<double> dmatrix;

typedef boost::numeric::ublas::compressed_vector<double> cvector;
typedef boost::numeric::ublas::compressed_matrix<double> cmatrix;

#if NO_COMPRESSED_MATRICES

/* dense vector/matrix implementation */
#define VEC_ASSIGN_CHECK_ZERO(x,y) ; { (x) = (y); }

typedef dvector bvector;
typedef dmatrix bmatrix;

#else

/* compressed vector/matrix implementation */
#define VEC_ASSIGN_CHECK_ZERO(x,y) \
  ; { typeof(y) __tmp_y = (y); if (__tmp_y != 0) { (x) = __tmp_y; } }

typedef cvector bvector;
typedef cmatrix bmatrix;

#endif

// select storage types of the data structures we use
typedef cvector belief_vector;
typedef dvector alpha_vector;
typedef dvector obs_prob_vector;

// undefine NDEBUG if it was previously undefined
#if VEC_OPTIM && !VEC_NDEBUG_WAS_DEFINED
#   undef NDEBUG
#endif

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
