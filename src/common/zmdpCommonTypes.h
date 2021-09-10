/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_COMMON_ZMDPCOMMONTYPES_H_
#define ZMDP_SRC_COMMON_ZMDPCOMMONTYPES_H_

#include <string>

#include "sla.h"

#ifndef GCC_VERSION
#define GCC_VERSION \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

// This section tries to auto-detect the correct definition of
// hash_map<string, type> to use with this compiler

// Note we use the __APPLE__ macro to detect a Mac OS X compiler and
// assume it supports C++11... otherwise the auto-detect based on GCC
// version fails because modern Mac OS X compilers weirdly report a very
// old GCC version.
#if __APPLE__

#include <unordered_map>
#define EXT_NAMESPACE std
#define hash_map unordered_map

#elif (GCC_VERSION >= 40300)

#include <tr1/unordered_map>
#define EXT_NAMESPACE std::tr1
#define hash_map unordered_map

#else  // if not GCC_VERSION >= 40300

#include <ext/hash_map>

#if GCC_VERSION >= 30200
#define EXT_NAMESPACE __gnu_cxx
#else
#define EXT_NAMESPACE std
#endif

namespace EXT_NAMESPACE {

template <>
struct hash<std::string> {
  size_t operator()(const std::string &s) const {
    hash<char const *> h;
    return h(s.c_str());
  }
};

template <class T>
struct hash<T *> {
  size_t operator()(T *p) const {
    hash<size_t> h;
    return h(static_cast<size_t>(p));
  }
};

};  // namespace EXT_NAMESPACE

#endif  // branching based on GCC version

namespace zmdp {

struct ValueInterval {
  double l, u;

  ValueInterval(void) {}
  ValueInterval(double _l, double _u) : l(_l), u(_u) {}
  bool overlapsWith(const ValueInterval &rhs) const {
    return (l <= rhs.u) && (rhs.l <= u);
  }
  double width(void) const { return u - l; }
};
std::ostream &operator<<(std::ostream &out, const ValueInterval &v);

// option_type<T> is like a pointer, but it follows copy-by-value semantics
// when you assign option_type<T> = T or option_type<T> = option_type<T>.
// it performs copy-by-reference when you assign option_type<T> = T*.
// the name comes from the "'a option" type in ML.
template <class _T>
struct option_type {
  typedef _T value_type;

  option_type(void) : val(NULL) {}
  explicit option_type(_T *_val) : val(_val) {}
  explicit option_type(const _T &ref) { copyByValue(&ref); }
  option_type(const option_type<_T> &rhs) { copyByValue(rhs.val); }
  ~option_type(void) { unbind(); }
  option_type<_T> &operator=(_T *_val) {
    unbind();
    val = _val;
    return *this;
  }
  option_type<_T> &operator=(const _T &ref) {
    unbind();
    copyByValue(&ref);
    return *this;
  }
  option_type<_T> &operator=(const option_type<_T> &rhs) {
    unbind();
    copyByValue(rhs.val);
    return *this;
  }
  _T *operator->(void) { return val; }
  _T &operator*(void) { return *val; }
  const _T *operator->(void) const { return val; }
  const _T &operator*(void) const { return *val; }
  bool defined(void) const { return NULL != val; }
  _T *pointer(void) const { return val; }

 protected:
  _T *val;

  void copyByValue(const _T *_val) {
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

};  // namespace zmdp

// we'll redundantly declare the debug level here (also declared in
// zmdpConfig.h) so that files that don't depend anything else in
// zmdpConfig.h don't need to include it.
extern int zmdpDebugLevelG;

#endif  // ZMDP_SRC_COMMON_ZMDPCOMMONTYPES_H_
