/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-01-26 04:15:20 $
 *  
 * @file    ublasMatrixUtils.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCublasMatrixUtils_h
#define INCublasMatrixUtils_h

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

namespace MatrixUtils {

/**********************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************/

  using namespace MATRIX_NAMESPACE;

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

  // Output matrix R such that R(i,j) = f( M(i,j) )
  template <class _FnType>
  cmatrix mapfun(_FnType f, const cmatrix& M);

  // Output matrix R such that R(i,j) = f( M(i,j) )
  template <class _FnType>
  dmatrix mapfun(_FnType f, const dmatrix& M);

  // Output vector x such that x(i) = f( v(i), w(i) )
  template <class _FnType>
  dvector mapfun2(_FnType f, const dvector& v, const dvector& w);

  template <class _FnType>
  cvector mapfun2(_FnType f, const cvector& v, const cvector& w);

  // Output length-n vector v such that v(i) = f(i)
  template <class _FnType>
  cvector mapfunrange(_FnType f, int n);

  // Return the greater of two values
  template <class _Tp>
  struct max_fun {
    inline const _Tp& operator()(const _Tp& a, const _Tp& b) {
      return (a > b) ? a : b;
    }
  };

  // Element-wise product of two vectors (equivalent to matlab .*)
  dvector eprod(const dvector& v, const dvector& w);
  cvector eprod(const cvector& v, const cvector& w);

  // Return a length-n vector v such that v(i) = i
  cvector range_vector(int n);
#if 0
  void write_matrix(const bmatrix& history, const std::string& outfile);
#endif

  // Returns a nice printable representation for big vectors (sorted in order
  //   of decreasing absolute value, with the index of each value labeled).
  std::string sparseRep(const bvector& v);

  // Like sparseRep, but doesn't take absolute value.
  std::string maxRep(const bvector& v);

  /**********************************************************************
   * FUNCTIONS
   **********************************************************************/

  inline void set_to_zero(dmatrix& M) {
    M = zero_matrix<double>(M.size1(), M.size2());
  }

  inline void set_to_zero(cmatrix& M) {
    bmatrix Mp(M.size1(), M.size2());
    M = Mp;
  }

  inline void set_to_zero(dvector& v) {
    FOR_EACH (elt, v.data()) {
      *elt = 0;
    }
  }

  inline void set_to_zero(cvector& v) {
    bvector vp(v.size());
    v = vp;
  }

  class CompareSecond {
  public:
    bool operator()(const std::pair<size_t, double>& lhs,
		    const std::pair<size_t, double>& rhs) {
      return lhs.second < rhs.second;
    }
  };

  // Initialize a vector from an array.
  inline cvector make_vector(double* arr, int num_elts) {
    cvector v(num_elts);
    FOR (i, num_elts) {
      VEC_ASSIGN_CHECK_ZERO( v(i), arr[i] );
    }
    return v;
  }

  // Index of maximum element of a vector
  inline int argmax_elt(const dvector& v) {
    assert(v.size() > 0);
    double maxval = v(0);
    int max_ind = 0;
    for (unsigned int i=1; i < v.size(); i++) {
      if (v(i) > maxval) {
	max_ind = i;
	maxval = v(i);
      }
    }
    return max_ind;
  }

  inline int argmax_elt(const cvector& v) {
    assert(v.size() > 0);
    double maxval = v(0);
    int max_ind = 0;
    // find the largest non-zero entry
    FOR_CV(v) {
      double val = CV_VAL(v);
      if (val > maxval) {
	max_ind = CV_INDEX(v);
	maxval = val;
      }
    }
    if (maxval >= 0 || v.non_zeros() == v.size()) {
      // a non-zero entry is maximal
      return max_ind;
    } else {
      // 0 is maximal; find a zero entry
      for (unsigned int i=1; i < v.size(); i++) {
	if (v(i) == 0) {
	  return i;
	}
      }
    }
    return max_ind;
  }

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

  inline dvector eprod(const dvector& v, const dvector& w) {
    return mapfun2(std::multiplies<double>(), v, w);
  }
  inline cvector eprod(const cvector& v, const cvector& w) {
    return mapfun2(std::multiplies<double>(), v, w);
  }

  inline cvector range_vector(int n) {
    cvector v(n);
    FOR (i, n) {
      v(i) = i;
    }
    return v;
  }

  struct IndPair {
    int ind;
    double val;
    IndPair(int _ind, double _val) : ind(_ind), val(_val) {}
  };
  
  struct AbsValGreater {
    bool operator()(const IndPair& lhs, const IndPair& rhs) {
      return fabs(lhs.val) > fabs(rhs.val);
    }
  };

  struct ValGreater {
    bool operator()(const IndPair& lhs, const IndPair& rhs) {
      return lhs.val > rhs.val;
    }
  };

  inline std::string sparseRep(const cvector& v) {
    std::vector<IndPair> sorted;
    FOR (i, v.size()) {
      sorted.push_back(IndPair(i, v(i)));
    }
    sort(sorted.begin(), sorted.end(), AbsValGreater());
    std::ostringstream out;
    int num_to_print = std::min((unsigned)8, v.size());
    FOR (i, num_to_print) {
      out << sorted[i].ind << ":" << sorted[i].val << " ";
    }
    return out.str();
  }

  inline std::string maxRep(const cvector& v) {
    std::vector<IndPair> sorted;
    FOR (i, v.size()) {
      sorted.push_back(IndPair(i, v(i)));
    }
    sort(sorted.begin(), sorted.end(), ValGreater());
    std::ostringstream out;
    int num_to_print = std::min((unsigned)8, v.size());
    FOR (i, num_to_print) {
      out << sorted[i].ind << ":" << sorted[i].val << " ";
    }
    return out.str();
  }
}

#endif // INCublasMatrixUtils_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/01/21 18:07:02  trey
 * preparing for transition to sla matrix types
 *
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
