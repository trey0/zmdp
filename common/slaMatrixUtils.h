/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.4 $  $Author: trey $  $Date: 2005-10-21 20:12:23 $
 *  
 * @file    slaMatrixUtils.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCslaMatrixUtils_h
#define INCslaMatrixUtils_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// iostream causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <iostream>

#include <math.h>
#include <vector>
#include <algorithm>
#include <functional>

#include "pomdpCommonDefs.h"
#include "pomdpCommonTypes.h"

/**********************************************************************
 * MACROS
 **********************************************************************/

#define HASH_VECTOR_PRECISION "%5d:%6.4lf "
#define HASH_VECTOR_LEN (14)

// convenience macros for iterating through compressed matrices and vectors

#define FOR_CM_MAJOR(c, M) \
  for (unsigned c=0; c < M.size2(); c++)

#define FOR_CM_MINOR(c, M) \
  typeof(M.data.begin()) __cm_begin = M.data.begin() + M.col_starts[c]; \
  typeof(M.data.begin()) __cm_end   = M.data.begin() + M.col_starts[c+1]; \
  for (typeof(M.data.begin()) __cm_j=__cm_begin; __cm_j != __cm_end; __cm_j++)

#define CM_VAL(M) (__cm_j->value)
#define CM_ROW(c,M) (__cm_j->index)
#define CM_COL(c,M) (c)

#define FOR_CV(v) \
  for (typeof(v.data.begin()) __cv_i=v.data.begin(); \
       __cv_i != v.data.end(); __cv_i++)

#define CV_VAL(v)   (__cv_i->value)

#define CV_INDEX(v) (__cv_i->index)

/**********************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************/

namespace MatrixUtils {
#if USE_UBLAS
  using namespace boost::numeric::ublas;
#else
  using namespace sla;
#endif

  // Set all entries to zero.
  void set_to_zero(dmatrix& M);
  void set_to_zero(kmatrix& M);
  void set_to_zero(cmatrix& M);
  void set_to_zero(dvector& v);
  void set_to_zero(cvector& v);

#if 0
  // Initialize a vector from an array.
  bvector make_vector(double* arr, int num_elts);

  // Index of maximum element of a vector
  int argmax_elt(const dvector& v);
  int argmax_elt(const cvector& v);

  // Output matrix R such that R(i,j) = f( M(i,j) )
  template <class _FnType>
  cmatrix mapfun(_FnType f, const cmatrix& M) {
    cmatrix result(M.size1(), M.size2());
    FOR (r, M.size1()) {
      FOR (c, M.size2()) {
	VEC_ASSIGN_CHECK_ZERO( result(r,c), f(M(r,c)) );
      }
    }
    return result;
  }

  // Output matrix R such that R(i,j) = f( M(i,j) )
  template <class _FnType>
  dmatrix mapfun(_FnType f, const dmatrix& M) {
    dmatrix result(M.size1(), M.size2());
    FOR (r, M.size1()) {
      FOR (c, M.size2()) {
	result(r,c) = f( M(r,c) );
      }
    }
    return result;
  }

  // Output vector x such that x(i) = f( v(i), w(i) )
  template <class _FnType>
  dvector mapfun2(_FnType f, const dvector& v, const dvector& w)
  {
    assert(v.size() == w.size());
    dvector x(v.size());
    FOR (i, v.size()) {
      x(i) = f( v(i), w(i) );
    }
    return x;
  }
  template <class _FnType>
  cvector mapfun2(_FnType f, const cvector& v, const cvector& w)
  {
    assert(v.size() == w.size());
    cvector x(v.size());
    FOR (i, v.size()) {
      VEC_ASSIGN_CHECK_ZERO( x(i), f( v(i), w(i) ) );
    }
    return x;
  }

  // Output length-n vector v such that v(i) = f(i)
  template <class _FnType>
  bvector mapfunrange(_FnType f, int n) {
    bvector result(n);
    FOR (i, n) {
      result(i) = f(i);
    }
    return result;
  }

  // Return the greater of two values
  template <class _Tp>
  struct max_fun {
    inline const _Tp& operator()(const _Tp& a, const _Tp& b) {
      return (a > b) ? a : b;
    }
  };

  // Scale the rows of M, like MATLAB diag(v) * M.
  bmatrix scale_columns(const bvector& v, const dmatrix& M);
  bmatrix scale_columns(const bvector& v, const cmatrix& M);

  // Element-wise maximum of two vectors (equivalent to Matlab max)
  inline dvector emax(const dvector& v, const dvector& w) {
    return mapfun2(max_fun<double>(), v, w);
  }
  inline cvector emax(const cvector& v, const cvector& w) {
    return mapfun2(max_fun<double>(), v, w);
  }

  // Element-wise product of two vectors (equivalent to matlab .*)
  inline dvector eprod(const dvector& v, const dvector& w) {
    return mapfun2(std::multiplies<double>(), v, w);
  }
  inline cvector eprod(const cvector& v, const cvector& w) {
    return mapfun2(std::multiplies<double>(), v, w);
  }

  // Return a length-n vector v such that v(i) = i
  inline bvector range_vector(int n) {
    bvector v(n);
    FOR (i, n) {
      v(i) = i;
    }
    return v;
  }

  // Returns a nice printable representation for big vectors (sorted in order
  //   of decreasing absolute value, with the index of each value labeled).
  std::string sparseRep(const bvector& v);

  // Like sparseRep, but doesn't take absolute value.
  std::string maxRep(const bvector& v);
#endif

  /**********************************************************************
   * FUNCTIONS
   **********************************************************************/

  // Set all entries to zero.
  inline void set_to_zero(dmatrix& M)
  {
    M.resize( M.size1(), M.size2() );
  }

  inline void set_to_zero(kmatrix& M)
  {
    M.resize( M.size1(), M.size2() );
  }

  inline void set_to_zero(cmatrix& M)
  {
    M.resize( M.size1(), M.size2() );
  }

  inline void set_to_zero(dvector& v)
  {
    v.resize( v.size() );
  }

  inline void set_to_zero(cvector& v)
  {
    v.resize( v.size() );
  }

} // namespace MatrixUtils

#endif // INCslaMatrixUtils_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/01/28 03:22:03  trey
 * added some functions, improved ublas compatibility
 *
 * Revision 1.2  2005/01/26 04:16:39  trey
 * major overhaul
 *
 *
 ***************************************************************************/
