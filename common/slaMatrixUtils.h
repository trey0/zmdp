/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.9 $  $Author: trey $  $Date: 2006-02-01 01:09:37 $
   
 @file    slaMatrixUtils.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

/**********************************************************************
 * MACROS
 **********************************************************************/

// FIX adjusting this based on observed bounds violations
#define HASH_VECTOR_PRECISION "%5d:%11.9lf "
#define HASH_VECTOR_LEN (19)

#if 0
#define HASH_VECTOR_PRECISION "%5d:%6.4lf "
#define HASH_VECTOR_LEN (14)
#endif

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
  using namespace sla;

  // Set all entries to zero.
  void set_to_zero(dmatrix& M);
  void set_to_zero(kmatrix& M);
  void set_to_zero(cmatrix& M);
  void set_to_zero(dvector& v);
  void set_to_zero(cvector& v);

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
 * Revision 1.8  2005/12/19 22:38:47  trey
 * made hashable() precision tighter to avoid serious bounds violations caused by belief caching
 *
 * Revision 1.7  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.6  2005/10/28 02:51:41  trey
 * added copyright headers
 *
 * Revision 1.5  2005/10/27 22:08:36  trey
 * removed cruft
 *
 * Revision 1.4  2005/10/21 20:12:23  trey
 * renamed headers to avoid potential conflicts with other projects
 *
 * Revision 1.3  2005/01/28 03:22:03  trey
 * added some functions, improved ublas compatibility
 *
 * Revision 1.2  2005/01/26 04:16:39  trey
 * major overhaul
 *
 *
 ***************************************************************************/
