/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2005-03-10 21:13:23 $
 *  
 * @file    sla_mask.h
 * @brief   Implements operations for masked compressed vectors.
 ***************************************************************************/

#ifndef INCsla_mask_h
#define INCsla_mask_h

#include "sla.h"

namespace sla {

  // current representation of mvector is as a cvector -- all
  // non-zeros of the cvector are elements of the mask
  typedef cvector mvector;

  /**********************************************************************
   * PROTOTYPES
   **********************************************************************/

  // m = ones(1,msize)
  void set_to_all_ones(mvector& m, int msize);

  // return true if non-zeros of x constitute a subset of m
  bool mask_subset(const cvector& x, const mvector& m);

  // for all i: result(i) = m(i) ? x(i) : 0
  void mask_copy(cvector& result,
		 const dvector& x,
		 const mvector& m);

  // for all i: result(i) = mask(i) ? x(i) : 0
  void mask_copy(cvector& result,
		 const cvector& x,
		 const mvector& m);

  // return true if [ym is a subset of xm] and [for all i: x(i) >= y(i) - eps]
  bool mask_dominates(const cvector& x, const cvector& y, double eps,
		      const mvector& xm, const mvector& ym);


  /**********************************************************************
   * FUNCTIONS
   **********************************************************************/

  // m = ones(msize)
  inline void set_to_all_ones(mvector& m, int msize)
  {
    m.resize( msize );
    FOR (i, msize) {
      m.push_back( i, 1 );
    }
    m.canonicalize();
  }

  // return true if non-zeros of x constitute a subset of m
  inline bool mask_subset(const cvector& x, const mvector& m)
  {
    assert( x.size() == m.size() );
    typeof(x.data.begin()) xi, xend = x.data.end();
    typeof(m.data.begin()) mi, mend = m.data.end();

    mi = m.data.begin();
    for (xi = x.data.begin(); xi != xend; xi++) {
      for (; mi != mend; mi++) {
	if (xi->index < mi->index) return false;
	if (xi->index == mi->index) {
	  mi++;
	  goto next_xi;
	}
      }
      return false;

    next_xi:
      ;
    }
    return true;
  }

  // for all i: result(i) = m(i) ? x(i) : 0
  inline void mask_copy(cvector& result,
			const dvector& x,
			const mvector& m)
  {
    assert( x.size() == m.size() );
    int mind;

    result.resize( x.size() );
    FOR_EACH (mi, m.data) {
      mind = mi->index;
      result.push_back( mind, x(mind) );
    }
    result.canonicalize();
  }

  // for all i: result(i) = mask(i) ? x(i) : 0
  inline void mask_copy(cvector& result,
			const cvector& x,
			const mvector& m)
  {
    assert( x.size() == m.size() );
    typeof(x.data.begin()) xi, xend = x.data.end();
    typeof(m.data.begin()) mi, mend = m.data.end();

    result.resize( x.size() );

    mi = m.data.begin();
    for (xi = x.data.begin(); xi != xend; xi++) {
      for (; mi != mend; mi++) {
	if (mi->index >= xi->index) {
	  if (mi->index == xi->index) {
	    result.push_back( xi->index, xi->value );
	  }
	  goto next_xi;
	}
      }
      break;

    next_xi:
      ;
    }
    result.canonicalize();
  }

  // return true if [ym is a subset of xm] and [for all i: x(i) >= y(i) - eps]
  inline bool mask_dominates(const cvector& x, const cvector& y, double eps,
			     const mvector& xm, const mvector& ym)
  {
    return mask_subset(ym,xm) && dominates(x,y,eps);
  }

  //void mask_mult( cvector, cvector, cmatrix, mask );
			  
} // namespace sla

#endif // INCsla_mask_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
