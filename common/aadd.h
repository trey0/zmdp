/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.4 $  $Author: trey $  $Date: 2005-10-21 20:10:30 $
 *  
 * @file    aadd.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCaadd_h
#define INCaadd_h

#include <vector>
#include <fstream>
#include <iostream>
#include <ext/hash_map>

#include "pomdpCommonDefs.h"
#include "sla.h"

#define EXT_NAMESPACE __gnu_cxx

namespace aadd {

#define EXTRACT_VAR(val,index) (!!((val) & (1<<(index))))

  // djb2 -- this algorithm (k=33) was first reported by dan bernstein many
  // years ago in comp.lang.c. another version of this algorithm (now
  // favored by bernstein) uses xor: hash(i) = hash(i - 1) * 33 ^
  // str[i]; the magic of number 33 (why it works better than many other
  // constants, prime or not) has never been adequately explained.
  inline size_t hash_memory4(const void* x, size_t x_size)
  {
    const size_t *xp, *x_end = ((size_t*) x) + (x_size/4);
    size_t ret = 5381;
    for (xp = (size_t*) x; xp < x_end; xp++) {
      ret = (33 * ret) + (*xp);
    }
    return ret;
  }

  // returns the number of bits needed to express numbers in
  // the range [0..x-1]
  inline int needed_bits(unsigned int x)
  {
    int ret;
    int mask = (1<<31);
    x--;
    for (ret = 32; ret >= 1; ret--) {
      if (x & mask) return ret;
      x <<= 1;
    }
    return 0;
  }

  struct avector {
    double c, b;
    int F;
    avector(void) {}
    avector(double _c, double _b, int _F) :
      c(_c),
      b(_b),
      F(_F)
    {}
    bool operator==(const avector& rhs) const {
      return (c == rhs.c) && (b == rhs.b) && (F == rhs.F);
    }
    void write(std::ostream& out) const;
    void write2(std::ostream& out) const;
  };

  // in general, a branch_node id F resolves to a struct branch_node in the
  // node_cache.  id F = 0 is an exception: it resolves to the
  // distinguished branch_node with no branch and value 0.
  struct branch_node {
    int var;
    avector lo, hi;
    branch_node(void) {}
    branch_node(int _var, avector _lo, avector _hi) :
      var(_var),
      lo(_lo),
      hi(_hi)
    {}
    bool operator==(const branch_node& b) const {
      return (var == b.var) && (lo == b.lo) && (hi == b.hi);
    }
    void write(std::ostream& out) const;
  };

  struct hash_branch_node {
    size_t operator()(const branch_node& x) const {
      return hash_memory4(&x,sizeof(x));
    }
  };

  enum op_type {
    OP_ADD,
    OP_MULTIPLY
    //OP_DIVIDE,
    //OP_MIN,
    //OP_MAX
  };

  struct op_node {
    avector x, y;
    op_type op;

    op_node(void) {}
    op_node(const avector& _x, const avector& _y, op_type _op) :
      x(_x),
      y(_y),
      op(_op)
    {}
    bool operator==(const op_node& b) const {
      return (x == b.x) && (y == b.y) && (op == b.op);
    }
  };

  struct hash_op_node {
    size_t operator()(const op_node& x) const {
      return hash_memory4(&x,sizeof(x));
    }
  };

  struct node_cache_type {
    typedef EXT_NAMESPACE::hash_map<branch_node,int,hash_branch_node>
      node_cache_hash_type;

    node_cache_hash_type h;
    std::vector<branch_node> v;

    node_cache_type(void) {
      // 0 is a distinguished id that maps to the 0 function.
      // this ensures no real branch gets 0 as its id.
      v.push_back( branch_node() );
    }

    // inserts n into the cache and assigns it an id (if it is not
    // already present).  returns the id.
    int insert(const branch_node& n)
    {
      std::pair< branch_node, int > pr(n,v.size());
      std::pair< typeof(h.begin()), bool > ans = h.insert(pr);
      if (ans.second) {
	v.push_back(n);
      }

#if 0
      std::cout << "insert: n=" << std::endl;
      n.write(std::cout);
      std::cout << std::endl
		<< " new=" << ans.second
		<< " ret=" << ans.first->second
		<< std::endl;
#endif

      return ans.first->second;
    }

    // if a node with the given id exists, set result to that node.
    // otherwise signal an error.
    void get(int id, branch_node& result)
    {
      assert( 1 <= id && id < (int)v.size() );
      result = v[id];
    }
  };

  struct reduce_cache_type {
    typedef EXT_NAMESPACE::hash_map<int,avector> reduce_cache_hash_type;

    reduce_cache_hash_type h;

    // inserts (F,x) into the cache
    void insert(int F, const avector& x)
    {
      h[F] = x;
    }

    // if some pair (F,x) is in the cache, sets result to x and returns
    // true.  otherwise returns false.
    bool get(int F, avector& result)
    {
      typeof(h.begin()) hpos = h.find(F);
      if (h.end() == hpos) {
	return false;
      } else {
	result = hpos->second;
	return true;
      }
    }
  };

  struct apply_cache_type {
    typedef EXT_NAMESPACE::hash_map<op_node,avector,hash_op_node>
      apply_cache_hash_type;
    
    apply_cache_hash_type h;

    // inserts (o,x) into the cache.
    void insert(const op_node& o, const avector& x)
    {
      h[o] = x;
    }

    // if some pair (o,x) is in the cache, sets result to x and returns
    // true.  otherwise returns false.
    bool get(const op_node& o, avector& result)
    {
      typeof(h.begin()) hpos = h.find(o);
      if (h.end() == hpos) {
	return false;
      } else {
	result = hpos->second;
	return true;
      }
    }
  };

  struct aadd_cache_type {

    reduce_cache_type reduce_cache;
    node_cache_type node_cache;
    apply_cache_type apply_cache;

    void get_avector_for_node(avector& result, const branch_node& n)
    {
      double r_min, r_max, r_range;
      branch_node nr;

      if (n.lo == n.hi) {
	result = n.lo;
	return;
      }
      r_min = std::min( n.lo.c, n.hi.c );
      r_max = std::max( n.lo.c + n.lo.b,
			n.hi.c + n.hi.b );
      r_range = r_max - r_min;

      nr.var = n.var;

      nr.lo.c = (n.lo.c - r_min) / r_range;
      nr.lo.b = n.lo.b / r_range;
      nr.lo.F = n.lo.F;

      nr.hi.c = (n.hi.c - r_min) / r_range;
      nr.hi.b = n.hi.b / r_range;
      nr.hi.F = n.hi.F;

      result.c = r_min;
      result.b = r_range;
      result.F = node_cache.insert(nr);
    }

    void reduce(avector& result, const avector& x)
    {
      branch_node n, np;
      avector resn;

      if (0 == x.F) {
	result.c = x.c;
	result.b = 0;
	result.F = 0;
	return;
      }

      if (!reduce_cache.get( x.F, resn )) {
	node_cache.get( x.F, np );

#if 0
	std::cout << "reduce: F=" << x.F << " np=";
	np.write(std::cout);
	std::cout << std::endl;
#endif

	n.var = np.var;
	reduce( n.lo, np.lo );
	reduce( n.hi, np.hi );
	get_avector_for_node( resn, n );

	reduce_cache.insert( x.F, resn );
      }

      result.c = x.b * resn.c + x.c;
      result.b = x.b * resn.b;
      result.F = resn.F;
    }

    double apply_op(double x, double y, op_type op)
    {
      switch (op) {
      case OP_ADD:
	return x + y;
      case OP_MULTIPLY:
	return x * y;
#if 0
      case OP_DIVIDE:
	return x / y;
      case OP_MIN:
	return std::min( x, y );
      case OP_MAX:
	return std::max( x, y );
#endif
      default:
	abort(); // never reach this point
      }
    }
    
    bool compute_result(avector& result, const op_node& o)
    {
      if (0 == o.x.F) {
	if (0 == o.y.F) {
	  result.c = apply_op( o.x.c, o.y.c, o.op );
	  result.b = 0;
	  result.F = 0;
	  return true;
	}
	if (OP_ADD == o.op) {
	  result.c = o.x.c + o.y.c;
	  result.b = o.y.b;
	  result.F = o.y.F;
	  return true;
	}
	if (OP_MULTIPLY == o.op) {
	  result.c = o.x.c * o.y.c;
	  result.b = o.x.c * o.y.b;
	  result.F = o.y.F;
	}
      }
      if (OP_ADD == o.op && o.x.F == o.y.F) {
	result.c = o.x.c + o.y.c;
	result.b = o.x.b + o.y.b;
	result.F = o.x.F;
	return true;
      }
      // several tests for other operators not implemented
      
      return false;
    }
    
    // given an operation o, define the result function f(o) as follows:
    //
    //    f(o) = (o.x.c + o.x.b*o.x.F) <o.op> (o.y.c + o.y.b*o.y.F)
    //
    // normalize_op(o) calculates a new operation ot and constants cz, bz
    // such that:
    //
    //   f(o) = cz + bz * f(ot)
    //
    // where ot is normalized so that there should be more apply_cache hits.
    void normalize_op(op_node& ot,
		      double& cz,
		      double& bz,
		      const op_node& o)
    {
      const avector *xp, *yp;
      
      ot.op = o.op;

      // reorder operands in increasing order of F
      // (NOTE: assumes a commutative operation!)
      if (o.x.F <= o.y.F) {
	xp = &o.x;
	yp = &o.y;
      } else {
	xp = &o.y;
	yp = &o.x;
      }
      
      switch (o.op) {
      case OP_ADD:
	cz = xp->c + yp->c;
	bz = xp->b;
	
	ot.x.c = 0;
	ot.x.b = 1;
	ot.x.F = xp->F;
	
	ot.y.c = 0;
	ot.y.b = yp->b / xp->b;
	ot.y.F = yp->F;
	
	return;
	
      case OP_MULTIPLY:
	cz = 0;
	bz = xp->b * yp->b;
	
	ot.x.c = xp->c / xp->b;
	ot.x.b = 1;
	ot.x.F = xp->F;
	
	ot.y.c = yp->c / yp->b;
	ot.y.b = 1;
	ot.y.F = yp->F;
	
	return;
	
      default:
	abort(); // never reach this point
      }
    }
    
    // result = x <op> y
    void apply(avector& result, const avector& x, const avector& y,
	       op_type op)
    {
      apply( result, op_node(x,y,op) );
    }
    
    // result = o.x <o.op> o.y
    void apply(avector& result, const op_node& o)
    {
      op_node ot;
      double cz, bz;
      branch_node xn, yn;
      branch_node xr, yr, ar;
      avector resn;
      
      // check if result can be immediately computed
      if (compute_result(result,o)) return;
      
      // normalize operation and check apply cache
      normalize_op( ot, cz, bz, o );
      
      if (!apply_cache.get( ot, resn )) {
	// not terminal, so recurse
	
	node_cache.get( ot.x.F, xn );
	node_cache.get( ot.y.F, yn );
	ar.var = std::max( xn.var, yn.var );
	
	// propagate affine transform to branches
	if ( ar.var == xn.var ) {
	  xr.lo.c = ot.x.c + ot.x.b * xn.lo.c;
	  xr.lo.b = ot.x.b * xn.lo.b;
	  xr.lo.F = xn.lo.F;
	  xr.hi.c = ot.x.c + ot.x.b * xn.hi.c;
	  xr.hi.b = ot.x.b * xn.hi.b;
	  xr.hi.F = xn.hi.F;
	} else {
	  xr.lo = xr.hi = ot.x;
	}
	if ( ar.var == yn.var ) {
	  yr.lo.c = ot.y.c + ot.y.b * yn.lo.c;
	  yr.lo.b = ot.y.b * yn.lo.b;
	  yr.lo.F = yn.lo.F;
	  yr.hi.c = ot.y.c + ot.y.b * yn.hi.c;
	  yr.hi.b = ot.y.b * yn.hi.b;
	  yr.hi.F = yn.hi.F;
	} else {
	  yr.lo = yr.hi = ot.y;
	}
	
	// recurse and get cached result
	apply( ar.lo, xr.lo, yr.lo, ot.op );
	apply( ar.hi, xr.hi, yr.hi, ot.op );
	get_avector_for_node( resn, ar );
	apply_cache.insert( ot, resn );
      }
      
      // correct for normalization
      result.c = cz + bz * resn.c;
      result.b = bz * resn.b;
      result.F = resn.F;
    }

    // result = x
    void copy(avector& result, sla::dvector& x)
    {
      int num_bits, i, k, var, F;
      std::vector<avector> old_layer, new_layer;
      branch_node n;

      num_bits = needed_bits(x.size());
      //std::cout << "num_bits = " << num_bits << std::endl;
      k = (1 << num_bits);

      // convert each value in x to a primitive avector
      for (i=0; i < k; i++) {
	if (i < (int) x.size()) {
	  old_layer.push_back( avector( x(i), 0, 0 ) );
	} else {
	  old_layer.push_back( avector( 0, 0, 0 ) );
	}
      }
      
      // build up successive layers to form a full binary tree
      for (var=0; var < num_bits; var++) {
	k >>= 1;
	for (i=0; i < k; i++) {
	  n.var = var;
	  n.lo = old_layer[ 2*i ];
	  n.hi = old_layer[ 2*i + 1 ];
	  F = node_cache.insert(n);
	  new_layer.push_back( avector( 0, 1, F ) );
	}
	old_layer.swap( new_layer );
	new_layer.clear();
      }

      // canonicalize the full binary tree to a compact AADD
      //result = old_layer[0];
      reduce( result, old_layer[0] );
    }

    void get_child_node_ids_recurse(std::vector<int>& result,
				    int F)
    {
      branch_node n;

      if (0 == F) return;

      result.push_back( F );
      node_cache.get( F, n );
      get_child_node_ids_recurse( result, n.lo.F );
      get_child_node_ids_recurse( result, n.hi.F );
    }

    void get_child_node_ids(std::vector<int>& result,
			    int F)
    {
      get_child_node_ids_recurse( result, F );
      std::sort( result.begin(), result.end() );
      // remove all non-unique elements
      result.erase( std::unique( result.begin(), result.end() ),
		    result.end() );
    }
  };

  extern aadd_cache_type aadd_cache_g;

  // result = x
  inline void copy(avector& result, const sla::dvector& x)
  {
    aadd_cache_g.copy( result, x );    
  }

  inline void copy(sla::dvector& result, const avector& x)
  {
    aadd_cache_g.copy( result, x );
  }

  // result = x + y
  inline void add(avector& result,
		  const avector& x,
		  const avector& y)
  {
    aadd_cache_g.apply( result, x, y, OP_ADD );
  }

  // result = x .* y
  inline void emult(avector& result,
		    const avector& x,
		    const avector& y)
  {
    aadd_cache_g.apply( result, x, y, OP_MULTIPLY );
  }

  inline void avector::write2(std::ostream& out) const {
    out << "c=" << c << " b=" << b << " F=" << F;
  }

  inline void avector::write(std::ostream& out) const {
    branch_node n;
    std::vector<int> child_node_ids;

    write2(out);
    aadd_cache_g.get_child_node_ids(child_node_ids, F);
    FOR_EACH ( Fp, child_node_ids ) {
      out << std::endl;
      aadd_cache_g.node_cache.get(*Fp,n);
      out << "F=" << (*Fp) << ":" << std::endl;
      n.write(out);
    }
  }

  inline void branch_node::write(std::ostream& out) const {
    out << "  var=" << var << std::endl
	<< "    lo: ";
    lo.write2(out);
    out << std::endl
	<< "    hi: ";
    hi.write2(out);
  }

} // namespace aadd

#endif // INCaadd_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/03/09 18:51:28  trey
 * added emult(), made some function args const
 *
 * Revision 1.2  2005/02/25 19:43:31  trey
 * add() works
 *
 * Revision 1.1  2005/02/25 19:31:55  trey
 * initial check-in
 *
 *
 ***************************************************************************/
