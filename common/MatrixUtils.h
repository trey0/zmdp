/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.4 $  $Author: trey $  $Date: 2005-01-27 05:30:56 $
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
#include <sstream>

#include "commonDefs.h"
#include "commonTypes.h"

#if USE_UBLAS
#  include "ublasMatrixUtils.h"
#else
#  include "slaMatrixUtils.h"
#endif

namespace MatrixUtils {

  /**********************************************************************
   * FUNCTION PROTOTYPES
   **********************************************************************/

  // seed random number generator (may also do other things in the future)
  void init_matrix_utils(void);

  // Generate a sample from a uniform distribution over [0,1].
  double unit_rand(void);

  // Generate a matrix where each sample is drawn according to unit_rand().
  void rand_matrix(dmatrix& result, int num_rows, int num_cols);
  void rand_vector(dvector& v, int num_entries);

  // b represents a discrete probability distribution Pr(outcome = i) = b(i).
  // Chooses an outcome according to the distribution.
  int chooseFromDistribution(const dvector& b);
  int chooseFromDistribution(const cvector& b);

  // Returns a string representation of b, suitable for hashing
  const char* hashable(const dvector& b);
  const char* hashable(const cvector& b);

  // Calculates the average and standard deviation for a collection.
  template <class _ForwardIterator>
  void calc_avg_stdev_collection(_ForwardIterator start, _ForwardIterator end,
				 double& avg, double& stdev);

  // Returns a nice printable representation for big vectors (sorted in order
  //   of decreasing absolute value, with the index of each value labeled).
  std::string sparseRep(const cvector& v);

  /**********************************************************************
   * FUNCTIONS
   **********************************************************************/

  inline void init_matrix_utils(void)
  {
    srand(time(0));
  }

  // Generate a sample from a uniform distribution over [0,1].
  inline double unit_rand(void)
  {
    return ((double)std::rand())/RAND_MAX;
  }

  // Generate a matrix where each sample is drawn according to unit_rand().
  inline void rand_matrix(dmatrix& result, int num_rows, int num_cols)
  {
    result.resize(num_rows, num_cols);
    FOR (r, num_rows) {
      FOR (c, num_cols) {
	result(r,c) = unit_rand();
      }
    }
  }

  inline void rand_vector(dvector& v, int num_entries)
  {
    v.resize(num_entries);
    FOR (i, num_entries) {
      v(i) = unit_rand();
    }
  }

  // b represents a discrete probability distribution Pr(outcome = i) = b(i).
  // Chooses an outcome according to the distribution.
  inline int chooseFromDistribution(const dvector& b) {
    double r = unit_rand();
    FOR (i, b.size()) {
      r -= b(i);
      if (r <= 0) return i;
    }
    // should never reach this point if b is normalized properly
    //   (up to round-off error)
    assert(r < 1e-10);
    return 0;
  }

  inline int chooseFromDistribution(const cvector& b) {
    double r = unit_rand();
    FOR_CV(b) {
      r -= CV_VAL(b);
      if (r <= 0) return CV_INDEX(b);
    }
    // should never reach this point if b is normalized properly
    //   (up to round-off error)
    assert(r < 1e-10);
    return 0;
  }

  inline const char* hashable(const dvector& b)
  {
    static char *buf;
    static unsigned int buf_size = 0;
    unsigned int n = b.size();
    if (0 == buf) {
      buf_size = n*HASH_VECTOR_LEN+1;
      buf = new char[buf_size];
    }
    assert(buf_size >= n*HASH_VECTOR_LEN+1);

    char *pos = buf;
    FOR (i, b.size()) {
      pos += snprintf(pos, HASH_VECTOR_LEN, HASH_VECTOR_PRECISION,
		      i, b(i));
    }
    return buf;
  }

  inline const char* hashable(const cvector& b)
  {
    static char *buf;
    static unsigned int buf_size = 0;
    unsigned int n = b.size();
    if (0 == buf) {
      buf_size = n*HASH_VECTOR_LEN+1;
      buf = new char[buf_size];
    }
    assert(buf_size >= n*HASH_VECTOR_LEN+1);

    char *pos = buf;
    FOR_CV (b) {
      pos += snprintf(pos, HASH_VECTOR_LEN, HASH_VECTOR_PRECISION,
		      CV_INDEX(b), CV_VAL(b));
    }
    return buf;
  }

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
    FOR_CV (v) {
      sorted.push_back(IndPair(CV_INDEX(v), CV_VAL(v)));
    }
    sort(sorted.begin(), sorted.end(), AbsValGreater());
    std::ostringstream out;
    int num_to_print = std::min((unsigned)8, sorted.size());
    if ( 0 == num_to_print) {
      out << "(no non-zero entries)";
    } else {
      FOR (i, num_to_print) {
	out << sorted[i].ind << ":" << sorted[i].val << " ";
      }
    }
    return out.str();
  }

  inline std::string sparseRep(const dvector& v) {
    std::vector<IndPair> sorted;
    FOR (i, v.size()) {
      sorted.push_back(IndPair(i,v(i)));
    }
    sort(sorted.begin(), sorted.end(), AbsValGreater());
    std::ostringstream out;
    int num_to_print = std::min((unsigned)8, sorted.size());
    if ( 0 == num_to_print) {
      out << "(no non-zero entries)";
    } else {
      FOR (i, num_to_print) {
	out << sorted[i].ind << ":" << sorted[i].val << " ";
      }
    }
    return out.str();
  }

}; // namespace MatrixUtils

#endif // INCMatrixUtils_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/01/26 04:09:25  trey
 * moved some common code from sla and ublasMatrixUtils back into MatrixUtils.h
 *
 * Revision 1.2  2005/01/21 18:07:02  trey
 * preparing for transition to sla matrix types
 *
 *
 ***************************************************************************/
