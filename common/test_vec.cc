#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

// I created this file in order to answer some questions about the
// semantics of the boost matrix library.
//
// BEHAVIOR OF SPARSE VECTORS/MATRICES:
//   - uninitialized entries are zero
//   - size grows automatically for new entries
//   - sparse matrix multiplication properly returns sparse output
//   - explicitly assigning any value to an element, including 0,
//     makes it a "non-zero"
//   - assignment w = v from a dense vector does the right
//     thing (i.e., only creates non-zeroes for v(i) != 0)
//   - resize(n, 0) zeroes the entries of a sparse vector

// ASSIGNMENT / CONSTRUCTOR SEMANTICS:
//   - w = v for vectors is valid if w and v have the same size, or if
//     w is uninitialized.  it does an element-wise copy, so later mods to
//     w do not affect v.
//   - copy constructor seems to be the same as assignment operator
//   - there doesn't appear to be a way to efficiently return vectors
//     from functions (since return calls the copy constructor)
//   - can't create a vector from a matrix or vice versa

// compile with 'g++ -o test_vec test_vec.cc -I../../external/include'

#define FOR_CM_ROWS(r, M) \
  for (unsigned r=0; r < M.size1(); r++)

#define FOR_CM_COLS(r, M) \
  unsigned __cm_begin = M.index1_data()[r]; \
  unsigned __cm_end   = M.index1_data()[r+1]; \
  for (unsigned __cm_j=__cm_begin; __cm_j < __cm_end; __cm_j++)

#define CM_VAL(M) \
  M.value_data()[__cm_j]

#define CM_COL(M) \
  M.index2_data()[__cm_j]

#define FOR_CV(v) \
  for (unsigned __cv_i=0; __cv_i < v.non_zeros(); __cv_i++)

#define CV_VAL(v) \
  v.value_data()[__cv_i]

#define CV_INDEX(v) \
  v.index_data()[__cv_i]


int main () {
    using namespace boost::numeric::ublas;

    compressed_matrix<double> m(3,3);
    m(1,2) = 3;
    m(2,2) = 4;

    FOR_CM_ROWS(r, m) {
      FOR_CM_COLS(r, m) {
	std::cout << "r = " << r
		  << ", c = " << CM_COL(m)
		  << ", val = " << CM_VAL(m) << std::endl;
      }
    }

    compressed_vector<double> v(3);
    v(1) = 7;
    v(2) = 3;
    FOR_CV(v) {
      std::cout << "index = " << CV_INDEX(v)
		<< ", val = " << CV_VAL(v) << std::endl;
    }

#if 0
    sparse_vector<double> v(3);

    v(0) = 1;
    v(1) = 2;
    sparse_vector<double> w(v);
    w(0) = 3;
    sparse_vector<double> x(4);
    //x = v;
    x(0) = 4;

    std::cout << "v=" << v << std::endl;
    std::cout << "w=" << w << std::endl;
    std::cout << "x=" << w << std::endl;

#if 0
    sparse_matrix<double> m(1,3);
    m = v; // doesn't work...
    std::cout << "m=" << m << std::endl;
#endif
    sparse_matrix<double> m = outer_prod( v, v );
    std::cout << "m=" << m << std::endl;

#endif

#if 0
    sparse_vector<double> v (3, 1);
    sparse_matrix<double> m (3, 3, 1);

    v(0) = 0;
    v(1) = 2;

    m(1,1) = 5;
    m(2,2) = 7;

    sparse_vector<double> w = prod(m,v);

    std::cout << "m=" << m << std::endl;
    //std::cout << "m.size=" << m.size() << std::endl;

    typeof(m.begin1()) iter1;
    typeof(m.begin1().begin()) iter2;
    for (iter1 = m.begin1(); iter1 != m.end1(); iter1++) {
      for (iter2 = iter1.begin(); iter2 != iter1.end(); iter2++) {
	std::cout << "  " << (*iter2) << std::endl;
      }
    }

    std::cout << "take 2" << std::endl;
    typeof(m.data().begin()) it;
    for (it = m.data().begin(); it != m.data().end(); it++) {
      std::cout << "  " << (it->first) << " " << (it->second) << std::endl;
    }

    std::cout << "v=" << v << std::endl;
    std::cout << "v.size=" << v.size() << std::endl;
    std::cout << "v.non_zeros=" << v.non_zeros() << std::endl;

    std::cout << "w=" << w << std::endl;
    std::cout << "w.non_zeros=" << w.non_zeros() << std::endl;

    vector<double> x(3);
    x(0) = 1;
    x(1) = 0;
    x(2) = 0;

    w = x;

    std::cout << "w=" << w << std::endl;
    std::cout << "w.non_zeros=" << w.non_zeros() << std::endl;

    w(0) = 0;
    std::cout << "w=" << w << std::endl;
    std::cout << "w.non_zeros=" << w.non_zeros() << std::endl;

    v.resize(3, 0);
    std::cout << "v=" << v << std::endl;
    std::cout << "v.non_zeros=" << v.non_zeros() << std::endl;
#endif
}

