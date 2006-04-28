/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    ValueFunction.cc
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

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "ValueFunction.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

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

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/04 17:21:49  trey
 * moved ValueFunction from common to bounds
 *
 * Revision 1.9  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.8  2005/11/03 17:45:47  trey
 * removed MATRIX_NAMESPACE macro
 *
 * Revision 1.7  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.6  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.5  2005/10/21 20:10:10  trey
 * added namespace zmdp
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
