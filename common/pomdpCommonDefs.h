/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2005-10-28 03:50:32 $
   
 @file    pomdpCommonDefs.h
 @brief   No brief

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

#ifndef INCpomdpCommonDefs_h
#define INCpomdpCommonDefs_h

namespace pomdp {

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

}; // namespace pomdp

#endif // INCpomdpCommonDefs_h
