/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-01-21 18:07:02 $
 *  
 * @file    ublasMatrixTypes.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCublasMatrixTypes_h
#define INCublasMatrixTypes_h

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

// undefine NDEBUG if it was previously undefined
#if VEC_OPTIM && !VEC_NDEBUG_WAS_DEFINED
#   undef NDEBUG
#endif

#endif // INCublasMatrixTypes_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/01/21 16:26:04  trey
 * initial check-in
 *
 *
 ***************************************************************************/
