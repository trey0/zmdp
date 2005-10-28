/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2005-10-28 02:51:40 $
   
 @file    pomdpCommonDefs.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * This code and derivatives may not be incorporated into commercial
   products without specific prior written permission.
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * The name of Trey Smith may not be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
