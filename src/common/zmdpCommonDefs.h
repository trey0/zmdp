/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-07-24 17:06:10 $
   
 @file    zmdpCommonDefs.h
 @brief   No brief

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

#ifndef INCzmdpCommonDefs_h
#define INCzmdpCommonDefs_h

namespace zmdp {

#undef FOR_EACH
#define FOR_EACH(elt,collection) \
  for (typeof((collection).begin()) elt=(collection).begin(), \
         __end=(collection).end(); \
       elt != __end; \
       elt++)

#undef FOR
#define FOR(i,n) \
  for (unsigned int i=0, __n = (n); i<__n; i++)

#define SPARSE_EPS (1e-10)
#define OBS_IS_ZERO_EPS (1e-10)
#define ZMDP_BOUNDS_PRUNE_EPS (1e-10)

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

template <class _CollectionType, class _ForwardIterator>
_ForwardIterator eraseElement(_CollectionType& c, _ForwardIterator i)
{
  _ForwardIterator ip1 = i;
  ip1++;
  c.erase(i,ip1);
  return ip1;
}

}; // namespace zmdp

#endif // INCzmdpCommonDefs_h
