/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-10-30 20:00:15 $
   
 @file    zmdpCommonTypes.h
 @brief   No brief

 Copyright (c) 2002-2005, Carnegie Mellon University. All rights reserved.

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

#ifndef INCzmdpCommonTypes_h
#define INCzmdpCommonTypes_h

#include "sla.h"

#ifndef GCC_VERSION
#  define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

// needed before we can use hash_map<string, type>
#if GCC_VERSION >= 40300
#  include <tr1/unordered_map>
#  define EXT_NAMESPACE std::tr1
#  define hash_map unordered_map
#else
#  include <ext/hash_map>
#  if GCC_VERSION >= 30200
#    define EXT_NAMESPACE __gnu_cxx
#  else
#    define EXT_NAMESPACE std
#  endif
namespace EXT_NAMESPACE {
  template <>
  struct hash<std::string> {
    size_t operator()(const std::string& s) const {
      hash<char const *> h;
      return h(s.c_str());
    }
  };
  template <class T>
  struct hash<T*> {
    size_t operator()(T* p) const {
      hash<size_t> h;
      return h((size_t) p);
    }
  };
};
#endif

namespace zmdp {

struct ValueInterval {
  double l, u;

  ValueInterval(void) {}
  ValueInterval(double _l, double _u) : l(_l), u(_u) {}
  bool overlapsWith(const ValueInterval& rhs) const {
    return (l <= rhs.u) && (rhs.l <= u);
  }
  double width(void) const {
    return u - l;
  }
};
std::ostream& operator<<(std::ostream& out, const ValueInterval& v);

// option_type<T> is like a pointer, but it follows copy-by-value semantics
// when you assign option_type<T> = T or option_type<T> = option_type<T>.
// it performs copy-by-reference when you assign option_type<T> = T*.
// the name comes from the "'a option" type in ML.
template <class _T>
struct option_type {
  typedef _T value_type;
  
  option_type(void) : val(NULL) {}
  option_type(_T* _val) : val(_val) {}
  option_type(const _T& ref) {
    copyByValue(&ref);
  }
  option_type(const option_type<_T>& rhs) { copyByValue(rhs.val); }
  ~option_type(void) {
    unbind();
  }
  option_type<_T>& operator=(_T* _val) {
    unbind();
    val = _val;
    return *this;
  }
  option_type<_T>& operator=(const _T& ref) {
    unbind();
    copyByValue(&ref);
    return *this;
  }
  option_type<_T>& operator=(const option_type<_T>& rhs) {
    unbind();
    copyByValue(rhs.val);
    return *this;
  }
  _T* operator->(void) { return val; }
  _T& operator*(void) { return *val; }
  const _T* operator->(void) const { return val; }
  const _T& operator*(void) const { return *val; }
  bool defined(void) const { return NULL != val; }
  _T* pointer(void) const { return val; }

protected:
  _T *val;

  void copyByValue(const _T* _val) {
    if (NULL != _val) {
      val = new _T(*_val);
    } else {
      val = NULL;
    }
  }
  void unbind(void) {
    if (NULL != val) {
      delete val;
      val = NULL;
    }
  }
};

}; // namespace zmdp

// we'll redundantly declare the debug level here (also declared in
// zmdpConfig.h) so that files that don't depend anything else in
// zmdpConfig.h don't need to include it.
extern int zmdpDebugLevelG;

#endif // INCzmdpCommonTypes_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.1  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.7  2005/12/05 16:05:46  trey
 * added hash<T*> template class
 *
 * Revision 1.6  2005/11/10 22:06:52  trey
 * added option_type<T>
 *
 * Revision 1.5  2005/11/08 18:14:25  trey
 * moved hash_map setup code from AlphaList.h
 *
 * Revision 1.4  2005/11/03 17:46:16  trey
 * removed MATRIX_NAMESPACE macro
 *
 * Revision 1.3  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.2  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.1  2005/10/21 20:11:52  trey
 * renamed to avoid potential conflicts with other projects
 *
 * Revision 1.5  2005/01/28 03:20:11  trey
 * VEC_OPTIM now implies NDEBUG for sla as well as ublas
 *
 * Revision 1.4  2005/01/26 04:14:15  trey
 * added MATRIX_NAMESPACE
 *
 * Revision 1.3  2005/01/21 18:07:02  trey
 * preparing for transition to sla matrix types
 *
 * Revision 1.2  2005/01/21 15:22:02  trey
 * added include of ublas/operation.hpp, allowing use of axpy_prod()
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.3  2003/09/22 18:48:14  trey
 * made several algorithm configurations depend on makefile settings, added extra timing output
 *
 * Revision 1.2  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.1  2003/09/11 01:45:50  trey
 * initial check-in
 *
 *
 ***************************************************************************/
