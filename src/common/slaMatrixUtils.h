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

#ifndef ZMDP_SRC_COMMON_SLAMATRIXUTILS_H_
#define ZMDP_SRC_COMMON_SLAMATRIXUTILS_H_

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// iostream causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <math.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

/**********************************************************************
 * MACROS
 **********************************************************************/

// FIX adjusting this based on observed bounds violations
#define HASH_VECTOR_PRECISION "%6d:%15.9lf "
#define HASH_VECTOR_LEN (24)

#if 0
#define HASH_VECTOR_PRECISION "%5d:%6.4lf "
#define HASH_VECTOR_LEN (14)
#endif

// convenience macros for iterating through compressed matrices and vectors

#define FOR_CM_MAJOR(c, M) for (unsigned c = 0; c < M.size2(); c++)

#define FOR_CM_MINOR(c, M)                                                \
  typeof(M.data.begin()) __cm_begin = M.data.begin() + M.col_starts[c];   \
  typeof(M.data.begin()) __cm_end = M.data.begin() + M.col_starts[c + 1]; \
  for (typeof(M.data.begin()) __cm_j = __cm_begin; __cm_j != __cm_end; __cm_j++)

#define CM_VAL(M) (__cm_j->value)
#define CM_ROW(c, M) (__cm_j->index)
#define CM_COL(c, M) (c)

#define FOR_CV(v)                                                              \
  for (typeof(v.data.begin()) __cv_i = v.data.begin(); __cv_i != v.data.end(); \
       __cv_i++)

#define CV_VAL(v) (__cv_i->value)

#define CV_INDEX(v) (__cv_i->index)

/**********************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************/

namespace MatrixUtils {
using namespace sla;

// Set all entries to zero.
void set_to_zero(dmatrix &M);
void set_to_zero(kmatrix &M);
void set_to_zero(cmatrix &M);
void set_to_zero(dvector &v);
void set_to_zero(cvector &v);

/**********************************************************************
 * FUNCTIONS
 **********************************************************************/

// Set all entries to zero.
inline void set_to_zero(dmatrix &M) { M.resize(M.size1(), M.size2()); }

inline void set_to_zero(kmatrix &M) { M.resize(M.size1(), M.size2()); }

inline void set_to_zero(cmatrix &M) { M.resize(M.size1(), M.size2()); }

inline void set_to_zero(dvector &v) { v.resize(v.size()); }

inline void set_to_zero(cvector &v) { v.resize(v.size()); }

}  // namespace MatrixUtils

#endif  // ZMDP_SRC_COMMON_SLAMATRIXUTILS_H_
