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

#ifndef ZMDP_SRC_COMMON_SLA_H_
#define ZMDP_SRC_COMMON_SLA_H_

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "zmdpCommonDefs.h"

// sla     = simple linear algebra
// dvector = dense vector
// cvector = compressed vector
// dmatrix = dense matrix
// kmatrix = coordinate matrix
// cmatrix = compressed matrix

namespace sla {

struct dvector;
struct cvector;
struct dmatrix;
struct kmatrix;
struct cmatrix;

/**********************************************************************
 * DVECTOR
 **********************************************************************/

struct dvector {
  std::vector<double> data;

  dvector(void) {}
  explicit dvector(unsigned int _size) { resize(_size); }

  double &operator()(unsigned int i) { return data[i]; }
  double operator()(unsigned int i) const { return data[i]; }
  void operator*=(double s);
  void operator+=(const dvector &x);
  void operator-=(const dvector &x);

  unsigned int size(void) const { return data.size(); }
  void resize(unsigned int _size, double value = 0.0);
  void clear(void) { data.clear(); }

  void read(std::istream &in);
  void write(std::ostream &out) const;
};

/**********************************************************************
 * CVECTOR
 **********************************************************************/

struct cvector_entry {
  unsigned int index;
  double value;

  cvector_entry(void) {}
  cvector_entry(unsigned int _index, double _value)
      : index(_index), value(_value) {}
};

struct cvector {
  unsigned int size_;
  std::vector<cvector_entry> data;

  cvector(void) : size_(0) {}
  explicit cvector(unsigned int _size) { resize(_size); }

  double operator()(unsigned int index) const;
  void operator+=(const cvector &x);
  void operator-=(const cvector &x);
  void operator*=(double s);

  unsigned int size(void) const { return size_; }
  unsigned int filled(void) const { return data.size(); }

  void resize(unsigned int _size, unsigned int _non_zeros = 0);
  void push_back(unsigned int index, double value);
  void canonicalize(void) {}
  void clear(void) { data.clear(); }

  void read(std::istream &in);
};

/**********************************************************************
 * DMATRIX
 **********************************************************************/

struct dmatrix {
  unsigned int size1_, size2_;
  std::vector<double> data;

  double &operator()(unsigned int r, unsigned int c);
  double operator()(unsigned int r, unsigned int c) const;

  unsigned int size1(void) const { return size1_; }
  unsigned int size2(void) const { return size2_; }
  void resize(unsigned int _size1, unsigned int _size2, double value = 0.0);
  void clear(void) { data.clear(); }
};

/**********************************************************************
 * KMATRIX
 **********************************************************************/

struct kmatrix_entry {
  unsigned int r, c;
  double value;

  kmatrix_entry(void) {}
  kmatrix_entry(unsigned int _r, unsigned int _c, double _value)
      : r(_r), c(_c), value(_value) {}
};

struct kmatrix {
  unsigned int size1_, size2_;
  std::vector<kmatrix_entry> data;

  kmatrix(void) : size1_(0), size2_(0) {}
  kmatrix(unsigned int _size1, unsigned int _size2) { resize(_size1, _size2); }

  double operator()(unsigned int r, unsigned int c) const;

  unsigned int size1(void) const { return size1_; }
  unsigned int size2(void) const { return size2_; }
  unsigned int filled(void) const { return data.size(); }
  void clear(void);
  void resize(unsigned int _size1, unsigned int _size2, double value = 0.0);
  void push_back(unsigned int r, unsigned int c, double value);
  void canonicalize(void);

  void read(std::istream &in);
};

/**********************************************************************
 * CMATRIX
 **********************************************************************/

struct cmatrix {
  unsigned int size1_, size2_;
  std::vector<unsigned int> col_starts;
  std::vector<cvector_entry> data;

  cmatrix(void) : size1_(0), size2_(0) {}
  cmatrix(unsigned int _size1, unsigned int _size2) { resize(_size1, _size2); }

  double operator()(unsigned int r, unsigned int c) const;

  unsigned int size1(void) const { return size1_; }
  unsigned int size2(void) const { return size2_; }
  unsigned int filled(void) const { return data.size(); }
  unsigned int filled_in_column(unsigned int col) const;
  void resize(unsigned int _size1, unsigned int _size2,
              unsigned int _non_zeros = 0);
  void push_back(unsigned int row, unsigned int col, double value);

  // if resize()/push_back() are used to initialize, you must call
  // canonicalize() before performing any operations with the matrix
  void canonicalize(void);

  void clear(void) { data.clear(); }

  void read(std::istream &in);
  void write(std::ostream &out) const;
};

/**********************************************************************
 * NON-MEMBER FUNCTION PROTOTYPES
 **********************************************************************/

// result = x
inline void copy(cvector &result, const cvector &x) { result = x; }
inline void copy(dvector &result, const dvector &x) { result = x; }

// result = x
void copy(dvector &result, const cvector &x);

// result = x
void copy(cvector &result, const dvector &x);

// result = A (side-effect: canonicalizes A)
void copy(cmatrix &result, kmatrix &A);

// result = A(.,c)
void copy_from_column(cvector &result, const cmatrix &A, unsigned int c);

// result = A(:,c)
void copy_from_column(dvector &result, const cmatrix &A, unsigned int c);

// result = ones(rsize)
void set_to_one(dvector &result, unsigned int rsize);

// result = ones(rsize)
void set_to_one(cvector &result, unsigned int rsize);

// A(r,c) = v
void kmatrix_set_entry(kmatrix &A, unsigned int r, unsigned int c, double v);

// A = A'
void kmatrix_transpose_in_place(kmatrix &A);

double norm_1(const cvector &x);
double norm_inf(const cvector &x);
double norm_inf(const dvector &x);
double sum(const cvector &x);
double sum(const dvector &x);

// result = A * x
void mult(dvector &result, const cmatrix &A, const cvector &x);

// result = A * x
void mult(cvector &result, const cmatrix &A, const cvector &x);

// result = x * A
void mult(dvector &result, const dvector &x, const cmatrix &A);

// result = x * A
void mult(dvector &result, const cvector &x, const cmatrix &A);

// result = x * A [note: if you have transpose(A) available, try
//   mult(result, A', x) instead; it is often much faster]
void mult(cvector &result, const cvector &x, const cmatrix &A);

// result = x .* y [for all i, result(i) = x(i) * y(i)]
void emult(dvector &result, const dvector &x, const dvector &y);

// result = x .* y [for all i, result(i) = x(i) * y(i)]
void emult(cvector &result, const cvector &x, const cvector &y);

// result = A(:,c) .* x
void emult_column(cvector &result, const cmatrix &A, unsigned int c,
                  const cvector &x);

// result = x .* y
void emult(dvector &result, const dvector &x, const cvector &y);

// result = A(:,c) .* x
void emult_column(dvector &result, const cmatrix &A, unsigned int c,
                  const dvector &x);

// result = max(x,y)
void emax(dvector &result, const dvector &x, const dvector &y);

// result = max(result,x)
void max_assign(dvector &result, const dvector &x);

// return x' * y
double inner_prod(const dvector &x, const cvector &y);

// return x' * y
double inner_prod(const cvector &x, const cvector &y);

// return A(:,c)' * y
double inner_prod_column(const cmatrix &A, unsigned int c, const cvector &y);

// result = x + y
void add(cvector &result, const cvector &x, const cvector &y);

// result = x - y
void subtract(cvector &result, const cvector &x, const cvector &y);

// return true if for all i: x(i) >= y(i) - eps
bool dominates(const dvector &x, const dvector &y, double eps);

// return true if for all i: x(i) >= y(i) - eps
bool dominates(const cvector &x, const cvector &y, double eps);

template <class T>
void read_from_file(T &x, const std::string &file_name);

template <class T>
void write_to_file(const T &x, const std::string &file_name);

/**********************************************************************
 * DVECTOR FUNCTIONS
 **********************************************************************/

inline void dvector::operator*=(double s) {
  FOR_EACH(di, data) { (*di) *= s; }
}

#define DVECTOR_OPERATOR(OP)                           \
  inline void dvector::operator OP(const dvector &x) { \
    typeof(x.data.begin()) xi;                         \
                                                       \
    assert(size() == x.size());                        \
                                                       \
    xi = x.data.begin();                               \
    FOR_EACH(di, data) {                               \
      (*di) OP(*xi);                                   \
      xi++;                                            \
    }                                                  \
  }

DVECTOR_OPERATOR(+=);
DVECTOR_OPERATOR(-=);

inline void dvector::resize(unsigned int _size, double value) {
  assert(0 == value);
  data.clear();
  data.resize(_size, value);
}

inline void dvector::read(std::istream &in) {
  int num_entries;

  in >> num_entries;
  resize(num_entries);
  FOR(i, num_entries) { in >> data[i]; }

#if 0
    std::cout << "dense read: size = " << data.size() << std::endl;
    FOR(i, data.size()) {
      if (i == 10) {
        std::cout << "..." << std::endl;
        break;
      }
      std::cout << i << " " << data[i] << std::endl;
    }
#endif
}

inline void dvector::write(std::ostream &out) const {
  out << size() << std::endl;
  FOR_EACH(x, data) { out << (*x) << std::endl; }
}

/**********************************************************************
 * CVECTOR FUNCTIONS
 **********************************************************************/

inline double cvector::operator()(unsigned int index) const {
  FOR_EACH(di, data) {
    if (di->index >= index) {
      if (di->index == index) {
        return di->value;
      } else {
        return 0.0;
      }
    }
  }
  return 0.0;
}

inline void cvector::operator*=(double s) {
  typeof(data.begin()) vi, vend = data.end();
  for (vi = data.begin(); vi != vend; vi++) {
    vi->value *= s;
  }
}

inline void cvector::operator+=(const cvector &x) {
  cvector tmp;
  add(tmp, *this, x);
  *this = tmp;
}

inline void cvector::operator-=(const cvector &x) {
  cvector tmp;
  subtract(tmp, *this, x);
  *this = tmp;
}

inline void cvector::resize(unsigned int _size, unsigned int _non_zeros) {
  assert(0 == _non_zeros);
  size_ = _size;
  data.clear();
}

inline void cvector::push_back(unsigned int index, double value) {
  data.push_back(cvector_entry(index, value));
}

inline void cvector::read(std::istream &in) {
  int num_entries;

  in >> size_;
  in >> num_entries;
  data.resize(num_entries);
  FOR(i, num_entries) { in >> data[i].index >> data[i].value; }
}

/**********************************************************************
 * DMATRIX FUNCTIONS
 **********************************************************************/

inline double &dmatrix::operator()(unsigned int r, unsigned int c) {
  return data[c * size1_ + r];
}

inline double dmatrix::operator()(unsigned int r, unsigned int c) const {
  return data[c * size1_ + r];
}

inline void dmatrix::resize(unsigned int _size1, unsigned int _size2,
                            double value) {
  assert(0 == value);
  size1_ = _size1;
  size2_ = _size2;
  data.clear();
  data.resize(size1_ * size2_, value);
}

/**********************************************************************
 * KMATRIX FUNCTIONS
 **********************************************************************/

inline double kmatrix::operator()(unsigned int r, unsigned int c) const {
  assert((0 <= r) && (r < size1()));
  assert((0 <= c) && (c < size2()));
  // NOTE: also assumes the kmatrix has been canonicalized

  FOR_EACH(di, data) {
    if (di->r == r && di->c == c) {
      return di->value;
    }
  }
  return 0.0;
}

inline void kmatrix::clear(void) { resize(0, 0, 0); }

inline void kmatrix::resize(unsigned int _size1, unsigned int _size2,
                            double value) {
  assert(0 == value);
  size1_ = _size1;
  size2_ = _size2;
  data.clear();
}

inline void kmatrix::push_back(unsigned int r, unsigned int c, double value) {
  data.push_back(kmatrix_entry(r, c, value));
}

struct ColumnMajorCompare {
  bool operator()(const kmatrix_entry &lhs, const kmatrix_entry &rhs) {
    return (lhs.c < rhs.c) || ((lhs.c == rhs.c) && (lhs.r < rhs.r));
  }
};

inline bool rc_equal(const kmatrix_entry &lhs, const kmatrix_entry &rhs) {
  return (lhs.r == rhs.r) && (lhs.c == rhs.c);
}

inline void kmatrix::canonicalize(void) {
  std::vector<kmatrix_entry> d;

  // sort in column-major order
  std::stable_sort(data.begin(), data.end(), ColumnMajorCompare());

  // ensure there is at most one entry with each (r,c) coordinate.
  // among all the entries with the same (r,c), keep the last one.
  // note that this operation does *not* get rid of near-zero entries.
  FOR(i, data.size()) {
    if (!d.empty() && rc_equal(d.back(), data[i])) {
      d.back() = data[i];
    } else {
      d.push_back(data[i]);
    }
  }
  data.swap(d);
}

inline void kmatrix::read(std::istream &in) {
  int rows, cols;
  int num_entries;
  int r, c;
  double value;

  in >> rows >> cols;
  resize(rows, cols);

  in >> num_entries;
  FOR(i, num_entries) {
    in >> r >> c >> value;
    push_back(r, c, value);
  }
}

/**********************************************************************
 * CMATRIX FUNCTIONS
 **********************************************************************/

inline double cmatrix::operator()(unsigned int r, unsigned int c) const {
  typeof(data.begin()) di;
  typeof(data.begin()) col_end = data.begin() + col_starts[c + 1];

  for (di = data.begin() + col_starts[c]; di != col_end; di++) {
    if (di->index >= r) {
      if (di->index == r) {
        return di->value;
      } else {
        return 0.0;
      }
    }
  }
  return 0.0;
}

inline unsigned int cmatrix::filled_in_column(unsigned int col) const {
  return col_starts[col + 1] - col_starts[col];
}

inline void cmatrix::resize(unsigned int _size1, unsigned int _size2,
                            unsigned int _non_zeros) {
  assert(0 == _non_zeros);
  size1_ = _size1;
  size2_ = _size2;
  col_starts.resize(size2() + 1);
  FOR_EACH(ci, col_starts) { (*ci) = 0; }
  data.clear();
}

inline void cmatrix::push_back(unsigned int r, unsigned int c, double value) {
  data.push_back(cvector_entry(r, value));
  col_starts[c + 1] = data.size();
}

inline void cmatrix::canonicalize(void) {
  FOR(i, size2_) {
    if (col_starts[i] > col_starts[i + 1]) {
      col_starts[i + 1] = col_starts[i];
    }
  }
}

inline void cmatrix::read(std::istream &in) {
  kmatrix km;
  km.read(in);
  copy(*this, km);
}

inline void cmatrix::write(std::ostream &out) const {
  typeof(data.begin()) di, col_end;

  out << size1_ << " " << size2_ << std::endl;
  out << data.size() << std::endl;
  FOR(c, size2_) {
    col_end = data.begin() + col_starts[c + 1];
    for (di = data.begin() + col_starts[c]; di != col_end; di++) {
      out << di->index << " " << c << " " << di->value << std::endl;
    }
  }
}

/**********************************************************************
 * NON-MEMBER FUNCTIONS
 **********************************************************************/

inline void copy(dvector &result, const cvector &x) {
  result.resize(x.size());
  FOR_EACH(xi, x.data) { result.data[xi->index] = xi->value; }
}

inline void copy(cvector &result, const dvector &x) {
  int i;

  result.resize(x.size());
  i = 0;
  FOR_EACH(xi, x.data) {
    if (fabs(*xi) > SPARSE_EPS) {
      result.push_back(i, *xi);
    }
    i++;
  }
}

inline void copy(cmatrix &result, kmatrix &A) {
  A.canonicalize();
  result.resize(A.size1(), A.size2());
  FOR_EACH(entry, A.data) {
    if (fabs(entry->value) > SPARSE_EPS) {
      result.push_back(entry->r, entry->c, entry->value);
    }
  }
  result.canonicalize();
}

// result = A(:,c)
inline void copy_from_column(cvector &result, const cmatrix &A,
                             unsigned int c) {
  assert(0 <= c && c < A.size2());

  typeof(A.data.begin()) Ai, col_start, col_end;
  typeof(result.data.begin()) ri;

  col_start = A.data.begin() + A.col_starts[c];
  col_end = A.data.begin() + A.col_starts[c + 1];

  result.resize(A.size1());
  result.data.resize(col_end - col_start);
  for (Ai = col_start, ri = result.data.begin(); Ai != col_end; Ai++, ri++) {
    (*ri) = (*Ai);
  }
}

// result = A(:,c)
inline void copy_from_column(dvector &result, const cmatrix &A,
                             unsigned int c) {
  assert(0 <= c && c < A.size2());
  typeof(A.data.begin()) Ai, col_end;

  result.resize(A.size1());

  col_end = A.data.begin() + A.col_starts[c + 1];
  for (Ai = A.data.begin() + A.col_starts[c]; Ai != col_end; Ai++) {
    result(Ai->index) = Ai->value;
  }
}

// result = ones(rsize)
inline void set_to_one(dvector &result, unsigned int rsize) {
  result.resize(rsize);
  FOR(s, rsize) { result(s) = 1.0; }
}

// result = ones(rsize)
inline void set_to_one(cvector &result, unsigned int rsize) {
  result.resize(rsize);
  FOR(s, rsize) { result.push_back(s, 1.0); }
  result.canonicalize();
}

// A(r,c) = v
inline void kmatrix_set_entry(kmatrix &A, unsigned int r, unsigned int c,
                              double v) {
  A.push_back(r, c, v);
}

// A = A'
inline void kmatrix_transpose_in_place(kmatrix &A) {
  std::swap(A.size1_, A.size2_);
  FOR_EACH(Ai, A.data) { std::swap(Ai->r, Ai->c); }
  A.canonicalize();
}

inline double norm_1(const cvector &x) {
  double sum = 0.0;
  FOR_EACH(xi, x.data) { sum += fabs(xi->value); }
  return sum;
}

inline double norm_inf(const cvector &x) {
  double val, max = 0.0;
  FOR_EACH(xi, x.data) {
    val = fabs(xi->value);
    if (val > max) max = val;
  }
  return max;
}

inline double norm_inf(const dvector &x) {
  double val, max = 0.0;
  FOR_EACH(xi, x.data) {
    val = fabs(*xi);
    if (val > max) max = val;
  }
  return max;
}

inline double sum(const cvector &x) {
  double sum = 0.0;
  FOR_EACH(xi, x.data) { sum += xi->value; }
  return sum;
}

inline double sum(const dvector &x) {
  double sum = 0.0;
  FOR_EACH(xi, x.data) { sum += *xi; }
  return sum;
}

// result = A * x
inline void mult(dvector &result, const cmatrix &A, const cvector &x) {
  typeof(A.data.begin()) Ai, col_end;
  int xind;
  double xval;

  result.resize(x.size());

  FOR_EACH(xi, x.data) {
    xind = xi->index;
    xval = xi->value;
    col_end = A.data.begin() + A.col_starts[xind + 1];
    for (Ai = A.data.begin() + A.col_starts[xind]; Ai != col_end; Ai++) {
      result.data[Ai->index] += xval * Ai->value;
    }
  }
}

// result = A * x
inline void mult(cvector &result, const cmatrix &A, const cvector &x) {
  if (x.filled() * A.filled() < 0.1 * A.size1() * A.size2()) {
    // this sparse accumulator technique is only a good idea if both A
    // and x are very sparse.  in that case, allocating and
    // initializing a dense vector of the right size for temporary
    // storage can dominate running time.  but for somewhat denser
    // operands, the alternate implementation below is likely to be
    // better.

    typeof(A.data.begin()) Ai, col_end;
    int xind;
    double xval;
    cvector tmp;
    cvector accum;

    accum.resize(x.size());
    FOR_EACH(xi, x.data) {
      xind = xi->index;
      xval = xi->value;
      col_end = A.data.begin() + A.col_starts[xind + 1];
      tmp.resize(x.size());
      for (Ai = A.data.begin() + A.col_starts[xind]; Ai != col_end; Ai++) {
        tmp.push_back(Ai->index, xval * Ai->value);
      }
      accum += tmp;
    }

    result.resize(x.size());
    FOR_EACH(ai, accum.data) {
      if (fabs(ai->value) > SPARSE_EPS) {
        result.data.push_back(*ai);
      }
    }
  } else {
    dvector tmp;

    mult(tmp, A, x);
    copy(result, tmp);
  }
}

// result = x * A
inline void mult(dvector &result, const dvector &x, const cmatrix &A) {
  typeof(A.data.begin()) Ai, col_end;

  assert(x.size() == A.size1());
  result.resize(A.size2());

  FOR(c, A.size2()) {
    col_end = A.data.begin() + A.col_starts[c + 1];
    for (Ai = A.data.begin() + A.col_starts[c]; Ai != col_end; Ai++) {
      result(c) += x(Ai->index) * Ai->value;
    }
  }
}

// result = x * A
inline void mult(dvector &result, const cvector &x, const cmatrix &A) {
  assert(x.size() == A.size1());
  result.resize(A.size2());

  FOR(c, A.size2()) { result(c) = inner_prod_column(A, c, x); }
}

// result = x * A
inline void mult(cvector &result, const cvector &x, const cmatrix &A) {
  dvector tmp;
  mult(tmp, x, A);
  copy(result, tmp);
}

// result = x .* y [for all i, result(i) = x(i) * y(i)]
inline void emult(dvector &result, const dvector &x, const dvector &y) {
  assert(x.size() == y.size());
  result.resize(x.size());
  FOR(i, result.size()) { result(i) = x(i) * y(i); }
}

// result = x .* y [for all i, result(i) = x(i) * y(i)]
template <class T, class U>
void emult_cc_internal(cvector &result, T xbegin, T xend, U ybegin, U yend) {
  U yi = ybegin;
  for (T xi = xbegin; xi != xend; xi++) {
    while (1) {
      if (yi == yend) return;
      if (yi->index >= xi->index) {
        if (yi->index == xi->index) {
          result.push_back(xi->index, xi->value * yi->value);
        }
        break;
      }
      yi++;
    }
  }
}

// result = x .* y [for all i, result(i) = x(i) * y(i)]
inline void emult(cvector &result, const cvector &x, const cvector &y) {
  assert(x.size() == y.size());
  result.resize(x.size());

  emult_cc_internal(result, x.data.begin(), x.data.end(), y.data.begin(),
                    y.data.end());

  result.canonicalize();
}

// result = A(:,c) .* x
inline void emult_column(cvector &result, const cmatrix &A, unsigned int c,
                         const cvector &x) {
  assert(A.size1() == x.size());
  assert(0 <= c && c < A.size2());
  result.resize(x.size());

  emult_cc_internal(result, A.data.begin() + A.col_starts[c],
                    A.data.begin() + A.col_starts[c + 1], x.data.begin(),
                    x.data.end());

  result.canonicalize();
}

// result = x .* y [for all i, result(i) = x(i) * y(i)]
template <class T>
void emult_dc_internal(dvector &result, const dvector &x, T ybegin, T yend) {
  int yind;
  for (T yi = ybegin; yi != yend; yi++) {
    yind = yi->index;
    result(yind) = x(yind) * yi->value;
  }
}

// result = x .* y
inline void emult(dvector &result, const dvector &x, const cvector &y) {
  assert(x.size() == y.size());
  result.resize(x.size());
  emult_dc_internal(result, x, y.data.begin(), y.data.end());
}

// result = A(:,c) .* x
inline void emult_column(dvector &result, const cmatrix &A, unsigned int c,
                         const dvector &x) {
  assert(A.size1() == x.size());
  assert(0 <= c && c < A.size2());
  result.resize(x.size());
  emult_dc_internal(result, x, A.data.begin() + A.col_starts[c],
                    A.data.begin() + A.col_starts[c + 1]);
}

// result = max(x,y)
inline void emax(dvector &result, const dvector &x, const dvector &y) {
  assert(x.size() == y.size());
  result.resize(x.size());

  typeof(y.data.begin()) yi = y.data.begin();
  typeof(result.data.begin()) ri = result.data.begin();
  FOR_EACH(xi, x.data) {
    (*ri) = std::max(*xi, *yi);
    yi++;
    ri++;
  }
}

// result = max(result,x)
inline void max_assign(dvector &result, const dvector &x) {
  assert(result.size() == x.size());

  typeof(x.data.begin()) xi = x.data.begin();
  double xval;

  FOR_EACH(ri, result.data) {
    xval = *xi;
    if (xval > (*ri)) (*ri) = xval;
    xi++;
  }
}

// return x' * y
inline double inner_prod(const dvector &x, const cvector &y) {
  assert(x.size() == y.size());
  double sum = 0.0;
  FOR_EACH(yi, y.data) { sum += x(yi->index) * yi->value; }
  return sum;
}

// result = x .* y [for all i, result(i) = x(i) * y(i)]
template <class T, class U>
double inner_prod_cvector_internal(T xbegin, T xend, U ybegin, U yend) {
  double sum = 0.0;
  U yi = ybegin;
  for (T xi = xbegin; xi != xend; xi++) {
    while (1) {
      if (yi == yend) return sum;
      if (yi->index >= xi->index) {
        if (yi->index == xi->index) {
          sum += xi->value * yi->value;
        }
        break;
      }
      yi++;
    }
  }
  return sum;
}

// return x' * y
inline double inner_prod(const cvector &x, const cvector &y) {
  assert(x.size() == y.size());
  return inner_prod_cvector_internal(x.data.begin(), x.data.end(),
                                     y.data.begin(), y.data.end());
}

// return A(:,c)' * x
inline double inner_prod_column(const cmatrix &A, unsigned int c,
                                const cvector &x) {
  assert(A.size1() == x.size());
  assert(0 <= c && c < A.size2());
  return inner_prod_cvector_internal(A.data.begin() + A.col_starts[c],
                                     A.data.begin() + A.col_starts[c + 1],
                                     x.data.begin(), x.data.end());
}

inline void add(cvector &result, const cvector &x, const cvector &y) {
  typeof(x.data.begin()) xi, xend;
  typeof(y.data.begin()) yi, yend;
  unsigned int xind, yind;
  bool xdone = false, ydone = false;

  assert(x.size() == y.size());
  result.resize(x.size());

#define CHECK_X()        \
  if (xi == xend) {      \
    xdone = true;        \
    goto main_loop_done; \
  } else {               \
    xind = xi->index;    \
  }

#define CHECK_Y()        \
  if (yi == yend) {      \
    ydone = true;        \
    goto main_loop_done; \
  } else {               \
    yind = yi->index;    \
  }

  xi = x.data.begin();
  yi = y.data.begin();
  xend = x.data.end();
  yend = y.data.end();

  CHECK_X();
  CHECK_Y();

  while (1) {
    if (xind < yind) {
      result.push_back(xind, xi->value);
      xi++;
      CHECK_X();
    } else if (xind == yind) {
      result.push_back(xind, xi->value + yi->value);
      xi++;
      yi++;
      CHECK_X();
      CHECK_Y();
    } else {
      result.push_back(yind, yi->value);
      yi++;
      CHECK_Y();
    }
  }

main_loop_done:
  if (!xdone) {
    for (; xi != xend; xi++) {
      result.push_back(xi->index, xi->value);
    }
  } else if (!ydone) {
    for (; yi != yend; yi++) {
      result.push_back(yi->index, yi->value);
    }
  }

  result.canonicalize();
}

inline void subtract(cvector &result, const cvector &x, const cvector &y) {
  typeof(x.data.begin()) xi, xend;
  typeof(y.data.begin()) yi, yend;
  unsigned int xind, yind;
  bool xdone = false, ydone = false;

  assert(x.size() == y.size());
  result.resize(x.size());

#define CHECK_X()        \
  if (xi == xend) {      \
    xdone = true;        \
    goto main_loop_done; \
  } else {               \
    xind = xi->index;    \
  }

#define CHECK_Y()        \
  if (yi == yend) {      \
    ydone = true;        \
    goto main_loop_done; \
  } else {               \
    yind = yi->index;    \
  }

  xi = x.data.begin();
  yi = y.data.begin();
  xend = x.data.end();
  yend = y.data.end();

  CHECK_X();
  CHECK_Y();

  while (1) {
    if (xind < yind) {
      result.push_back(xind, xi->value);
      xi++;
      CHECK_X();
    } else if (xind == yind) {
      result.push_back(xind, xi->value - yi->value);
      xi++;
      yi++;
      CHECK_X();
      CHECK_Y();
    } else {
      result.push_back(yind, -yi->value);
      yi++;
      CHECK_Y();
    }
  }

main_loop_done:
  if (!xdone) {
    for (; xi != xend; xi++) {
      result.push_back(xi->index, xi->value);
    }
  } else if (!ydone) {
    for (; yi != yend; yi++) {
      result.push_back(yi->index, -yi->value);
    }
  }

  result.canonicalize();
}

// return true if for all i: x(i) >= y(i) - eps
inline bool dominates(const dvector &x, const dvector &y, double eps) {
  FOR(i, x.size()) {
    if (x(i) < y(i) - eps) return false;
  }
  return true;
}

// return true if for all i: x(i) >= y(i) - eps
inline bool dominates(const cvector &x, const cvector &y, double eps) {
  typeof(x.data.begin()) xi, xend;
  typeof(y.data.begin()) yi, yend;
  unsigned int xind, yind;
  bool xdone = false, ydone = false;

  assert(x.size() == y.size());

#define CHECK_X()        \
  if (xi == xend) {      \
    xdone = true;        \
    goto main_loop_done; \
  } else {               \
    xind = xi->index;    \
  }

#define CHECK_Y()        \
  if (yi == yend) {      \
    ydone = true;        \
    goto main_loop_done; \
  } else {               \
    yind = yi->index;    \
  }

  xi = x.data.begin();
  yi = y.data.begin();
  xend = x.data.end();
  yend = y.data.end();

  CHECK_X();
  CHECK_Y();

  while (1) {
    if (xind < yind) {
      // x(xind) == xi->value, y(xind) == 0
      if (xi->value < -eps) return false;
      xi++;
      CHECK_X();
    } else if (xind == yind) {
      // x(xind) == xi->value, y(xind) == yi->value
      if (xi->value < yi->value - eps) return false;
      xi++;
      yi++;
      CHECK_X();
      CHECK_Y();
    } else {
      // x(yind) == 0, y(yind) == yi->value
      if (0 < yi->value - eps) return false;
      yi++;
      CHECK_Y();
    }
  }

main_loop_done:
  if (!xdone) {
    for (; xi != xend; xi++) {
      if (xi->value < -eps) return false;
    }
  } else if (!ydone) {
    for (; yi != yend; yi++) {
      if (0 < yi->value - eps) return false;
    }
  }

  return true;
}

template <class T>
void read_from_file(T &x, const std::string &file_name) {
  std::ifstream in(file_name.c_str());
  if (!in) {
    std::cerr << "ERROR: couldn't open " << file_name
              << " for reading: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  x.read(in);
  in.close();
}

template <class T>
void write_to_file(const T &x, const std::string &file_name) {
  std::ofstream out(file_name.c_str());
  if (!out) {
    std::cerr << "ERROR: couldn't open " << file_name
              << " for writing: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  x.write(out);
  out.close();
}

}  // namespace sla

/**********************************************************************
 * TYPE ALIASES
 **********************************************************************/

// select storage types of the data structures we use
typedef sla::cvector state_vector;
typedef sla::cvector belief_vector;
typedef sla::cvector alpha_vector;
typedef sla::dvector outcome_prob_vector;
typedef sla::dvector obs_prob_vector;

#endif  // ZMDP_SRC_COMMON_SLA_H_
