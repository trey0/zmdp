/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-01-26 04:12:23 $
 *  
 * @file    commonDefs.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCcommonDefs_h
#define INCcommonDefs_h

#define FOR_EACH(elt,collection) \
  for (typeof((collection).begin()) elt=(collection).begin(); \
       elt != (collection).end(); elt++)
#define FOR(i,n) \
  for (unsigned int i=0, __n = (n); i<__n; i++)

#define SPARSE_EPS (1e-10)

// argmax(start, end, f) returns the element x between start and end
//   that has the highest f(x) value.
// example: v is a vector, norm is a function defined for vectors
//          v_with_largest_norm = argmax(v.begin(), v.end(), norm);
#define argmax(a,b,c) (argmax_tp<typeof(*(a))>(a,b,c))

template <class _InType, class _ForwardIterator, class _ConvertType>
const _InType& argmax_tp(_ForwardIterator start, _ForwardIterator end,
			_ConvertType f)
{
  typeof(f(*start)) val, max_val;
  const _InType* max_elt;

  assert(start != end);

  max_elt = &(*start);
  max_val = f(*start);

  start++;

  for (_ForwardIterator i = start; i != end; i++) {
    val = f(*i);
    if (val > max_val) {
      max_val = val;
      max_elt = &(*i);
    }
  }

  return *max_elt;
}

#endif // INCcommonDefs_h
