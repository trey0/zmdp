/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-13 23:29:44 $
 *  
 * @file    MatrixUtils.cc
 * @brief   No brief
 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "MatrixUtils.h"

using namespace std;

/***************************************************************************
 * FUNCTIONS
 ***************************************************************************/

namespace MatrixUtils {
  void init_matrix_utils(void) {
    srand(time(0));
  }

  // Generate a sample from a uniform distribution over [0,1].
  double unit_rand(void) {
    return ((double)std::rand())/RAND_MAX;
  }

  // Generate a matrix where each sample is drawn according to unit_rand().
  bmatrix rand_matrix(int num_rows, int num_cols) {
    bmatrix M(num_rows, num_cols);
    FOR (r, num_rows) {
      FOR (c, num_cols) {
	M(r,c) = unit_rand();
      }
    }
    return M;
  }

  bvector rand_vector(int num_entries) {
    bvector v(num_entries);
    FOR (i, num_entries) {
      v(i) = unit_rand();
    }
    return v;
  }

  void set_to_zero(dmatrix& M) {
    M = zero_matrix<double>(M.size1(), M.size2());
  }
  void set_to_zero(cmatrix& M) {
    bmatrix Mp(M.size1(), M.size2());
    M = Mp;
  }

  void set_to_zero(dvector& v) {
    FOR_EACH (elt, v.data()) {
      *elt = 0;
    }
  }
  void set_to_zero(cvector& v) {
    bvector vp(v.size());
    v = vp;
  }

  class CompareSecond {
  public:
    bool operator()(const pair<size_t, double>& lhs,
		    const pair<size_t, double>& rhs) {
      return lhs.second < rhs.second;
    }
  };

  // Initialize a vector from an array.
  bvector make_vector(double* arr, int num_elts) {
    bvector v(num_elts);
    FOR (i, num_elts) {
      VEC_ASSIGN_CHECK_ZERO( v(i), arr[i] );
    }
    return v;
  }

  // Index of maximum element of a vector
  int argmax_elt(const dvector& v) {
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

  int argmax_elt(const cvector& v) {
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

  // b represents a discrete probability distribution Pr(outcome = i) = b(i).
  // Chooses an outcome according to the distribution.
  int chooseFromDistribution(const dvector& b) {
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

  int chooseFromDistribution(const cvector& b) {
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

  // Scale the rows of M, like MATLAB diag(v) * M.
  bmatrix scale_columns(const bvector& v, const dmatrix& M) {
    bmatrix R(M.size1(), M.size2());
    FOR (r, M.size1()) {
      FOR (c, M.size2()) {
	R(r,c) = v(c) * M(r,c);
      }
    }
    return R;
  }

  // Scale the rows of M, like MATLAB diag(v) * M.
  bmatrix scale_columns(const bvector& v, const cmatrix& M) {
    bmatrix R(M.size1(), M.size2());
    // efficient iteration through compressed matrix
    FOR_CM_ROWS (r, M) {
      FOR_CM_COLS(r, M) {
	int c = CM_COL(M);
	double M_r_c = CM_VAL(M);
	R(r,c) = v(c) * M_r_c;
      }
    }
    return R;
  }

#if 0
  // Element-wise maximum of two matrices (equivalent to Matlab max)
  bmatrix emax(const bmatrix& M, const bmatrix& N) {
    assert(M.size(0) == N.size(0));
    assert(M.size(1) == N.size(1));
    int num_rows = M.size(0);
    int num_cols = M.size(1);
    bmatrix result(num_rows, num_cols);
    FOR (r, num_rows) {
      FOR (c, num_cols) {
	result(r,c) = max(M(r,c), N(r,c));
      }
    }
    return result;
  }
#endif

#if 0
  void write_matrix(const bmatrix& history, const string& outfile) {
    ofstream out(outfile.c_str());
    if (!out) {
      cerr << "ERROR: couldn't open " << outfile << " for writing: "
	   << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }

    out << history;

    out.close();
  }
#endif

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

  string sparseRep(const bvector& v) {
    std::vector<IndPair> sorted;
    FOR (i, v.size()) {
      sorted.push_back(IndPair(i, v(i)));
    }
    sort(sorted.begin(), sorted.end(), AbsValGreater());
    ostringstream out;
    int num_to_print = min((unsigned)8, v.size());
    FOR (i, num_to_print) {
      out << sorted[i].ind << ":" << sorted[i].val << " ";
    }
    return out.str();
  }

  string maxRep(const bvector& v) {
    std::vector<IndPair> sorted;
    FOR (i, v.size()) {
      sorted.push_back(IndPair(i, v(i)));
    }
    sort(sorted.begin(), sorted.end(), ValGreater());
    ostringstream out;
    int num_to_print = min((unsigned)8, v.size());
    FOR (i, num_to_print) {
      out << sorted[i].ind << ":" << sorted[i].val << " ";
    }
    return out.str();
  }

  const char* hashable(const dvector& b) {
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

  const char* hashable(const cvector& b) {
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
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.12  2003/09/22 21:42:28  trey
 * made some minor fixes so that algorithm variants to compile
 *
 * Revision 1.11  2003/09/22 18:48:13  trey
 * made several algorithm configurations depend on makefile settings, added extra timing output
 *
 * Revision 1.10  2003/09/20 02:26:09  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.9  2003/09/17 20:54:24  trey
 * seeing good performance on tagAvoid (but some mods since run started...)
 *
 * Revision 1.8  2003/09/17 18:30:17  trey
 * seems to show best performance so far
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
 * Revision 1.4  2003/07/23 20:48:33  trey
 * added eabs and scalar_mat_mult
 *
 * Revision 1.3  2003/07/18 16:07:59  trey
 * finished implementing Q caching
 *
 * Revision 1.2  2003/07/16 16:05:46  trey
 * added avg/stdev calculation
 *
 * Revision 1.1  2003/06/26 15:41:21  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
