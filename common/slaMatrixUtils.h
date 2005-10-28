/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2005-10-28 02:51:41 $
   
 @file    slaMatrixUtils.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * This code and derivatives may not be incorporated into commercial
   products without specific prior written permission.
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * The name of Trey Smith may not be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
