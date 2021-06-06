/********** tell emacs we use -*- c++ -*- style comments *******************
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "MatrixUtils.h"
#include "ValueFunction.h"
#include "zmdpCommonDefs.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

/**********************************************************************
 * MEMBER FUNCTIONS
 **********************************************************************/

// return true if the value intervals returned by this value function
// and <rhs> overlap at all of <numSamples> randomly chosen belief points.
bool ValueFunction::consistentWith(const ValueFunction &rhs, int numSamples,
                                   bool debug) const {
  ValueInterval selfint, rhsint;
  dvector bd(numStates);
  belief_vector b(numStates);
  FOR(i, numSamples) {
    rand_vector(bd, numStates);
    copy(b, bd);
    b *= (1.0 / norm_1(b)); // normalize so components add to 1
    selfint = getValueAt(b);
    rhsint = rhs.getValueAt(b);
    if (debug) {
      cout << "b' = " << sparseRep(b) << endl;
      ;
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
