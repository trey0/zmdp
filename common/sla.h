/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.4 $  $Author: trey $  $Date: 2005-01-28 03:20:45 $
 *  
 * @file    sla.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCsla_h
#define INCsla_h

#include <vector>
#include <fstream>

#include "commonDefs.h"

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

    double& operator()(unsigned int i) { return data[i]; }
    double operator()(unsigned int i) const { return data[i]; }
    void operator*=(double s);
    void operator+=(const dvector& x);
    void operator-=(const dvector& x);

    unsigned int size(void) const { return data.size(); }
    void resize(unsigned int _size, double value = 0.0);

    void read(std::istream& in);
    void write(std::ostream& out) const;
  };

  /**********************************************************************
   * CVECTOR
   **********************************************************************/

  struct cvector_entry {
    unsigned int index;
    double value;

    cvector_entry(void) {}
    cvector_entry(unsigned int _index,
		     double _value) :
      index(_index),
      value(_value)
    {}
  };
  
  struct cvector {
    unsigned int size_;
    std::vector< cvector_entry > data;
    
    cvector(void) : size_(0) {}
    explicit cvector(unsigned int _size) { resize(_size); }

    double operator()(unsigned int index) const;
    void operator*=(double s);

    unsigned int size(void) const { return size_; }
    unsigned int filled(void) const { return data.size(); }

    void resize(unsigned int _size, unsigned int _non_zeros = 0);
    void push_back(unsigned int index, double value);
    void canonicalize(void) {}

    void read(std::istream& in);
  };
  
  /**********************************************************************
   * DMATRIX
   **********************************************************************/

  struct dmatrix {
    unsigned int size1_, size2_;
    std::vector< double > data;

    double& operator()(unsigned int r, unsigned int c);
    double operator()(unsigned int r, unsigned int c) const;

    unsigned int size1(void) const { return size1_; }
    unsigned int size2(void) const { return size2_; }
    void resize(unsigned int _size1, unsigned int _size2, double value = 0.0);
  };

  /**********************************************************************
   * KMATRIX
   **********************************************************************/

  struct kmatrix_entry {
    unsigned int r, c;
    double value;

    kmatrix_entry(void) {}
    kmatrix_entry(unsigned int _r,
		  unsigned int _c,
		  double _value) :
      r(_r),
      c(_c),
      value(_value)
    {}
  };

  struct kmatrix {
    unsigned int size1_, size2_;
    std::vector< kmatrix_entry > data;

    kmatrix(void) : size1_(0), size2_(0) {}
    kmatrix(unsigned int _size1, unsigned int _size2) { resize(_size1,_size2); }

    double operator()(unsigned int r, unsigned int c) const;

    unsigned int size1(void) const { return size1_; }
    unsigned int size2(void) const { return size2_; }
    unsigned int filled(void) const { return data.size(); }
    void resize(unsigned int _size1, unsigned int _size2, double value = 0.0);
    void push_back(unsigned int r, unsigned int c, double value);
    void canonicalize(void);

    void read(std::istream& in);
  };

  /**********************************************************************
   * CMATRIX
   **********************************************************************/

  struct cmatrix {
    unsigned int size1_, size2_;
    std::vector< unsigned int > col_starts;
    std::vector< cvector_entry > data;

    cmatrix(void) : size1_(0), size2_(0) {}
    cmatrix(unsigned int _size1, unsigned int _size2) { resize(_size1,_size2); }

    double operator()(unsigned int r, unsigned int c) const;

    unsigned int size1(void) const { return size1_; }
    unsigned int size2(void) const { return size2_; }
    unsigned int filled(void) const { return data.size(); }
    void resize(unsigned int _size1, unsigned int _size2, unsigned int _non_zeros = 0);
    void push_back(unsigned int row, unsigned int col, double value);

    // if resize()/push_back() are used to initialize, you must call
    // canonicalize() before performing any operations with the matrix
    void canonicalize(void);

    void read(std::istream& in);
    void write(std::ostream& out) const;
  };
  
  /**********************************************************************
   * NON-MEMBER FUNCTION PROTOTYPES
   **********************************************************************/

  // result = x
  void dvector_from_cvector(dvector& result, const cvector& x);

  // result = x
  void cvector_from_dvector(cvector& result, const dvector& x);

  // result = A (side-effect: canonicalizes A)
  void cmatrix_from_kmatrix(cmatrix& result, kmatrix& A);

  // result = A(.,c)
  void cvector_from_cmatrix_column(cvector& result, const cmatrix& A,
				   unsigned int c);

  // A(r,c) = v
  void kmatrix_set_entry(kmatrix& A, unsigned int r, unsigned int c,
			 double v);

  // A = A'
  void kmatrix_transpose_in_place(kmatrix& A);

  double norm_1(const cvector& x);

  double norm_inf(const dvector& x);

  // result = A * x
  void mult(dvector& result, const cmatrix& A, const cvector& x);


  // result = A * x
  void mult(cvector& result, const cmatrix& A, const cvector& x);

  // result = x * A
  void mult(dvector& result, const dvector& x, const cmatrix& A);

  // result = x .* y [for all i, result(i) = x(i) * y(i)]
  void emult(cvector& result, const cvector& x, const cvector& y);

  // result = A(:,c) .* x
  void emult_column(cvector& result, const cmatrix& A, unsigned int c,
		    const cvector& x);

  // result = x .* y
  void emult(dvector& result, const dvector& x, const cvector& y);

  // result = x .* A(:,c)
  void emult_column(dvector& result, const dvector& x,
		    const cmatrix& A, unsigned int c);

  // return x' * y
  double inner_prod(const dvector& x, const cvector& y);

  // return x' * y
  double inner_prod(const cvector& x, const cvector& y);

  // return A(:,c)' * y
  double inner_prod_column(const cmatrix& A, unsigned int c,
			   const cvector& y);

  template <class T>
  void read_from_file(T& x, const std::string& file_name);
			  
  template <class T>
  void write_to_file(const T& x, const std::string& file_name);
			  
  /**********************************************************************
   * DVECTOR FUNCTIONS
   **********************************************************************/

  inline void dvector::operator*=(double s)
  {
    FOR_EACH (di, data) {
      (*di) *= s;
    }
  }

#define DVECTOR_OPERATOR( OP )                       \
  inline void dvector::operator OP(const dvector& x) \
  {                                                  \
    typeof(x.data.begin()) xi;                       \
                                                     \
    assert( size() == x.size() );                    \
                                                     \
    xi = x.data.begin();                             \
    FOR_EACH (di, data) {                            \
      (*di) OP (*xi);                                \
      xi++;                                          \
    }                                                \
  }

  DVECTOR_OPERATOR( += );
  DVECTOR_OPERATOR( -= );

  inline void dvector::resize(unsigned int _size, double value)
  {
    assert(0 == value);
    data.resize( _size );
    FOR_EACH (di, data) {
      (*di) = 0.0;
    }
  }

  inline void dvector::read(std::istream& in)
  {
    int num_entries;
    
    in >> num_entries;
    resize( num_entries );
    FOR (i, num_entries) {
      in >> data[i];
    }
    
#if 0
    std::cout << "dense read: size = " << data.size() << std::endl;
    FOR (i, data.size()) {
      if (i == 10) {
	std::cout << "..." << std::endl;
	break;
      }
      std::cout << i << " " << data[i] << std::endl;
    }
#endif
  }

  inline void dvector::write(std::ostream& out) const
  {
    out << size() << std::endl;
    FOR_EACH (x, data) {
      out << (*x) << std::endl;
    }
  }

  /**********************************************************************
   * CVECTOR FUNCTIONS
   **********************************************************************/

  inline double cvector::operator()(unsigned int index) const
  {
    FOR_EACH (di, data) {
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

  inline void cvector::operator*=(double s)
  {
    typeof(data.begin()) vi, vend = data.end();
    for (vi = data.begin(); vi != vend; vi++) {
      vi->value *= s;      
    }
  }

  inline void cvector::resize(unsigned int _size, unsigned int _non_zeros)
  {
    assert( 0 == _non_zeros );
    size_ = _size;
    data.clear();
  }

  inline void cvector::push_back(unsigned int index, double value)
  {
    data.push_back( cvector_entry( index, value ) );
  }

  inline void cvector::read(std::istream& in)
  {
    int num_entries;
    
    in >> size_;
    in >> num_entries;
    data.resize( num_entries );
    FOR (i, num_entries) {
      in >> data[i].index >> data[i].value;
    }
  }

  /**********************************************************************
   * DMATRIX FUNCTIONS
   **********************************************************************/

  inline double& dmatrix::operator()(unsigned int r, unsigned int c)
  {
    return data[ c * size1_ + r ];
  }

  inline double dmatrix::operator()(unsigned int r, unsigned int c) const
  {
    return data[ c * size1_ + r ];
  }

  inline void dmatrix::resize(unsigned int _size1, unsigned int _size2,
			      double value)
  {
    assert( 0 == value );
    size1_ = _size1;
    size2_ = _size2;
    data.resize( size1_ * size2_ );
    FOR_EACH (di, data) {
      (*di) = 0.0;
    }
  }

  /**********************************************************************
   * KMATRIX FUNCTIONS
   **********************************************************************/

  inline double kmatrix::operator()(unsigned int r, unsigned int c) const
  {
    assert( 0 <= r < size1() );
    assert( 0 <= c < size2() );
    // NOTE: also assumes the kmatrix has been canonicalized

    FOR_EACH (di, data) {
      if (di->r == r && di->c == c) {
	return di->value;
      }
    }
    return 0.0;
  }

  inline void kmatrix::resize(unsigned int _size1, unsigned int _size2,
			      double value)
  {
    assert( 0 == value );
    size1_ = _size1;
    size2_ = _size2;
    data.clear();
  }

  inline void kmatrix::push_back(unsigned int r, unsigned int c, double value)
  {
    data.push_back( kmatrix_entry(r,c,value) );
  }
  
  struct ColumnMajorCompare {
    bool operator()(const kmatrix_entry& lhs, const kmatrix_entry& rhs) {
      return (lhs.c < rhs.c) || ((lhs.c == rhs.c) && (lhs.r < rhs.r));
    }
  };

  inline bool rc_equal(const kmatrix_entry& lhs, const kmatrix_entry& rhs)
  {
    return (lhs.r == rhs.r) && (lhs.c == rhs.c);
  }

  inline void kmatrix::canonicalize(void)
  {
    std::vector< kmatrix_entry > d;

    // sort in column-major order
    std::stable_sort( data.begin(), data.end(), ColumnMajorCompare() );
    
    // ensure there is at most one entry with each (r,c) coordinate.
    // among all the entries with the same (r,c), keep the last one.
    // note that this operation does *not* get rid of near-zero entries.
    FOR ( i, data.size() ) {
      if (!d.empty() && rc_equal( d.back(), data[i] )) {
	d.back() = data[i];
      } else {
	d.push_back( data[i] );
      }
    }
    data.swap( d );
  }

  inline void kmatrix::read(std::istream& in)
  {
    int rows, cols;
    int num_entries;
    int r, c;
    double value;
    
    in >> rows >> cols;
    resize( rows, cols );

    in >> num_entries;
    FOR (i, num_entries) {
      in >> r >> c >> value;
      push_back( r, c, value );
    }
  }

  /**********************************************************************
   * CMATRIX FUNCTIONS
   **********************************************************************/

  inline double cmatrix::operator()(unsigned int r, unsigned int c) const
  {
    typeof(data.begin()) di;
    typeof(data.begin()) col_end = data.begin() + col_starts[c+1];

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

  inline void cmatrix::resize(unsigned int _size1,
			      unsigned int _size2,
			      unsigned int _non_zeros)
  {
    assert( 0 == _non_zeros );
    size1_ = _size1;
    size2_ = _size2;
    col_starts.resize( size2()+1 );
    FOR_EACH (ci, col_starts) {
      (*ci) = 0;
    }
    data.clear();
  }

  inline void cmatrix::push_back(unsigned int r,
				 unsigned int c,
				 double value)
  {
    data.push_back( cvector_entry( r, value ) );
    col_starts[c+1] = data.size();
  }

  inline void cmatrix::canonicalize(void)
  {
    FOR (i, size2_) {
      if (col_starts[i] > col_starts[i+1]) {
	col_starts[i+1] = col_starts[i];
      }
    }
  }

  inline void cmatrix::read(std::istream& in)
  {
    kmatrix km;
    km.read(in);
    cmatrix_from_kmatrix(*this,km);
  }

  inline void cmatrix::write(std::ostream& out) const
  {
    typeof(data.begin()) di, col_end;

    out << size1_ << " " << size2_ << std::endl;
    out << data.size() << std::endl;
    FOR (c, size2_) {
      col_end = data.begin() + col_starts[c+1];
      for (di = data.begin() + col_starts[c]; di != col_end; di++) {
	out << di->index << " " << c << " " << di->value << std::endl;
      }
    }
  }

  /**********************************************************************
   * NON-MEMBER FUNCTIONS
   **********************************************************************/

  inline void dvector_from_cvector(dvector& result, const cvector& x)
  {
    result.resize( x.size() );
    FOR_EACH (xi, x.data) {
      result.data[xi->index] = xi->value;
    }
  }

  inline void cvector_from_dvector(cvector& result, const dvector& x)
  {
    int num_filled;
    int i;
    typeof(result.data.begin()) ri;
    
    // count non-zeros
    num_filled = 0;
    FOR_EACH (xi, x.data) {
      if (fabs(*xi) > SPARSE_EPS) num_filled++;
    }
    //std::cout << "convert: num_filled=" << num_filled << std::endl;
    
    // resize result vector
    result.resize( x.size() );
    result.data.resize(num_filled);
    
    // copy non-zeros to result
    i = 0;
    ri = result.data.begin();
    FOR_EACH (xi, x.data) {
      if (fabs(*xi) > SPARSE_EPS) {
	ri->index = i;
	ri->value = *xi;
	ri++;
      }
      i++;
    }
  }
  
  inline void cmatrix_from_kmatrix(cmatrix& result, kmatrix& A)
  {
    A.canonicalize();
    result.resize( A.size1(), A.size2() );
    FOR_EACH (entry, A.data) {
      if (fabs(entry->value) > SPARSE_EPS) {
	result.push_back( entry->r, entry->c, entry->value );
      }
    }
    result.canonicalize();
  }

  // result = A(:,c)
  inline void cvector_from_cmatrix_column(cvector& result, const cmatrix& A, unsigned int c)
  {
    assert( 0 <= c && c < A.size2() );

    typeof(A.data.begin()) Ai, col_start, col_end;
    typeof(result.data.begin()) ri;

    col_start = A.data.begin() + A.col_starts[c];
    col_end   = A.data.begin() + A.col_starts[c+1];

    result.resize( A.size1() );
    result.data.resize( col_end - col_start );
    for (Ai = col_start, ri=result.data.begin(); Ai != col_end; Ai++, ri++) {
      (*ri) = (*Ai);
    }
  }

  // result = A(:,c)
  inline void dvector_from_cmatrix_column(dvector& result, const cmatrix& A, unsigned int c)
  {
    assert( 0 <= c && c < A.size2() );
    typeof(A.data.begin()) Ai, col_end;

    result.resize( A.size1() );

    col_end = A.data.begin() + A.col_starts[c+1];
    for (Ai = A.data.begin() + A.col_starts[c]; Ai != col_end; Ai++) {
      result(Ai->index) = Ai->value;
    }
  }

  // A(r,c) = v
  inline void kmatrix_set_entry(kmatrix& A, unsigned int r, unsigned int c,
				double v)
  {
    A.push_back( r, c, v );
  }

  // A = A'
  inline void kmatrix_transpose_in_place(kmatrix& A)
  {
    FOR_EACH ( Ai, A.data ) {
      std::swap( Ai->r, Ai->c );
    }
    A.canonicalize();
  }

  inline double norm_1(const cvector& x)
  {
    double sum = 0.0;
    FOR_EACH (xi, x.data) {
      sum += fabs(xi->value);
    }
    return sum;
  }

  inline double norm_inf(const dvector& x)
  {
    double val, max = 0.0;
    FOR_EACH (xi, x.data) {
      val = fabs(*xi);
      if (val > max) max = val;
    }
    return max;
  }

  // result = A * x
  inline void mult(dvector& result,
		   const cmatrix& A,
		   const cvector& x)
  {
    typeof(A.data.begin()) Ai, col_end;
    int xind;
    double xval;
    
    result.resize( x.size() );
    
    FOR_EACH (xi, x.data) {
      xind = xi->index;
      xval = xi->value;
      col_end = A.data.begin() + A.col_starts[xind+1];
      for (Ai = A.data.begin() + A.col_starts[xind];
	   Ai != col_end;
	   Ai++) {
	result.data[Ai->index] += xval * Ai->value;
      }
    }
  }

  // result = A * x
  inline void mult(cvector& result,
		   const cmatrix& A,
		   const cvector& x)
  {
    dvector tmp;
    
    mult( tmp, A, x );
    cvector_from_dvector( result, tmp );
  }
  
  // result = x * A
  inline void mult(dvector& result, const dvector& x, const cmatrix& A)
  {
    typeof(A.data.begin()) Ai, col_end;

    assert( x.size() == A.size1() );
    result.resize( A.size2() );

    FOR (c, A.size2()) {
      col_end = A.data.begin() + A.col_starts[c+1];
      for (Ai = A.data.begin() + A.col_starts[c];
	   Ai != col_end; Ai++) {
	result(c) += x(Ai->index) * Ai->value;
      }
    }
  }

  // result = x .* y [for all i, result(i) = x(i) * y(i)]
  template <class T, class U>
  void emult_cc_internal(cvector& result,
			 T xbegin, T xend,
			 U ybegin, U yend)
  {
    U yi = ybegin;
    for (T xi = xbegin; xi != xend; xi++) {
      while (1) {
	if (yi == yend) return;
	if (yi->index >= xi->index) {
	  if (yi->index == xi->index) {
	    result.push_back( xi->index, xi->value * yi->value);
	  }
	  break;
	}
	yi++;
      }
    }
  }

  // result = x .* y [for all i, result(i) = x(i) * y(i)]
  inline void emult(cvector& result, const cvector& x, const cvector& y) {
    assert( x.size() == y.size() );
    result.resize( x.size() );

    emult_cc_internal( result, x.data.begin(), x.data.end(),
		       y.data.begin(), y.data.end() );

    result.canonicalize();
  }

  // result = A(:,c) .* x
  inline void emult_column(cvector& result, const cmatrix& A, unsigned int c,
			   const cvector& x)
  {
    assert( A.size1() == x.size() );
    assert( 0 <= c && c < A.size2() );
    result.resize( x.size() );

    emult_cc_internal( result,
		       A.data.begin() + A.col_starts[c],
		       A.data.begin() + A.col_starts[c+1],
		       x.data.begin(), x.data.end() );

    result.canonicalize();
  }

  // result = x .* y [for all i, result(i) = x(i) * y(i)]
  template <class T>
  void emult_dc_internal(dvector& result,
			 const dvector& x,
			 T ybegin, T yend)
  {
    int yind;
    for (T yi = ybegin; yi != yend; yi++) {
      yind = yi->index;
      result(yind) = x(yind) * yi->value;
    }
  }

  // result = x .* y
  inline void emult(dvector& result, const dvector& x, const cvector& y)
  {
    assert( x.size() == y.size() );
    result.resize( x.size() );
    emult_dc_internal( result, x, y.data.begin(), y.data.end() );
  }

  // result = x .* A(:,c)
  inline void emult_column(dvector& result, const dvector& x,
			   const cmatrix& A, unsigned int c)
  {
    assert( A.size1() == x.size() );
    assert( 0 <= c && c < A.size2() );
    result.resize( x.size() );
    emult_dc_internal( result, x,
		       A.data.begin() + A.col_starts[c],
		       A.data.begin() + A.col_starts[c+1] );
  }

  // return x' * y
  inline double inner_prod(const dvector& x, const cvector& y)
  {
    assert( x.size() == y.size() );
    double sum = 0.0;
    FOR_EACH (yi, y.data) {
      sum += x(yi->index) * yi->value;
    }
    return sum;
  }

  // result = x .* y [for all i, result(i) = x(i) * y(i)]
  template <class T, class U>
  double inner_prod_cvector_internal(T xbegin, T xend,
				     U ybegin, U yend)
  {
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
  inline double inner_prod(const cvector& x, const cvector& y)
  {
    assert( x.size() == y.size() );
    return inner_prod_cvector_internal( x.data.begin(), x.data.end(),
					y.data.begin(), y.data.end() );
  }

  // return A(:,c)' * x
  inline double inner_prod_column(const cmatrix& A, unsigned int c,
				  const cvector& x)
  {
    assert( A.size1() == x.size() );
    assert( 0 <= c && c < A.size2() );
    return inner_prod_cvector_internal( A.data.begin() + A.col_starts[c],
					A.data.begin() + A.col_starts[c+1],
					x.data.begin(), x.data.end() );
  }

  template <class T>
  void read_from_file(T& x, const std::string& file_name)
  {
    std::ifstream in(file_name.c_str());
    if (!in) {
      std::cerr << "ERROR: couldn't open " << file_name << " for reading: "
		<< strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    x.read(in);
    in.close();
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
    x.write(out);
    out.close();
  }
			  
} // namespace sla

/**********************************************************************
 * TYPE ALIASES
 **********************************************************************/

// select storage types of the data structures we use
typedef sla::cvector belief_vector;
typedef sla::dvector alpha_vector;
typedef sla::dvector obs_prob_vector;

#endif // INCsla_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/01/27 05:34:10  trey
 * added several functions and fixed bugs
 *
 * Revision 1.2  2005/01/26 04:10:12  trey
 * implemented several functions
 *
 * Revision 1.1  2005/01/21 18:07:02  trey
 * preparing for transition to sla matrix types
 *
 *
 ***************************************************************************/
