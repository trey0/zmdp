/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2005-01-21 18:07:02 $
 *  
 * @file    MatrixUtils.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCMatrixUtils_h
#define INCMatrixUtils_h

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

#include "commonDefs.h"
#include "commonTypes.h"

/**********************************************************************
 * MACROS
 **********************************************************************/

#define HASH_VECTOR_PRECISION "%5d:%6.4lf "
#define HASH_VECTOR_LEN (14)

// convenience macros for iterating through compressed matrices and vectors

#define FOR_CM_ROWS(r, M) \
  for (unsigned r=0; r < M.size1(); r++)

#define FOR_CM_COLS(r, M) \
  unsigned __cm_begin = M.index1_data()[r]; \
  unsigned __cm_end   = M.index1_data()[r+1]; \
  for (unsigned __cm_j=__cm_begin; __cm_j < __cm_end; __cm_j++)

#define CM_VAL(M) \
  M.value_data()[__cm_j]

#define CM_COL(M) \
  M.index2_data()[__cm_j]

#define FOR_CV(v) \
  for (unsigned __cv_i=0; __cv_i < v.non_zeros(); __cv_i++)

#define CV_VAL(v) \
  v.value_data()[__cv_i]

#define CV_INDEX(v) \
  v.index_data()[__cv_i]

/**********************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************/

namespace MatrixUtils {
#if USE_UBLAS
  using namespace boost::numeric::ublas;
#else
  using namespace sla;
#endif

  // seed random number generator (may also do other things in the future)
  void init_matrix_utils(void);

  // Generate a sample from a uniform distribution over [0,1].
  double unit_rand(void);

  // Generate a matrix where each sample is drawn according to unit_rand().
  bmatrix rand_matrix(int num_rows, int num_cols);
  bvector rand_vector(int num_entries);

  // Set all entries to zero.
  void set_to_zero(dmatrix& M);
  void set_to_zero(cmatrix& M);
  void set_to_zero(dvector& v);
  void set_to_zero(cvector& v);

  // Initialize a vector from an array.
  bvector make_vector(double* arr, int num_elts);

  // Index of maximum element of a vector
  int argmax_elt(const dvector& v);
  int argmax_elt(const cvector& v);


  // b represents a discrete probability distribution Pr(outcome = i) = b(i).
  // Chooses an outcome according to the distribution.
  int chooseFromDistribution(const dvector& b);
  int chooseFromDistribution(const cvector& b);

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

#if 0
  void write_matrix(const bmatrix& history, const std::string& outfile);
#endif

  // Returns a nice printable representation for big vectors (sorted in order
  //   of decreasing absolute value, with the index of each value labeled).
  std::string sparseRep(const bvector& v);

  // Like sparseRep, but doesn't take absolute value.
  std::string maxRep(const bvector& v);

  // Returns a string representation of b, suitable for hashing
  const char* hashable(const dvector& b);
  const char* hashable(const cvector& b);

  // Calculates the average and standard deviation for a collection.
  template <class _ForwardIterator>
  void calc_avg_stdev_collection(_ForwardIterator start, _ForwardIterator end,
				 double& avg, double& stdev)
  {
    double sum = 0, sqsum = 0;
    int n = 0;
    for (_ForwardIterator i = start; i != end; i++) {
      double x_i = *i;
      sum += x_i;
      sqsum += (x_i * x_i);
      n++;
    }
    avg = sum / n;
    if (n > 1) {
      stdev = sqrt( (sqsum - (sum * sum) / n) / (n-1) );
    } else {
      stdev = -1; // not enough samples
    }
  }
}

#endif // INCMatrixUtils_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.11  2003/09/22 21:42:28  trey
 * made some minor fixes so that algorithm variants to compile
 *
 * Revision 1.10  2003/09/22 18:48:13  trey
 * made several algorithm configurations depend on makefile settings, added extra timing output
 *
 * Revision 1.9  2003/09/20 02:26:09  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.8  2003/09/17 20:54:24  trey
 * seeing good performance on tagAvoid (but some mods since run started...)
 *
 * Revision 1.7  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.6  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.5  2003/09/07 02:28:07  trey
 * started to adapt for boost matrix library
 *
 * Revision 1.4  2003/07/23 20:48:31  trey
 * added eabs and scalar_mat_mult
 *
 * Revision 1.3  2003/07/18 16:08:00  trey
 * finished implementing Q caching
 *
 * Revision 1.2  2003/07/16 16:05:44  trey
 * added avg/stdev calculation
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
