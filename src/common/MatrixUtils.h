/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License.  You may
 obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.

 ***************************************************************************/

#ifndef ZMDP_SRC_COMMON_MATRIXUTILS_H_
#define ZMDP_SRC_COMMON_MATRIXUTILS_H_

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// iostream causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <math.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "slaMatrixUtils.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

namespace MatrixUtils {

/**********************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************/

// seed random number generator (may also do other things in the future)
void init_matrix_utils(void);

// Generate a sample from a uniform distribution over [0,1].
double unit_rand(void);

// Generate a matrix where each sample is drawn according to unit_rand().
void rand_matrix(dmatrix &result, int num_rows, int num_cols);
void rand_vector(dvector &v, int num_entries);

// Index of maximum element of a vector
int argmax_elt(const dvector &v);
int argmax_elt(const cvector &v);

// b represents a discrete probability distribution Pr(outcome = i) = b(i).
// Chooses an outcome according to the distribution.
int chooseFromDistribution(const dvector &b);
int chooseFromDistribution(const cvector &b);

// Returns a string representation of b, suitable for hashing
const char *hashable(const dvector &b);
const char *hashable(const cvector &b);

// Calculates the average and standard deviation for a collection.
template <class _ForwardIterator>
void calc_avg_stdev_collection(_ForwardIterator start, _ForwardIterator end,
                               double &avg, double &stdev);

// Returns a nice printable representation for big vectors (sorted in order
//   of decreasing absolute value, with the index of each value labeled).
std::string sparseRep(const cvector &v, int num_to_print = 4);
std::string sparseRep(const dvector &v);

// Returns a full printable representation of v.  Probably not best to use
//   with large vectors.
std::string denseRep(const cvector &v);
std::string denseRep(const dvector &v);

/**********************************************************************
 * FUNCTIONS
 **********************************************************************/

inline void init_matrix_utils(void) {
  // Initialize the random number generator
  srand(time(0));
  // Seems the random number generator needs to "clear its throat" on some
  // systems.
  rand();
}

// Generate a sample from a uniform distribution over [0,1].
inline double unit_rand(void) {
  return (static_cast<double>(std::rand())) / RAND_MAX;
}

// Generate a matrix where each sample is drawn according to unit_rand().
inline void rand_matrix(dmatrix &result, int num_rows, int num_cols) {
  result.resize(num_rows, num_cols);
  FOR(r, num_rows) {
    FOR(c, num_cols) { result(r, c) = unit_rand(); }
  }
}

inline void rand_vector(dvector &v, int num_entries) {
  v.resize(num_entries);
  FOR(i, num_entries) { v(i) = unit_rand(); }
}

// Index of maximum element of a vector
inline int argmax_elt(const dvector &v) {
  assert(v.size() > 0);
  double maxval = v(0);
  int max_ind = 0;
  for (unsigned int i = 1; i < v.size(); i++) {
    if (v(i) > maxval) {
      max_ind = i;
      maxval = v(i);
    }
  }
  return max_ind;
}

inline int argmax_elt(const cvector &v) {
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
  if (maxval >= 0 || v.filled() == v.size()) {
    // a non-zero entry is maximal
    return max_ind;
  } else {
    // all non-zero entries are negative; return
    // the index of a zero entry.
    int ind, last_ind = -1;
    FOR_CV(v) {
      ind = CV_INDEX(v);
      if (ind - last_ind > 1) {
        return ind - 1;
      }
      last_ind = ind;
    }
    return ind + 1;
  }
}

// b represents a discrete probability distribution Pr(outcome = i) = b(i).
// Chooses an outcome according to the distribution.
inline int chooseFromDistribution(const dvector &b) {
  double r = unit_rand();
  FOR(i, b.size()) {
    r -= b(i);
    if (r <= 0) return i;
  }
  if (r >= 1e-10) {
    // should never reach this point if b is normalized properly
    //   (up to round-off error)
    printf("chooseFromDistribution: b=%s 1-sum(b)=%g\n", denseRep(b).c_str(),
           1 - sum(b));
    assert(0);
  }
  return 0;
}

inline int chooseFromDistribution(const cvector &b) {
  double r = unit_rand();
  FOR_CV(b) {
    r -= CV_VAL(b);
    if (r <= 0) return CV_INDEX(b);
  }
  if (r >= 1e-10) {
    // should never reach this point if b is normalized properly
    //   (up to round-off error)
    printf("chooseFromDistribution: b=%s 1-sum(b)=%g\n", sparseRep(b).c_str(),
           1 - sum(b));
    assert(0);
  }
  return 0;
}

inline const char *hashable(const dvector &b) {
  static char *buf = NULL;
  static unsigned int n = 0;

  if (b.size() > n) {
    n = b.size();
    if (NULL != buf) delete[] buf;
    buf = new char[n * HASH_VECTOR_LEN + 1];
  }

  char *pos = buf;
  FOR(i, b.size()) {
    pos += snprintf(pos, HASH_VECTOR_LEN, HASH_VECTOR_PRECISION, i, b(i));
  }
  return buf;
}

inline const char *hashable(const cvector &b) {
  static char *buf = NULL;
  static unsigned int n = 0;

  if (b.size() > n) {
    n = b.size();
    if (NULL != buf) delete[] buf;
    buf = new char[n * HASH_VECTOR_LEN + 1];
  }

  char *pos = buf;
  FOR_CV(b) {
    pos += snprintf(pos, HASH_VECTOR_LEN, HASH_VECTOR_PRECISION, CV_INDEX(b),
                    CV_VAL(b));
  }
  return buf;
}

// Calculates the average and standard deviation for a collection.
template <class _ForwardIterator>
void calc_avg_stdev_collection(_ForwardIterator start, _ForwardIterator end,
                               double &avg, double &stdev) {
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
    stdev = sqrt((sqsum - (sum * sum) / n) / (n - 1));
  } else {
    stdev = -1;  // not enough samples
  }
}

// Estimate the proportion-p quantile using the weighted-average method.
inline double getQuantile(const dvector &sortedVals, double p) {
  int n = sortedVals.size();
  double x = p * n - 0.5;
  int i = static_cast<int>(floor(x));
  double r = x - i;
  assert(0 <= i && (i + 1) < n);
  return sortedVals(i) + r * (sortedVals(i + 1) - sortedVals(i));
}

// Uses the bootstrap method to calculate the mean and quantiles
// alpha/2 and (1-alpha/2) for the estimate of the mean.  (For
// example, if alpha=0.05, [quantile1 .. quantile2] is a 95%
// confidence interval for the mean.)
inline void calc_bootstrap_mean_quantile(const dvector &vals, double alpha,
                                         double &mean, double &quantile1,
                                         double &quantile2) {
  int n = vals.size();
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += vals(i);
  }
  mean = sum / n;

  assert(0.0 < alpha && alpha < 1.0);
  const int numSamplesInQuantile = 50;  // totally arbitrary
  int numBootstrapSamples =
      static_cast<int>(numSamplesInQuantile / (alpha / 2));
  dvector bvals(numBootstrapSamples);
  for (int i = 0; i < numBootstrapSamples; i++) {
    sum = 0.0;
    for (int j = 0; j < n; j++) {
      int k = static_cast<int>(unit_rand() * n);
      sum += vals(k);
    }
    bvals(i) = sum / n;
  }

  std::sort(bvals.data.begin(), bvals.data.end());
  quantile1 = getQuantile(bvals, alpha / 2);
  quantile2 = getQuantile(bvals, (1.0 - alpha / 2));
}

struct IndPair {
  int ind;
  double val;
  IndPair(int _ind, double _val) : ind(_ind), val(_val) {}
};

struct AbsValGreater {
  bool operator()(const IndPair &lhs, const IndPair &rhs) {
    return fabs(lhs.val) > fabs(rhs.val);
  }
};

struct ValGreater {
  bool operator()(const IndPair &lhs, const IndPair &rhs) {
    return lhs.val > rhs.val;
  }
};

inline std::string sparseRep(const cvector &v, int num_to_print) {
  std::vector<IndPair> sorted;
  FOR_CV(v) { sorted.push_back(IndPair(CV_INDEX(v), CV_VAL(v))); }
  sort(sorted.begin(), sorted.end(), AbsValGreater());
  std::ostringstream out;
  num_to_print = std::min(static_cast<size_t>(num_to_print), sorted.size());
  if (0 == num_to_print) {
    out << "(no non-zero entries)";
  } else {
    FOR(i, num_to_print) {
      out << sorted[i].ind << ":" << sorted[i].val << " ";
    }
  }
  return out.str();
}

inline std::string sparseRep(const dvector &v) {
  std::vector<IndPair> sorted;
  FOR(i, v.size()) { sorted.push_back(IndPair(i, v(i))); }
  sort(sorted.begin(), sorted.end(), AbsValGreater());
  std::ostringstream out;
  int num_to_print = std::min(static_cast<size_t>(8), sorted.size());
  if (0 == num_to_print) {
    out << "(no non-zero entries)";
  } else {
    FOR(i, num_to_print) {
      out << sorted[i].ind << ":" << sorted[i].val << " ";
    }
  }
  return out.str();
}

inline std::string denseRep(const cvector &v) {
  std::ostringstream out;
  FOR(i, v.size()) { out << v(i) << " "; }
  return out.str();
}

inline std::string denseRep(const dvector &v) {
  std::ostringstream out;
  FOR(i, v.size()) { out << v(i) << " "; }
  return out.str();
}

};  // namespace MatrixUtils

#endif  // ZMDP_SRC_COMMON_MATRIXUTILS_H_
