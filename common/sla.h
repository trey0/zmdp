/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2005-01-21 18:07:02 $
 *  
 * @file    sla.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCsla_h
#define INCsla_h

#define SPARSE_EPS (1e-10)

// sla     = simple linear algebra
// dvector = dense vector
// cvector = compressed vector

namespace sla {

  /**********************************************************************
   * CLASSES
   **********************************************************************/

  struct dvector {
    std::vector<double> data;
    
    void resize(unsigned int _size);
    void resize(unsigned int _size, double value);

    void read(std::istream& in);
    void read_from_file(const std::string& file_name);
    void write(std::ostream& out);
    void write_to_file(const std::string& file_name);
  };

  struct index_value_pair {
    unsigned int index;
    double value;

    index_value_pair(void) {}
    index_value_pair(unsigned int _index,
		     double _value) :
      index(_index),
      value(_value)
    {}
  };
  
  struct cvector {
    unsigned int size;
    std::vector< index_value_pair > data;
    
    void resize(unsigned int _size);
    void clear(void);
    void push_back(unsigned int index, double value);

    void read(std::istream& in);
    void read_from_file(const std::string& file_name);
  };
  
  struct cmatrix {
    unsigned int size1, size2;
    std::vector< unsigned int > row_starts;
    std::vector< index_value_pair > data;

    void resize(unsigned int _size1, unsigned int _size2);
    void clear(void);
    void push_back(unsigned int row, unsigned int col, double value);

    // if clear()/push_back() are used to initialize, you must call
    // canonicalize() before performing any operations with the matrix
    void canonicalize(void);

    void read(std::istream& in);
    void read_from_file(const std::string& file_name);
  };
  
  /**********************************************************************
   * NON-MEMBER FUNCTION PROTOTYPES
   **********************************************************************/

  // result = x
  void cvector_from_dvector(cvector& result,
			    const dvector& x);

  // result = x
  void dvector_from_cvector(dvector& result,
			    const cvector& x);

  // result = A * x
  void mult(dvector& result, const cmatrix& A, const cvector& x);


  // result = A * x
  void mult(cvector& result, const cmatrix& A, const cvector& x);

			  
  /**********************************************************************
   * DVECTOR FUNCTIONS
   **********************************************************************/

  inline void dvector::resize(unsigned int _size)
  {
    data.resize( _size );
  }

  inline void dvector::resize(unsigned int _size, double value)
  {
    data.resize( _size, value );
  }

  inline void dvector::read(std::istream& in)
  {
    int num_entries;
    
    in >> num_entries;
    data.resize( num_entries );
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

  inline void dvector::read_from_file(const std::string& file_name)
  {
    std::ifstream in(file_name.c_str());
    if (!in) {
      std::cerr << "ERROR: couldn't open " << file_name << " for reading: "
		<< strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    read(in);
    in.close();
  }

  inline void dvector::write(std::ostream& out)
  {
    out << data.size() << std::endl;
    FOR_EACH (x, data) {
      out << (*x) << std::endl;
    }
  }

  inline void dvector::write_to_file(const std::string& file_name)
  {
    std::ofstream out(file_name.c_str());
    if (!out) {
      std::cerr << "ERROR: couldn't open " << file_name << " for writing: "
		<< strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    write(out);
    out.close();
  }

  /**********************************************************************
   * CVECTOR FUNCTIONS
   **********************************************************************/

  inline void cvector::resize(unsigned int _size)
  {
    size = _size;
  }

  inline void cvector::clear(void)
  {
    data.clear();
  }

  inline void cvector::push_back(unsigned int index, double value)
  {
    data.push_back( index_value_pair( index, value ) );
  }

  inline void cvector::read(std::istream& in)
  {
    int num_entries;
    
    in >> size;
    in >> num_entries;
    data.resize( num_entries );
    FOR (i, num_entries) {
      in >> data[i].index >> data[i].value;
    }
  }

  inline void cvector::read_from_file(const std::string& file_name)
  {
    std::ifstream in(file_name.c_str());
    if (!in) {
      std::cerr << "ERROR: couldn't open " << file_name << " for reading: "
		<< strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    read(in);
    in.close();
  }

  /**********************************************************************
   * CMATRIX FUNCTIONS
   **********************************************************************/

  inline void cmatrix::resize(unsigned int _size1,
			      unsigned int _size2)
  {
    size1 = _size1;
    size2 = _size2;
    row_starts.resize( _size1 );
  }

  inline void cmatrix::clear(void)
  {
    data.clear();
    row_starts.resize( size1, 0 );
  }

  inline void cmatrix::push_back(unsigned int row,
				 unsigned int col,
				 double value)
  {
    data.push_back( index_value_pair( index, value ) );
    row_starts[index+1] = data.size();
  }

  inline void cmatrix::canonicalize(void)
  {
    FOR (i, row_starts.size()) {
      if (row_starts[i-1] > row_starts[i]) {
	row_starts[i] = row_starts[i-1];
      }
    }
  }

  inline void cmatrix::read(std::istream& in)
  {
    int num_entries;
    int row;
    
    in >> size1 >> size2;
    in >> num_entries;
    
    row_starts.resize( size1+1 );
    data.resize( num_entries );
    
    row_starts[0] = 0;
    FOR (i, num_entries) {
      in >> row >> data[i].index >> data[i].value;
      row_starts[row+1] = i+1;
    }
    canonicalize();
  }

  inline void cmatrix::read_from_file(const std::string& file_name)
  {
    std::ifstream in(file_name.c_str());
    if (!in) {
      std::cerr << "ERROR: couldn't open " << file_name << " for reading: "
		<< strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
    read(in);
    in.close();
  }

  /**********************************************************************
   * NON-MEMBER FUNCTIONS
   **********************************************************************/

  // result = x
  inline void cvector_from_dvector(cvector& result,
				   const dvector& x)
  {
    int num_non_zeros;
    int i;
    typeof(result.data.begin()) ri;
    
    // count non-zeros
    num_non_zeros = 0;
    FOR_EACH (xi, x.data) {
      if (fabs(*xi) > SPARSE_EPS) num_non_zeros++;
    }
    //std::cout << "convert: num_non_zeros=" << num_non_zeros << std::endl;
    
    // resize result vector
    result.size = x.data.size();
    result.data.resize(num_non_zeros);
    
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
  
  inline void dvector_from_cvector(dvector& result,
				   const cvector& x)
  {
    result.data.resize( x.size, 0.0 );
    FOR_EACH (xi, x.data) {
      result.data[xi->index] = xi->value;
    }
  }

  // result = A * x
  inline void mult(dvector& result,
		   const cmatrix& A,
		   const cvector& x)
  {
    typeof(A.data.begin()) Ai, row_end;
    int xind;
    double xval;
    
    result.data.resize( x.size, 0.0 );
    
    FOR_EACH (xi, x.data) {
      xind = xi->index;
      xval = xi->value;
      row_end = A.data.begin() + A.row_starts[xind+1];
      for (Ai = A.data.begin() + A.row_starts[xind];
	   Ai != row_end;
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
			  
} // namespace sla

#endif // INCsla_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
