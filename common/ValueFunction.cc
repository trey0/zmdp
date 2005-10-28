/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2005-10-28 02:51:40 $
   
 @file    ValueFunction.cc
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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

//#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "pomdpCommonDefs.h"
#include "MatrixUtils.h"
#include "ValueFunction.h"

using namespace std;
using namespace MatrixUtils;
using namespace MATRIX_NAMESPACE;

namespace pomdp {

/**********************************************************************
 * MEMBER FUNCTIONS
 **********************************************************************/

// return true if the value intervals returned by this value function
// and <rhs> overlap at all of <numSamples> randomly chosen belief points.
bool ValueFunction::consistentWith(const ValueFunction& rhs, int numSamples,
				   bool debug) const
{
  ValueInterval selfint, rhsint;
  dvector bd(numStates);
  belief_vector b(numStates);
  FOR (i, numSamples) {
    rand_vector(bd,numStates);
    copy(b,bd);
    b *= (1.0/norm_1(b)); // normalize so components add to 1
    selfint = getValueAt(b);
    rhsint = rhs.getValueAt(b);
    if (debug) {
      cout << "b' = " << sparseRep(b) << endl;;
      cout << "lhs = " << selfint << endl;
      cout << "rhs = " << rhsint << endl << endl;
    }
    if (!selfint.overlapsWith(rhsint)) {
      cout << "inconsistent at b' = " << sparseRep(b) << endl;
      cout << "lhs = " << selfint << endl;
      cout << "rhs = " << rhsint << endl << endl;
      return false;
    }
  }
  return true;
}

}; // namespace pomdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/10/21 20:10:10  trey
 * added namespace pomdp
 *
 * Revision 1.4  2005/02/08 23:54:35  trey
 * updated to use less type-specific function names
 *
 * Revision 1.3  2005/01/27 05:33:40  trey
 * modified for sla compatibility
 *
 * Revision 1.2  2005/01/26 04:12:06  trey
 * fixed for new rand_vector() API
 *
 * Revision 1.1  2004/11/24 20:48:05  trey
 * moved to common from hsvi
 *
 *
 ***************************************************************************/
