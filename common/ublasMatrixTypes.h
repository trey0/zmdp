/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.4 $  $Author: trey $  $Date: 2005-01-27 05:36:11 $
 *  
 * @file    ublasMatrixTypes.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCublasMatrixTypes_h
#define INCublasMatrixTypes_h

#include <fstream>

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
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>

// for some reason, this needs to be included after the others
#include <boost/numeric/ublas/io.hpp>

#include "commonDefs.h"

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

/**********************************************************************
 * PROVIDE COMPATIBILITY WITH SLA LIBRARY
 **********************************************************************/

namespace boost { namespace numeric { namespace ublas {

  typedef coordinate_matrix<double> kmatrix;

  // result = x
  inline void cvector_from_dvector(cvector& result, const dvector& x)
  {
    result.resize( x.size() );
    FOR (i, x.size()) {
      if (fabs(x(i)) > SPARSE_EPS) result.push_back(i,x(i));
    }
  }

  // result = x
  inline void dvector_from_cvector(dvector& result, const cvector& x)
  {
    result.resize( x.size() );
    FOR (i, x.size()) {
      result(i) = x(i);
    }
  }

  // result = A
  inline void cmatrix_from_kmatrix(cmatrix& result, const kmatrix& A)
  {
    result.resize( A.size1(), A.size2() );
    result = A;
  }

  // result = A(:,c)
  inline void cvector_from_cmatrix_column(cvector& result,
					  cmatrix& A,
					  unsigned int c)
  {
    result = matrix_column<cmatrix>(A,c);
  }

  // A(r,c) = v
  inline void kmatrix_set_entry(kmatrix& A, unsigned int r, unsigned int c,
				double v)
  {
    if (fabs(v) > SPARSE_EPS) {
      A(r,c) = v;
    }
  }

  // A = A'
  inline void kmatrix_transpose_in_place(kmatrix& A)
  {
    A = trans(A);
  }

  inline void mult(dvector& result,
		   const cmatrix& A,
		   const cvector& x)
  {
    axpy_prod( A, x, result, true );
  }

  inline void mult(cvector& result,
		   const cmatrix& A,
		   const cvector& x)
  {
    axpy_prod( A, x, result, true );
  }

  // result = x * A
  inline void mult(dvector& result, const dvector& x, const cmatrix& A)
  {
    axpy_prod( x, A, result, true );
  }

  inline void write(const cmatrix& A, std::ostream& out)
  {
    out << A.size1() << " " << A.size2() << std::endl;
    out << A.filled() << std::endl;
    FOR (c, A.size2()) {
      FOR (r, A.size1()) {
	if (A(r,c) != 0.0) {
	  out << r << " " << c << " " << A(r,c) << std::endl;
	}
      }
    }
  }

  inline void write(const kmatrix& A, std::ostream& out)
  {
    out << A.size1() << " " << A.size2() << std::endl;
    out << A.filled() << std::endl;
    FOR (c, A.size2()) {
      FOR (r, A.size1()) {
	if (A(r,c) != 0.0) {
	  out << r << " " << c << " " << A(r,c) << std::endl;
	}
      }
    }
  }

  template <class T>
  void write_to_file(const T& x, const std::string& file_name)
  {
    std::ofstream out(file_name.c_str());
    if (!out) {
      std::cerr << "ERROR: couldn't open " << file_name << " for writing: "
		<< strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    write(x,out);
    out.close();
  }
			  
}; }; }; // namespace boost::numeric::ublas

#endif // INCublasMatrixTypes_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/01/26 04:14:40  trey
 * added sla compatibility functions
 *
 * Revision 1.2  2005/01/21 18:07:02  trey
 * preparing for transition to sla matrix types
 *
 * Revision 1.1  2005/01/21 16:26:04  trey
 * initial check-in
 *
 *
 ***************************************************************************/
