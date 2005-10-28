/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2005-10-28 03:50:32 $
   
 @file    sla_mask.h
 @brief   Implements operations for masked compressed vectors.

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
  void mask_set_all(mvector& m, int msize);

  // for all i: result(i) = m(i) ? 1 : 0
  void mask_set_to_one(cvector& result, const mvector& m);

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
  inline void mask_set_all(mvector& m, int msize)
  {
    m.resize( msize );
    FOR (i, msize) {
      m.push_back( i, 1 );
    }
    m.canonicalize();
  }

  // for all i: result(i) = m(i) ? 1 : 0
  inline void mask_set_to_one(cvector& result, const mvector& m)
  {
    typeof(result.data.begin()) ri, rend;

    result = m;
    rend = result.data.end();
    for (ri = result.data.begin(); ri != rend; ri++) {
      ri->value = 1;
    }
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
 * Revision 1.3  2005/10/28 02:51:41  trey
 * added copyright headers
 *
 * Revision 1.2  2005/03/28 18:13:06  trey
 * standardized API a bit
 *
 * Revision 1.1  2005/03/10 21:13:23  trey
 * initial check-in
 *
 *
 ***************************************************************************/
